#include "LateFDN.h"

namespace maya
{
namespace
{
// Prime-ish base delays in ms, one per line (placeholder values, to be tuned).
constexpr std::array<float, LateFDN::numDelays> baseDelaysMs { 29.7f, 37.1f, 41.1f, 43.7f, 47.3f, 53.9f, 59.3f, 61.7f };

// 1/sqrt(8): normalises the Hadamard matrix and the input feed so the loop stays orthogonal.
constexpr float invSqrt8 = 0.35355339f;
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
        delays[(size_t) i].setDelay (delayTimesSamples[(size_t) i]);
        dampers[(size_t) i].prepare (spec);
        dampers[(size_t) i].reset();
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
    for (auto& x : v)
        x *= invSqrt8;
}

void LateFDN::process (juce::AudioBuffer<float>& buffer)
{
    // The FDN is a single shared network. Each sample is popped once from every line and
    // pushed once to every line, regardless of channel count. Channels are not looped over
    // the network, which previously advanced the delays and dampers once per channel.
    const int numSamples = buffer.getNumSamples();
    const int numChannels = juce::jmin (buffer.getNumChannels(), 2);
    if (numChannels == 0)
        return;

    for (int n = 0; n < numSamples; ++n)
    {
        // 1. Read every delay line exactly once.
        std::array<float, numDelays> lineOut {};
        for (int i = 0; i < numDelays; ++i)
            lineOut[(size_t) i] = delays[(size_t) i].popSample (0);

        // 2. Read the input once, as a mono feed of all channels.
        float input = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            input += buffer.getSample (ch, n);
        input /= (float) numChannels;

        // 3. Damp, scale by decay, mix through the Hadamard matrix.
        std::array<float, numDelays> feedback {};
        for (int i = 0; i < numDelays; ++i)
            feedback[(size_t) i] = dampers[(size_t) i].processSample (lineOut[(size_t) i]) * feedbackGain[(size_t) i];
        applyHadamard (feedback);

        // 4. Write every delay line exactly once, feeding the input into each line.
        for (int i = 0; i < numDelays; ++i)
            delays[(size_t) i].pushSample (0, feedback[(size_t) i] + input * invSqrt8);

        // 5. Stereo tap: even lines to left, odd lines to right. Mono output is the sum of both.
        float left = 0.0f;
        float right = 0.0f;
        for (int i = 0; i < numDelays; ++i)
            ((i % 2 == 0) ? left : right) += lineOut[(size_t) i];

        if (numChannels == 1)
        {
            buffer.setSample (0, n, (left + right) * 0.125f);
        }
        else
        {
            buffer.setSample (0, n, left * 0.25f);
            buffer.setSample (1, n, right * 0.25f);
        }
    }
}
} // namespace maya
