#include "EarlyReflections.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <utility>

namespace maya
{
namespace
{
constexpr float speedOfSound = 343.0f;
constexpr float maxEarlyTimeSeconds = 0.12f;
constexpr int maxImageOrder = 4;

// Deterministic generator so the same environment always produces the same IR.
struct Lcg
{
    std::uint32_t state = 12345u;

    float next() noexcept
    {
        state = state * 1664525u + 1013904223u;
        return (float) (state >> 8) * (1.0f / 16777216.0f);
    }
};

// Position of the n-th image of a source across a wall pair in one axis.
float imagePosition (int n, float length, float source) noexcept
{
    return (float) n * length + ((n % 2 == 0) ? source : (length - source));
}
} // namespace

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

void EarlyReflections::loadImpulseResponse (juce::AudioBuffer<float> ir, double irSampleRate)
{
    if (! prepared)
        return;

    convolution.loadImpulseResponse (std::move (ir), irSampleRate,
                                     juce::dsp::Convolution::Stereo::yes,
                                     juce::dsp::Convolution::Trim::no,
                                     juce::dsp::Convolution::Normalise::yes);
}

void EarlyReflections::loadEnvironment (const EnvironmentProfile& profile, double sampleRate)
{
    if (! prepared)
        return;

    loadImpulseResponse (synthesiseRoomImpulse (profile, sampleRate), sampleRate);
}

juce::AudioBuffer<float> EarlyReflections::synthesiseRoomImpulse (const EnvironmentProfile& p, double sampleRate)
{
    const auto sr = (float) sampleRate;
    const int irLength = juce::jmax (1, (int) std::ceil (maxEarlyTimeSeconds * sr));
    juce::AudioBuffer<float> ir (2, irLength);
    ir.clear();

    const float lx = juce::jmax (0.1f, p.roomX);
    const float ly = juce::jmax (0.1f, p.roomY);
    const float lz = juce::jmax (0.1f, p.roomZ);
    const float reflectCoef = std::sqrt (juce::jlimit (0.0f, 1.0f, 1.0f - p.wallAbsorption));

    // Source sits off-centre; the listener is offset along the room length, with left and
    // right ears separated laterally by stereoSpread.
    const float srcX = 0.3f * lx;
    const float srcY = 0.4f * ly;
    const float srcZ = 0.5f * lz;
    const float earX = 0.7f * lx;
    const float earZ = 0.5f * lz;
    const float lateral = p.stereoSpread * 0.4f * ly;
    const float earY[2] { 0.5f * ly - lateral, 0.5f * ly + lateral };

    Lcg rng;

    for (int nx = -maxImageOrder; nx <= maxImageOrder; ++nx)
        for (int ny = -maxImageOrder; ny <= maxImageOrder; ++ny)
            for (int nz = -maxImageOrder; nz <= maxImageOrder; ++nz)
            {
                const int order = std::abs (nx) + std::abs (ny) + std::abs (nz);
                if (order > maxImageOrder)
                    continue;

                const bool isDirect = (order == 0);
                if (! isDirect && rng.next() >= p.tapDensity)
                    continue;

                const float sign = (isDirect || rng.next() < 0.5f) ? 1.0f : -1.0f;
                const float amp = sign * std::pow (reflectCoef, (float) order);

                const float ix = imagePosition (nx, lx, srcX);
                const float iy = imagePosition (ny, ly, srcY);
                const float iz = imagePosition (nz, lz, srcZ);

                for (int ch = 0; ch < 2; ++ch)
                {
                    const float dx = ix - earX;
                    const float dy = iy - earY[ch];
                    const float dz = iz - earZ;
                    const float dist = std::max (0.1f, std::sqrt (dx * dx + dy * dy + dz * dz));
                    const int tap = (int) std::round (dist / speedOfSound * sr);
                    if (tap >= 0 && tap < irLength)
                        ir.addSample (ch, tap, amp / dist);
                }
            }

    return ir;
}

void EarlyReflections::process (juce::dsp::ProcessContextReplacing<float>& context)
{
    if (! prepared)
        return;

    convolution.process (context);
}
} // namespace maya
