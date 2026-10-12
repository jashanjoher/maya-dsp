#pragma once
#include <juce_dsp/juce_dsp.h>

namespace maya
{
// Early reflections stage: partitioned convolution with a swappable IR.
// SKELETON: the IR is currently a placeholder. Real IRs must follow the
// asset discipline in docs/REVERB_SPEC.md (record origin and license for each).
class EarlyReflections
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void setSize (float normalisedSize);       // maps SIZE control
    void loadImpulseResponse (const juce::AudioBuffer<float>& ir, double irSampleRate);
    void process (juce::dsp::ProcessContextReplacing<float>& context);

private:
    juce::dsp::Convolution convolution { juce::dsp::Convolution::Latency { 512 } };
    juce::dsp::ProcessSpec currentSpec {};
    bool prepared = false;
};
} // namespace maya
