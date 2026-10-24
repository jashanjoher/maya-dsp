#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <array>
#include <cmath>

namespace maya::echo
{
const EnvironmentProfile& getEnvironmentProfile (int index)
{
    // First-pass per-environment tuning, not measured. Names are placeholders for the ME-24 selector.
    static constexpr std::array<EnvironmentProfile, kNumEnvironments> table { {
        { "Tape Room",        6000.0f, 1.00f,  0.0f },
        { "Garage",          4500.0f, 0.80f,  3.0f },
        { "Cathedral",       5000.0f, 0.60f, 11.0f },
        { "Tiled Bath",      8000.0f, 0.50f,  7.0f },
        { "Stone Hall",      5500.0f, 0.70f,  9.0f },
        { "Canyon",          4000.0f, 0.90f, 13.0f },
        { "Cave",            3000.0f, 0.70f, 17.0f },
        { "Hangar",          6500.0f, 0.90f,  5.0f },
        { "Plate Room",      7000.0f, 0.40f,  4.0f },
        { "Spring Tank",     3500.0f, 1.20f,  2.0f },
        { "Lo-Fi Cassette",  2800.0f, 1.50f,  1.0f },
        { "Radio Booth",     2400.0f, 0.60f,  0.0f },
        { "Subway Tunnel",   3800.0f, 0.80f,  6.0f },
        { "Ballroom",        6000.0f, 0.70f,  8.0f },
        { "Chapel",          5200.0f, 0.50f, 12.0f },
        { "Gym",             7500.0f, 0.60f,  5.0f },
        { "Kitchen",         6800.0f, 0.50f,  4.0f },
        { "Bedroom",         5000.0f, 0.70f,  2.0f },
        { "Stairwell",       5800.0f, 0.80f, 10.0f },
        { "Car Interior",    3200.0f, 1.10f,  3.0f },
        { "Studio A",        9000.0f, 0.30f,  0.0f },
        { "Studio B",        8500.0f, 0.40f,  1.0f },
        { "Ice Cave",        9500.0f, 0.50f, 15.0f },
        { "Desert Dome",     4200.0f, 1.00f, 14.0f },
    } };

    return table[(size_t) juce::jlimit (0, kNumEnvironments - 1, index)];
}
} // namespace maya::echo

MayaEchoProcessor::MayaEchoProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    // Embedded NAM default model. If loading fails, the NAM stage passes audio through.
    neural.loadEmbeddedModel();
}

juce::AudioProcessorValueTreeState::ParameterLayout MayaEchoProcessor::createParameterLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "time", 1 }, "Time", NormalisableRange<float> (1.0f, 2000.0f, 0.1f, 0.4f), 375.0f, "ms"));
    params.push_back (std::make_unique<AudioParameterBool> (ParameterID { "sync", 1 }, "Sync", false));
    StringArray divisions { "1/16", "1/8", "1/8 dotted", "1/4", "1/4 dotted", "1/2", "1/1" };
    params.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "division", 1 }, "Division", divisions, 3));
    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "feedback", 1 }, "Feedback", NormalisableRange<float> (0.0f, 110.0f, 0.1f), 45.0f, "%"));
    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "wowflutter", 1 }, "Wow/Flutter", NormalisableRange<float> (0.0f, 100.0f, 0.1f), 20.0f, "%"));
    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "drive", 1 }, "Neural Drive", NormalisableRange<float> (0.0f, 24.0f, 0.01f), 6.0f, "dB"));
    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "color", 1 }, "Color", NormalisableRange<float> (-1.0f, 1.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "mix", 1 }, "Mix", NormalisableRange<float> (0.0f, 100.0f, 0.1f), 35.0f, "%"));
    params.push_back (std::make_unique<AudioParameterBool> (ParameterID { "pingpong", 1 }, "Ping-Pong", true));

    StringArray environmentNames;
    for (int i = 0; i < maya::echo::kNumEnvironments; ++i)
        environmentNames.add (maya::echo::getEnvironmentProfile (i).name);
    params.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "environment", 1 }, "Environment", environmentNames, 0));

    return { params.begin(), params.end() };
}

