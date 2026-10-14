#include "PluginProcessor.h"
#include "PluginEditor.h"

MayaChamberProcessor::MayaChamberProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    // The NAM default model is embedded at build time. If it fails to load, the stage is bypassed.
    neuralDrive.loadEmbeddedModel();
}

juce::AudioProcessorValueTreeState::ParameterLayout MayaChamberProcessor::createParameterLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    // Main musical controls from the spec. All normalised 0..1 unless noted.
    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "size", 1 }, "Size", 0.0f, 1.0f, 0.5f));
    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "decay", 1 }, "Decay", NormalisableRange<float> (0.1f, 30.0f, 0.001f, 0.3f), 2.0f, "s"));
    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "density", 1 }, "Density", 0.0f, 1.0f, 0.5f));
    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "diffusion", 1 }, "Diffusion", 0.0f, 1.0f, 0.5f));
    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "resonance", 1 }, "Resonance", 0.0f, 1.0f, 0.5f));
    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "brightness", 1 }, "Brightness", 0.0f, 1.0f, 0.5f));
    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "modulation", 1 }, "Modulation", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "predelay", 1 }, "Pre-Delay", NormalisableRange<float> (0.0f, 250.0f, 0.1f), 20.0f, "ms"));
    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "width", 1 }, "Width", 0.0f, 1.0f, 1.0f));
    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "mix", 1 }, "Mix", 0.0f, 1.0f, 0.3f));

    // Neural drive (NAM). Drive is the input gain into the model, Neural Model turns the stage on,
    // and Neural Output is the trim applied after the model. The stage is off by default.
    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "drive", 1 }, "Drive", NormalisableRange<float> (0.0f, 24.0f, 0.01f), 6.0f, "dB"));
    params.push_back (std::make_unique<AudioParameterBool> (ParameterID { "neural_model", 1 }, "Neural Model", false));
    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "neural_output", 1 }, "Neural Output", NormalisableRange<float> (-24.0f, 12.0f, 0.01f), 0.0f, "dB"));

    // Environment selector: one choice per environment, in enum order.
    StringArray environmentNames;
    for (int i = 0; i < (int) maya::Environment::Count; ++i)
        environmentNames.add (maya::getEnvironmentInfo ((maya::Environment) i).name);
    params.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "environment", 1 }, "Environment", environmentNames, 0));

    return { params.begin(), params.end() };
}

void MayaChamberProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 2 };
    currentSampleRate = sampleRate;
    appliedEnvironment = -1; // force the environment to be re-applied after prepare

    earlyReflections.prepare (spec);
    resonators.prepare (spec);
    lateTail.prepare (spec);
    preDelay.prepare (spec);
    neuralDrive.prepare (sampleRate, samplesPerBlock);

    // Dry path is delayed by the convolution latency so dry and wet stay phase-aligned.
    dryDelay.prepare (spec);
    dryDelay.reset();
    dryDelay.setDelay ((float) dryLatencySamples);
    setLatencySamples (dryLatencySamples);

    // Equal-power mix ramp, 20 ms, starting at the current parameter value.
    mixSmoothed.reset (sampleRate, 0.02);
    mixSmoothed.setCurrentAndTargetValue (apvts.getRawParameterValue ("mix")->load());

    updateDspFromParameters();
}

void MayaChamberProcessor::releaseResources()
{
    earlyReflections.reset();
    resonators.reset();
    lateTail.reset();
    preDelay.reset();
    dryDelay.reset();
}

bool MayaChamberProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

void MayaChamberProcessor::updateDspFromParameters()
{
    const auto envIndex = (int) apvts.getRawParameterValue ("environment")->load();
    const auto& profile = maya::getEnvironmentProfile ((maya::Environment) envIndex);

    // Only rebuild the environment-dependent state when the selection actually changes.
    if (envIndex != appliedEnvironment)
    {
        appliedEnvironment = envIndex;
        earlyReflections.loadEnvironment (profile, currentSampleRate);
        lateTail.setEnvironmentShape (profile.fdnScale, profile.dampingScale);
        resonators.setFundamental (profile.fundamentalHz, profile.inharmonicity);
    }

    const auto decay = apvts.getRawParameterValue ("decay")->load() * profile.t60Scale;
    const auto brightness = apvts.getRawParameterValue ("brightness")->load();
    const auto density = apvts.getRawParameterValue ("density")->load();
    const auto resonance = apvts.getRawParameterValue ("resonance")->load();
    const auto size = apvts.getRawParameterValue ("size")->load();

    lateTail.setDecay (decay);
    lateTail.setBrightness (brightness);
    lateTail.setDensity (density);
    resonators.setResonance (resonance);
    earlyReflections.setSize (size);

    neuralDrive.setEnabled (apvts.getRawParameterValue ("neural_model")->load() > 0.5f);
    neuralDrive.setDriveDb (apvts.getRawParameterValue ("drive")->load());
    neuralDrive.setOutputDb (apvts.getRawParameterValue ("neural_output")->load());
}

void MayaChamberProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    updateDspFromParameters();

    const auto numSamples = buffer.getNumSamples();
    const auto numChannels = buffer.getNumChannels();
    const auto mix = apvts.getRawParameterValue ("mix")->load();
    const auto preDelayMs = apvts.getRawParameterValue ("predelay")->load();

    mixSmoothed.setTargetValue (mix);

    juce::AudioBuffer<float> dry;
    dry.makeCopyOf (buffer);

    // 1. Pre-delay
    preDelay.setDelay ((float) (preDelayMs * 0.001 * getSampleRate()));
    for (int ch = 0; ch < numChannels; ++ch)
        for (int n = 0; n < numSamples; ++n)
        {
            const float x = buffer.getSample (ch, n);
            preDelay.pushSample (ch, x);
            buffer.setSample (ch, n, preDelay.popSample (ch));
        }

    // 2. Early reflections (partitioned convolution with the environment IR)
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    earlyReflections.process (context);

    // 3. Modal resonator bank
    resonators.process (buffer);

    // 4. FDN late tail (sums its output into the buffer)
    lateTail.process (buffer);

    // 5. Neural drive (NAM). Runs on the wet signal, before the dry/wet blend. Bypassed when off.
    neuralDrive.process (buffer);

    // 6. Dry/wet mix: equal-power crossfade with a smoothed mix value. The dry signal is
    //    delayed by the convolution latency so it lines up with the wet signal.
    for (int n = 0; n < numSamples; ++n)
    {
        const float m = mixSmoothed.getNextValue();
        const float wetGain = std::sin (m * juce::MathConstants<float>::halfPi);
        const float dryGain = std::cos (m * juce::MathConstants<float>::halfPi);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            dryDelay.pushSample (ch, dry.getSample (ch, n));
            const float d = dryDelay.popSample (ch);
            const float wet = buffer.getSample (ch, n);
            buffer.setSample (ch, n, d * dryGain + wet * wetGain);
        }
    }
}

juce::AudioProcessorEditor* MayaChamberProcessor::createEditor()
{
    return new MayaChamberEditor (*this);
}

void MayaChamberProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void MayaChamberProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

// Plugin entry point.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MayaChamberProcessor();
}
