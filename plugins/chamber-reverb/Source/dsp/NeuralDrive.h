#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <memory>
#include <vector>

#include "NAM/dsp.h"

namespace maya
{
// Neural drive stage: wraps two NAM Core models (one per channel) and runs the wet signal
// through them. With no model loaded, or when disabled, audio passes through unchanged.
class NeuralDrive
{
public:
    // Builds the model from the embedded WaveNet A1 default. Returns false if loading fails.
    bool loadEmbeddedModel();
    bool hasModel() const;

    // Must be called from prepareToPlay. Resets the models at the host rate and block size.
    void prepare (double sampleRate, int maxBlockSize);

    void setEnabled (bool shouldProcess)      { enabled = shouldProcess; }
    void setDriveDb (float driveDb)           { driveDb_ = driveDb; }
    void setOutputDb (float outputDb)         { outputDb_ = outputDb; }

    // Real-time safe after prepare(). Input is scaled by the drive, output by the trim.
    void process (juce::AudioBuffer<float>& buffer);

private:
    void resetModels();

    std::array<std::unique_ptr<nam::DSP>, 2> models;
    std::array<std::vector<float>, 2> scratchIn;
    std::array<std::vector<float>, 2> scratchOut;
    double sampleRate = 0.0;
    int blockSize = 0;
    bool enabled = false;
    float driveDb_ = 6.0f;
    float outputDb_ = 0.0f;
};
} // namespace maya
