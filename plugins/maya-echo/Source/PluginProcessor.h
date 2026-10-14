#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <vector>

#include "dsp/NeuralDrive.h"   // NAM wrapper, from plugins/chamber-reverb/Source/dsp

namespace maya::echo
{
constexpr int kNumEnvironments = 24;

struct EnvironmentProfile
{
    const char* name;
    float highCutHz;       // one-pole lowpass on the feedback path
    float wowScale;        // multiplier on wow and flutter depth
    float stereoOffsetMs;  // extra delay on the right tap
};

const EnvironmentProfile& getEnvironmentProfile (int index);

// Fractional delay line read with cubic Hermite interpolation.
// read() requires delaySamples >= 2 for the samples it touches; the processor enforces a larger minimum.
class EchoLine
{
public:
    void prepare (int maxSamples)
    {
        buffer.assign ((size_t) juce::jmax (8, maxSamples), 0.0f);
        writePos = 0;
    }

    void reset()
    {
        std::fill (buffer.begin(), buffer.end(), 0.0f);
        writePos = 0;
    }

    int size() const { return (int) buffer.size(); }

    float read (float delaySamples) const
    {
        const float pos = (float) writePos - 1.0f - delaySamples;
        const int i1 = (int) std::floor (pos);
        const float t = pos - (float) i1;
        const float y0 = at (i1 - 1);
        const float y1 = at (i1);
        const float y2 = at (i1 + 1);
        const float y3 = at (i1 + 2);
        const float c1 = 0.5f * (y2 - y0);
        const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
        return ((c3 * t + c2) * t + c1) * t + y1;
    }

    void push (float x)
    {
        buffer[(size_t) writePos] = x;
        if (++writePos >= size())
            writePos = 0;
    }

private:
    float at (int i) const
    {
        const int n = size();
        i %= n;
        if (i < 0)
            i += n;
        return buffer[(size_t) i];
    }

    std::vector<float> buffer;
    int writePos = 0;
};
} // namespace maya::echo

class MayaEchoProcessor : public juce::AudioProcessor
{
public:
    MayaEchoProcessor();
    ~MayaEchoProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Maya Echo ME-24"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    float param (const char* id) const { return apvts.getRawParameterValue (id)->load(); }
    double currentBpm() const;
    void processChunk (float* left, float* right, int numSamples);
    float shapeTone (int channel, float x);

    maya::NeuralDrive neural;
    maya::echo::EchoLine lineL, lineR;

    juce::SmoothedValue<float> delaySmooth { 375.0f };
    juce::SmoothedValue<float> feedbackSmooth { 0.45f };
    juce::SmoothedValue<float> mixSmooth { 0.35f };

    std::vector<float> tapL, tapR, namL, namR;
    std::array<float, 2> lpTilt { 0.0f, 0.0f };
    std::array<float, 2> lpEnv { 0.0f, 0.0f };

    float tiltCoeff = 0.0f;
    float envCoeff = 1.0f;
    float tiltAmount = 0.0f;
    float wowDepth = 0.0f;
    float wowInc = 0.0f;
    float flutterInc = 0.0f;
    float wowPhase = 0.0f;
    float flutterPhase = 0.0f;
    float noiseLp = 0.0f;
    float envWowScale = 1.0f;
    float stereoOffsetSamples = 0.0f;
    bool pingPong = true;
    unsigned int rng = 0x12345678u;
    double sampleRate = 44100.0;
    int maxBlock = 512;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MayaEchoProcessor)
};
