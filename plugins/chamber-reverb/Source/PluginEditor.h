#pragma once
#include "PluginProcessor.h"
#include "ui/MayaIndustrialLookAndFeel.h"

class MayaChamberEditor : public juce::AudioProcessorEditor
{
public:
    explicit MayaChamberEditor (MayaChamberProcessor&);
    ~MayaChamberEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Array<juce::Rectangle<int>> getSectionRects() const;

    MayaChamberProcessor& processor;
    MayaIndustrialLookAndFeel lookAndFeel;
    juce::ComboBox environmentSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> environmentAttachment;
    juce::OwnedArray<juce::Slider> knobs;
    juce::OwnedArray<juce::Label> knobLabels;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> attachments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MayaChamberEditor)
};
