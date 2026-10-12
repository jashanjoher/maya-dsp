#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>

namespace maya
{
// Late tail: 8-delay Feedback Delay Network with a Hadamard mixing matrix
// and per-line one-pole damping filters. Delay lengths, damping and decay are
// retuned per environment through setEnvironmentShape and setDecay.
class LateFDN
{
public:
    static constexpr int numDelays = 8;

    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void setDecay (float decaySeconds);      // maps DECAY control
    void setBrightness (float brightness01); // maps BRIGHTNESS (damping filter cutoff)
    void setDensity (float density01);       // maps DENSITY (TODO)
    void setEnvironmentShape (float fdnScale, float dampingScale); // per-environment delay and damping
    void process (juce::AudioBuffer<float>& buffer);

private:
    void applyHadamard (std::array<float, numDelays>& v) const;
    void updateDelayTimes();
    void updateFeedbackGains();
    void updateDampers();

    std::array<juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>, numDelays> delays {
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> { 96000 },
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> { 96000 },
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> { 96000 },
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> { 96000 },
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> { 96000 },
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> { 96000 },
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> { 96000 },
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> { 96000 } };
    std::array<juce::dsp::IIR::Filter<float>, numDelays> dampers;
    std::array<float, numDelays> delayTimesSamples {};
    std::array<float, numDelays> feedbackGain {};
    double sampleRate = 44100.0;
    float decaySeconds = 2.0f;
    float brightness01 = 0.5f;
    float fdnScale = 1.0f;
    float dampingScale = 1.0f;
};
} // namespace maya
