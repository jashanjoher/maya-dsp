#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "dsp/Environments.h"
#include "dsp/EarlyReflections.h"
#include "dsp/LateFDN.h"
#include "dsp/NeuralDrive.h"
#include "dsp/ResonatorBank.h"

// Hybrid signal chain from docs/REVERB_SPEC.md:
// INPUT -> Pre-delay -> Early reflections (partitioned convolution) -> Diffusion
//       -> Modal resonator bank -> FDN late tail -> Neural drive (NAM) -> Dry/wet mix -> OUTPUT
class MayaChamberProcessor : public juce::AudioProcessor
{
public:
    MayaChamberProcessor();
    ~MayaChamberProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Maya Chamber"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // The environment is stored in the "environment" choice parameter, so it is saved with presets.
    maya::Environment getEnvironment() const { return (maya::Environment) (int) apvts.getRawParameterValue ("environment")->load(); }
    void setEnvironment (maya::Environment env)
    {
        if (auto* param = apvts.getParameter ("environment"))
            param->setValueNotifyingHost (param->convertTo0to1 ((float) (int) env));
    }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateDspFromParameters();

    // Convolution (early reflections) latency. Dry signal is delayed by this amount.
    static constexpr int dryLatencySamples = 512;

    int appliedEnvironment = -1;
    double currentSampleRate = 44100.0;
    maya::EarlyReflections earlyReflections;
    maya::ResonatorBank resonators;
    maya::LateFDN lateTail;
    maya::NeuralDrive neuralDrive;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> preDelay { 96000 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> dryDelay { 1024 };
    juce::SmoothedValue<float> mixSmoothed { 0.3f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MayaChamberProcessor)
};
