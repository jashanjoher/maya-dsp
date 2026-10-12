#include "ResonatorBank.h"

namespace maya
{
void ResonatorBank::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    for (auto& m : modes)
    {
        m.prepare (spec);
        m.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
    }
    setResonance (0.5f);
    setFundamental (fundamentalHz, inharmonicityAmt);
}

void ResonatorBank::reset()
{
    for (auto& m : modes)
        m.reset();
}

void ResonatorBank::setResonance (float resonance01)
{
    const auto q = juce::jmap (resonance01, 0.7f, 40.0f);
    for (auto& m : modes)
        m.setResonance (q);
}

void ResonatorBank::setFundamental (float hz, float inharmonicity)
{
    fundamentalHz = hz;
    inharmonicityAmt = inharmonicity;
    for (int i = 0; i < numModes; ++i)
    {
        // Partial i sits at (i+1) times the fundamental, stretched by inharmonicity for higher modes.
        const float stretch = 1.0f + inharmonicityAmt * 0.01f * (float) (i * i);
        const float freq = juce::jlimit (20.0f, (float) sampleRate * 0.45f, hz * (float) (i + 1) * stretch);
        modes[(size_t) i].setCutoffFrequency (freq);
        modeGains[(size_t) i] = 1.0f / (float) (i + 1);
    }
}

void ResonatorBank::process (juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = juce::jmin (buffer.getNumChannels(), 2);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        for (int n = 0; n < numSamples; ++n)
        {
            const float x = data[n];
            float y = 0.0f;
            for (int i = 0; i < numModes; ++i)
                y += modes[(size_t) i].processSample (ch, x) * modeGains[(size_t) i];
            data[n] = y * 0.25f;
        }
    }
}
} // namespace maya
