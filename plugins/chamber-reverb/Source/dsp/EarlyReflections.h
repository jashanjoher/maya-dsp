#pragma once
#include <juce_dsp/juce_dsp.h>
#include "Environments.h"

namespace maya
{
// Early reflections stage: partitioned convolution with a procedurally synthesised IR.
// The IR is built from an image-source model of each environment's room geometry
// (no recorded assets are used, so there is no third-party licence to track).
class EarlyReflections
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void setSize (float normalisedSize);       // maps SIZE control (TODO)
    void loadImpulseResponse (const juce::AudioBuffer<float>& ir, double irSampleRate);
    void loadEnvironment (const EnvironmentProfile& profile, double sampleRate);
    void process (juce::dsp::ProcessContextReplacing<float>& context);

private:
    static juce::AudioBuffer<float> synthesiseRoomImpulse (const EnvironmentProfile& profile, double sampleRate);

    juce::dsp::Convolution convolution { juce::dsp::Convolution::Latency { 512 } };
    juce::dsp::ProcessSpec currentSpec {};
    bool prepared = false;
};
} // namespace maya
