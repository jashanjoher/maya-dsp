#pragma once
#include "PluginProcessor.h"

class MayaChamberEditor : public juce::AudioProcessorEditor
{
public:
    explicit MayaChamberEditor (MayaChamberProcessor&);
    ~MayaChamberEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    MayaChamberProcessor& processor;
    juce::ComboBox environmentSelector;
    juce::OwnedArray<juce::Slider> knobs;
    juce::OwnedArray<juce::Label> knobLabels;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> attachments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MayaChamberEditor)
};
