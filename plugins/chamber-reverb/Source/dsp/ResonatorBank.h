#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>

namespace maya
{
// Modal resonator bank: parallel band-pass modes excited by the input.
// Mode frequencies follow a harmonic series stretched by inharmonicity, so
// metallic and bell-like environments ring at non-integer partials.
class ResonatorBank
{
public:
    static constexpr int numModes = 12;

    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void setResonance (float resonance01);                          // maps RESONANCE (mode Q)
    void setFundamental (float hz, float inharmonicity = 0.0f);    // base frequency and stretch of the mode series
    void process (juce::AudioBuffer<float>& buffer);

private:
    std::array<juce::dsp::StateVariableTPTFilter<float>, numModes> modes;
    std::array<float, numModes> modeGains {};
    double sampleRate = 44100.0;
    float fundamentalHz = 110.0f;
    float inharmonicityAmt = 0.0f;
};
} // namespace maya
