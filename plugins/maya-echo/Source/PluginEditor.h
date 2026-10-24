#pragma once

#include <array>
#include <memory>

#include "PluginProcessor.h"
#include "MeterComponents.h"
#include "ui/CuteLookAndFeel.h"   // from plugins/chamber-reverb/Source/ui

class MayaEchoEditor : public juce::AudioProcessorEditor,
                       private juce::Timer
{
public:
    explicit MayaEchoEditor (MayaEchoProcessor&);
    ~MayaEchoEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    struct Knob
    {
        juce::Label label;
        juce::Slider slider;
        std::unique_ptr<SliderAttachment> attachment;
    };

    MayaEchoProcessor& processor;
    CuteLookAndFeel lookAndFeel;

    std::array<Knob, 6> knobs;

    juce::Label envLabel, divisionLabel;
    juce::ComboBox envBox, divisionBox;
    std::unique_ptr<ComboAttachment> envAttachment, divisionAttachment;

    juce::ToggleButton syncToggle { "Sync" };
    juce::ToggleButton pingPongToggle { "Ping-Pong" };
    std::unique_ptr<ButtonAttachment> syncAttachment, pingPongAttachment;

    VuMeter vuMeter;
    LedBarMeter namMeter;
    juce::Label namLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MayaEchoEditor)
};
