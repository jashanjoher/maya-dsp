#pragma once

#include <array>
#include <memory>
#include <vector>

#include "PluginProcessor.h"
#include "ui/CuteLookAndFeel.h"

// Bubblegum pedal editor. Every control is bound to the processor's APVTS.
class MayaChamberEditor : public juce::AudioProcessorEditor
{
public:
    explicit MayaChamberEditor (MayaChamberProcessor&);
    ~MayaChamberEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void setupKnob (juce::Slider& slider, juce::Label& label, const juce::String& caption);
    void setupSectionLabel (juce::Label& label, const juce::String& caption);

    MayaChamberProcessor& processor;
    CuteLookAndFeel lookAndFeel;

    juce::Label environmentLabel;
    juce::Label coreLabel;
    juce::Label neuralLabel;

    juce::ComboBox environmentBox;

    juce::Slider preDelaySlider;
    juce::Slider sizeSlider;
    juce::Slider decaySlider;
    juce::Slider dampingSlider;
    juce::Slider mixSlider;
    juce::Slider driveSlider;
    juce::Slider outputSlider;

    juce::Label preDelayLabel;
    juce::Label sizeLabel;
    juce::Label decayLabel;
    juce::Label dampingLabel;
    juce::Label mixLabel;
    juce::Label driveLabel;
    juce::Label outputLabel;

    juce::ToggleButton neuralToggle;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> environmentAttachment;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> neuralAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MayaChamberEditor)
};
