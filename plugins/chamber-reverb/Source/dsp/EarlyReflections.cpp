#include "EarlyReflections.h"

namespace maya
{
void EarlyReflections::prepare (const juce::dsp::ProcessSpec& spec)
{
    currentSpec = spec;
    convolution.prepare (spec);
    prepared = true;
}

void EarlyReflections::reset()
{
    convolution.reset();
}

void EarlyReflections::setSize (float normalisedSize)
{
    // TODO: scale the early reflection tap spacing or IR time-stretch with SIZE.
    juce::ignoreUnused (normalisedSize);
}

void EarlyReflections::loadImpulseResponse (const juce::AudioBuffer<float>& ir, double irSampleRate)
{
    if (! prepared)
        return;

    // Load into the partitioned convolution engine. Resampling to the host rate is TODO.
    convolution.loadImpulseResponse (ir, irSampleRate,
                                     juce::dsp::Convolution::Stereo::yes,
                                     juce::dsp::Convolution::Trim::no,
                                     juce::dsp::Convolution::Normalise::yes);
}

void EarlyReflections::process (juce::dsp::ProcessContextReplacing<float>& context)
{
    if (! prepared)
        return;

    convolution.process (context);
}
} // namespace maya