void MayaEchoProcessor::prepareToPlay (double sr, int samplesPerBlock)
{
    sampleRate = sr;
    maxBlock = juce::jmax (1, samplesPerBlock);

    // 2 s maximum delay plus headroom for wow and flutter.
    const int maxDelaySamples = (int) std::ceil (sr * 2.2) + 64;
    lineL.prepare (maxDelaySamples);
    lineR.prepare (maxDelaySamples);

    tapL.assign ((size_t) maxBlock, 0.0f);
    tapR.assign ((size_t) maxBlock, 0.0f);
    namL.assign ((size_t) maxBlock, 0.0f);
    namR.assign ((size_t) maxBlock, 0.0f);

    neural.prepare (sr, maxBlock);
    neural.setOutputDb (0.0f);

    delaySmooth.reset (sr, 0.08);
    delaySmooth.setCurrentAndTargetValue (param ("time") * 0.001f * (float) sr);
    feedbackSmooth.reset (sr, 0.05);
    feedbackSmooth.setCurrentAndTargetValue (param ("feedback") * 0.01f);
    mixSmooth.reset (sr, 0.02);
    mixSmooth.setCurrentAndTargetValue (param ("mix") * 0.01f);

    lpTilt.fill (0.0f);
    lpEnv.fill (0.0f);
    tiltCoeff = 1.0f - std::exp (-juce::MathConstants<float>::twoPi * 1000.0f / (float) sr);
    wowInc = juce::MathConstants<float>::twoPi * 0.6f / (float) sr;
    flutterInc = juce::MathConstants<float>::twoPi * 7.0f / (float) sr;
    wowPhase = 0.0f;
    flutterPhase = 0.0f;
    noiseLp = 0.0f;

    // Meters: VU ballistics use a 300 ms time constant. The NAM peak falls about 60 dB per second.
    vuCoeff = 1.0f - std::exp (-1.0f / (0.3f * (float) sr));
    namPeakDecay = std::pow (10.0f, -3.0f / (float) sr);
    vuMeanSquare = 0.0f;
    namPeakHold = 0.0f;
    vuLevelDb.store (-60.0f);
    namPeakDb.store (-120.0f);
}

void MayaEchoProcessor::releaseResources()
{
    lineL.reset();
    lineR.reset();
    lpTilt.fill (0.0f);
    lpEnv.fill (0.0f);
}

bool MayaEchoProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

double MayaEchoProcessor::currentBpm() const
{
    if (auto* playHead = getPlayHead())
        if (auto position = playHead->getPosition())
            if (auto bpm = position->getBpm())
                if (*bpm > 1.0)
                    return *bpm;

    return 120.0; // fallback when the host reports no tempo
}

void MayaEchoProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    if (buffer.getNumChannels() < 2 || numSamples == 0)
        return;

    // Delay time: free ms, or a tempo-synced division of the host BPM.
    float timeMs = param ("time");
    if (param ("sync") > 0.5f)
    {
        constexpr std::array<double, 7> beats { 0.25, 0.5, 0.75, 1.0, 1.5, 2.0, 4.0 };
        const int division = juce::jlimit (0, 6, (int) param ("division"));
        timeMs = (float) (beats[(size_t) division] * 60000.0 / currentBpm());
    }
    timeMs = juce::jlimit (1.0f, 2000.0f, timeMs);
    delaySmooth.setTargetValue (timeMs * 0.001f * (float) sampleRate);

    feedbackSmooth.setTargetValue (param ("feedback") * 0.01f);
    mixSmooth.setTargetValue (param ("mix") * 0.01f);
    wowDepth = param ("wowflutter") * 0.01f;
    pingPong = param ("pingpong") > 0.5f;
    tiltAmount = juce::jlimit (-1.0f, 1.0f, param ("color"));

    neural.setEnabled (true);
    neural.setDriveDb (param ("drive"));

    const auto& env = maya::echo::getEnvironmentProfile ((int) param ("environment"));
    envWowScale = env.wowScale;
    stereoOffsetSamples = env.stereoOffsetMs * 0.001f * (float) sampleRate;
    envCoeff = 1.0f - std::exp (-juce::MathConstants<float>::twoPi
                                * juce::jmin (env.highCutHz, 0.45f * (float) sampleRate) / (float) sampleRate);

    float* left = buffer.getWritePointer (0);
    float* right = buffer.getWritePointer (1);
    for (int start = 0; start < numSamples; start += maxBlock)
        processChunk (left + start, right + start, juce::jmin (maxBlock, numSamples - start));

    updateVuMeter (left, right, numSamples);
}

// Tilt-style colour filter: a 1 kHz one-pole split, low and high gains set by Color,
// followed by the environment high-cut. Applied inside the feedback loop.
float MayaEchoProcessor::shapeTone (int channel, float x)
{
    auto& low = lpTilt[(size_t) channel];
    low += tiltCoeff * (x - low);
    const float high = x - low;
    const float tilted = low * (1.0f - 0.3f * tiltAmount) + high * (1.0f + 0.9f * tiltAmount);

    auto& env = lpEnv[(size_t) channel];
    env += envCoeff * (tilted - env);
    return env;
}

