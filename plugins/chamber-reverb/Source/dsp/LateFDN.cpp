#include "LateFDN.h"

namespace maya
{
namespace
{
// Prime-ish base delays in ms, one per line (placeholder values, to be tuned).
constexpr std::array<float, LateFDN::numDelays> baseDelaysMs { 29.7f, 37.1f, 41.1f, 43.7f, 47.3f, 53.9f, 59.3f, 61.7f };
} // namespace

void LateFDN::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    for (auto& d : delays)
    {
        d.prepare (spec);
        d.reset();
    }
    for (int i = 0; i < numDelays; ++i)
    {
        delayTimesSamples[(size_t) i] = baseDelaysMs[(size_t) i] * 0.001f * (float) sampleRate;
        dampers[(size_t) i].prepare (spec);
    }
    setDecay (2.0f);
    setBrightness (0.5f);
}

void LateFDN::reset()
{
    for (auto& d : delays)
        d.reset();
    for (auto& f : dampers)
        f.reset();
}

void LateFDN::setDecay (float decaySeconds)
{
    // Per-line gain so that the loop reaches -60 dB after decaySeconds: g = 10^(-3 * D / T60).
    for (int i = 0; i < numDelays; ++i)
    {
        const float t = delayTimesSamples[(size_t) i] / (float) sampleRate;
        feedbackGain[(size_t) i] = std::pow (10.0f, -3.0f * t / juce::jmax (decaySeconds, 0.05f));
    }
}

void LateFDN::setBrightness (float brightness01)
{
    const auto cutoff = juce::jmap (brightness01, 800.0f, 12000.0f);
    for (auto& f : dampers)
        *f.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, cutoff);
}

void LateFDN::setDensity (float density01)
{
    // TODO: map density to delay-line length spread; currently unused.
    juce::ignoreUnused (density01);
}

void LateFDN::applyHadamard (std::array<float, numDelays>& v) const
{
    // Fast Walsh-Hadamard transform, normalised by 1/sqrt(8), so the matrix is orthogonal.
    for (int h = 1; h < numDelays; h <<= 1)
        for (int i = 0; i < numDelays; i += h << 1)
            for (int j = i; j < i + h; ++j)
            {
                const float a = v[(size_t) j];
                const float b = v[(size_t) (j + h)];
                v[(size_t) j] = a + b;
                v[(size_t) (j + h)] = a - b;
            }
    constexpr float norm = 0.35355339f; // 1 / sqrt(8)
    for (auto& x : v)
        x *= norm;
}

void LateFDN::process (juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = juce::jmin (buffer.getNumChannels(), 2);

    for (int n = 0; n < numSamples; ++n)
    {
        std::array<float, numDelays> lineOut {};

        for (int i = 0; i < numDelays; ++i)
        {
            auto& line = delays[(size_t) i];
            line.setDelay (delayTimesSamples[(size_t) i]);
            lineOut[(size_t) i] = line.popSample (0);
        }

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float input = buffer.getSample (ch, n);
            std::array<float, numDelays> feedback {};

            for (int i = 0; i < numDelays; ++i)
            {
                const float damped = dampers[(size_t) i].processSample (lineOut[(size_t) i]);
                feedback[(size_t) i] = damped * feedbackGain[(size_t) i];
            }
            applyHadamard (feedback);

            for (int i = 0; i < numDelays; ++i)
                delays[(size_t) i].pushSample (0, feedback[(size_t) i] + (i == ch ? input : 0.0f));

            // Simple output tap: sum of line outputs, scaled.
            float wet = 0.0f;
            for (auto v : lineOut)
                wet += v;
            buffer.setSample (ch, n, wet * 0.125f);
        }
    }
}
} // namespace maya
