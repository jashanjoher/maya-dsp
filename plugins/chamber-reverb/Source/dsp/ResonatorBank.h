#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>

namespace maya
{
// Modal resonator bank: a set of parallel two-pole resonators (modes) excited by the input.
// SKELETON: modal frequencies are placeholders; pitch tracking and per-environment
// mode tables are TODO.
class ResonatorBank
{
public:
    static constexpr int numModes = 12;

    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void setResonance (float resonance01);   // maps RESONANCE (mode Q)
    void setFundamental (float hz);          // base frequency for the mode series
    void process (juce::AudioBuffer<float>& buffer);

private:
    std::array<juce::dsp::StateVariableTPTFilter<float>, numModes> modes;
    std::array<float, numModes> modeGains {};
    double sampleRate = 44100.0;
    float fundamentalHz = 110.0f;
};
} // namespace maya