void MayaEchoProcessor::processChunk (float* left, float* right, int n)
{
    // Minimum delay keeps every read behind the write pointer for the whole chunk.
    const float minDelay = (float) n + 4.0f;
    const float maxDelay = (float) lineL.size() - 8.0f;
    auto clampDelay = [minDelay, maxDelay] (float d) { return juce::jlimit (minDelay, maxDelay, d); };

    // Phase 1: read the delay taps, with wow and flutter modulation. No writes yet.
    for (int i = 0; i < n; ++i)
    {
        const float wow = std::sin (wowPhase);
        const float flutterTone = std::sin (flutterPhase);

        rng = rng * 1664525u + 1013904223u;
        const float white = (float) (rng >> 9) * (1.0f / 4194304.0f) - 1.0f;
        noiseLp += 0.01f * (white - noiseLp);

        wowPhase += wowInc;
        if (wowPhase > juce::MathConstants<float>::twoPi)
            wowPhase -= juce::MathConstants<float>::twoPi;
        flutterPhase += flutterInc;
        if (flutterPhase > juce::MathConstants<float>::twoPi)
            flutterPhase -= juce::MathConstants<float>::twoPi;

        const float base = delaySmooth.getNextValue();
        const float flutter = 0.7f * flutterTone + 1.5f * noiseLp;
        const float mod = wowDepth * envWowScale * base * (0.004f * wow + 0.0015f * flutter);

        tapL[(size_t) i] = lineL.read (clampDelay (base + mod));
        tapR[(size_t) i] = lineR.read (clampDelay (base + mod + stereoOffsetSamples));
    }

    // Phase 2: feedback path. Ping-pong crosses the taps so echoes bounce between channels.
    const auto& srcL = pingPong ? tapR : tapL;
    const auto& srcR = pingPong ? tapL : tapR;
    for (int i = 0; i < n; ++i)
    {
        namL[(size_t) i] = shapeTone (0, srcL[(size_t) i]);
        namR[(size_t) i] = shapeTone (1, srcR[(size_t) i]);
    }

    // NAM tape/transformer saturation inside the loop (NeuralDrive, one model per channel).
    float* channelPtrs[2] = { namL.data(), namR.data() };
    juce::AudioBuffer<float> feedbackView (channelPtrs, 2, n);
    neural.process (feedbackView);

    // Peak meter: track the instantaneous peak at the NAM output before the limiter.
    // The hold decays about 60 dB per second, so the LED shows fast transients.
    for (int i = 0; i < n; ++i)
    {
        const float peak = juce::jmax (std::abs (namL[(size_t) i]), std::abs (namR[(size_t) i]));
        namPeakHold = juce::jmax (peak, namPeakHold * namPeakDecay);
    }

    // Soft limiter (tanh) after NAM, bounding the loop for runaway safety, then feedback gain.
    for (int i = 0; i < n; ++i)
    {
        const float fb = feedbackSmooth.getNextValue();
        namL[(size_t) i] = fb * std::tanh (namL[(size_t) i]);
        namR[(size_t) i] = fb * std::tanh (namR[(size_t) i]);
    }

    // Phase 3: write input plus feedback, and produce the equal-power dry/wet output.
    for (int i = 0; i < n; ++i)
    {
        const float xl = left[i];
        const float xr = right[i];
        lineL.push (xl + namL[(size_t) i]);
        lineR.push (xr + namR[(size_t) i]);

        const float m = mixSmooth.getNextValue();
        const float wetGain = std::sin (m * juce::MathConstants<float>::halfPi);
        const float dryGain = std::cos (m * juce::MathConstants<float>::halfPi);

        left[i] = xl * dryGain + tapL[(size_t) i] * wetGain;
        right[i] = xr * dryGain + tapR[(size_t) i] * wetGain;
    }
}

// Publishes meter values for the editor. Runs once per block on the audio thread, with no locks.
void MayaEchoProcessor::updateVuMeter (const float* left, const float* right, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        const float mono = 0.5f * (left[i] * left[i] + right[i] * right[i]);
        vuMeanSquare += vuCoeff * (mono - vuMeanSquare);
    }

    // 0 VU is defined as -18 dBFS RMS, the usual convention for a VU meter.
    const float rmsDb = 10.0f * std::log10 (vuMeanSquare + 1.0e-12f);
    vuLevelDb.store (rmsDb + 18.0f, std::memory_order_relaxed);

    const float peakDb = 20.0f * std::log10 (juce::jmax (namPeakHold, 1.0e-6f));
    namPeakDb.store (peakDb, std::memory_order_relaxed);
}

juce::AudioProcessorEditor* MayaEchoProcessor::createEditor()
{
    return new MayaEchoEditor (*this);
}

void MayaEchoProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void MayaEchoProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MayaEchoProcessor();
}
