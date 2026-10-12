#include "NeuralDrive.h"

#include <nlohmann/json.hpp>
#include <exception>
#include <string>
#include <utility>

#include "NAM/get_dsp.h"
#include "EmbeddedModel.h"

namespace maya
{
bool NeuralDrive::loadEmbeddedModel()
{
    std::array<std::unique_ptr<nam::DSP>, 2> fresh;

    try
    {
        const std::string modelText (reinterpret_cast<const char*> (kWavenetA1Model), kWavenetA1ModelSize);
        const nlohmann::json config = nlohmann::json::parse (modelText);

        for (auto& model : fresh)
            model = nam::get_dsp (config);
    }
    catch (const std::exception& e)
    {
        juce::Logger::writeToLog ("NAM embedded model load failed: " + juce::String (e.what()));
        return false;
    }

    if (fresh[0] == nullptr || fresh[1] == nullptr)
        return false;

    models = std::move (fresh);
    resetModels();
    return true;
}

bool NeuralDrive::hasModel() const
{
    return models[0] != nullptr && models[1] != nullptr;
}

void NeuralDrive::prepare (double newSampleRate, int maxBlockSize)
{
    sampleRate = newSampleRate;
    blockSize = juce::jmax (1, maxBlockSize);

    for (auto& buf : scratchIn)
        buf.assign ((size_t) blockSize, 0.0f);
    for (auto& buf : scratchOut)
        buf.assign ((size_t) blockSize, 0.0f);

    resetModels();
}

void NeuralDrive::resetModels()
{
    if (sampleRate <= 0.0 || blockSize <= 0)
        return;

    for (auto& model : models)
        if (model != nullptr)
            model->Reset (sampleRate, blockSize);
}

void NeuralDrive::process (juce::AudioBuffer<float>& buffer)
{
    if (! enabled || ! hasModel() || blockSize <= 0)
        return;

    const int numChannels = juce::jmin (buffer.getNumChannels(), 2);
    const int numSamples = buffer.getNumSamples();

    if (numChannels == 0 || numSamples == 0)
        return;

    const float inputGain = juce::Decibels::decibelsToGain (driveDb_);
    const float outputGain = juce::Decibels::decibelsToGain (outputDb_);

    // Process in chunks no larger than the block size prepared in prepare().
    for (int start = 0; start < numSamples; start += blockSize)
    {
        const int n = juce::jmin (blockSize, numSamples - start);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* channelData = buffer.getWritePointer (ch, start);
            auto& in = scratchIn[(size_t) ch];
            auto& out = scratchOut[(size_t) ch];

            for (int i = 0; i < n; ++i)
                in[(size_t) i] = channelData[i] * inputGain;

            // NAM_SAMPLE is float (NAM_SAMPLE_FLOAT), so the channel pointers are float* arrays.
            float* inPtr = in.data();
            float* outPtr = out.data();
            models[(size_t) ch]->process (&inPtr, &outPtr, n);

            for (int i = 0; i < n; ++i)
                channelData[i] = outPtr[i] * outputGain;
        }
    }
}
} // namespace maya
