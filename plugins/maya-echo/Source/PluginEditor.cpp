#include "PluginEditor.h"

namespace
{
struct KnobSpec
{
    const char* paramId;
    const char* caption;
};

constexpr std::array<KnobSpec, 6> knobSpecs { {
    { "time", "Time" },
    { "feedback", "Feedback" },
    { "wowflutter", "Wow/Flutter" },
    { "drive", "Neural Drive" },
    { "color", "Color" },
    { "mix", "Mix" },
} };
} // namespace

MayaEchoEditor::MayaEchoEditor (MayaEchoProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&lookAndFeel);
    setSize (680, 400);

    for (size_t i = 0; i < knobs.size(); ++i)
    {
        auto& knob = knobs[i];
        knob.label.setText (knobSpecs[i].caption, juce::dontSendNotification);
        knob.label.setJustificationType (juce::Justification::centred);
        knob.label.setFont (CuteLookAndFeel::uiFont (13.0f, true));

        knob.slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        knob.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 18);

        addAndMakeVisible (knob.label);
        addAndMakeVisible (knob.slider);
        knob.attachment = std::make_unique<SliderAttachment> (processor.apvts, knobSpecs[i].paramId, knob.slider);
    }

    envLabel.setText ("Environment", juce::dontSendNotification);
    envLabel.setFont (CuteLookAndFeel::uiFont (13.0f, true));
    divisionLabel.setText ("Sync Division", juce::dontSendNotification);
    divisionLabel.setFont (CuteLookAndFeel::uiFont (13.0f, true));
    addAndMakeVisible (envLabel);
    addAndMakeVisible (divisionLabel);

    addAndMakeVisible (envBox);
    addAndMakeVisible (divisionBox);
    if (auto* env = dynamic_cast<juce::AudioParameterChoice*> (processor.apvts.getParameter ("environment")))
        envBox.addItemList (env->choices, 1);
    if (auto* division = dynamic_cast<juce::AudioParameterChoice*> (processor.apvts.getParameter ("division")))
        divisionBox.addItemList (division->choices, 1);
    envAttachment = std::make_unique<ComboAttachment> (processor.apvts, "environment", envBox);
    divisionAttachment = std::make_unique<ComboAttachment> (processor.apvts, "division", divisionBox);

    addAndMakeVisible (syncToggle);
    addAndMakeVisible (pingPongToggle);
    syncAttachment = std::make_unique<ButtonAttachment> (processor.apvts, "sync", syncToggle);
    pingPongAttachment = std::make_unique<ButtonAttachment> (processor.apvts, "pingpong", pingPongToggle);
}

MayaEchoEditor::~MayaEchoEditor()
{
    setLookAndFeel (nullptr);
}

void MayaEchoEditor::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient background (CuteLookAndFeel::pinkTop(), 0.0f, 0.0f,
                                     CuteLookAndFeel::pinkBottom(), 0.0f, bounds.getHeight(), false);
    g.setGradientFill (background);
    g.fillRect (bounds);

    g.setColour (CuteLookAndFeel::ink());
    g.setFont (CuteLookAndFeel::uiFont (22.0f, true));
    g.drawText ("Maya Echo ME-24", getLocalBounds().removeFromTop (44), juce::Justification::centred, false);
}

void MayaEchoEditor::resized()
{
    auto area = getLocalBounds().reduced (16);
    area.removeFromTop (44);

    auto bottom = area.removeFromBottom (78);

    const int knobWidth = area.getWidth() / (int) knobs.size();
    for (auto& knob : knobs)
    {
        auto cell = area.removeFromLeft (knobWidth).reduced (4, 0);
        knob.label.setBounds (cell.removeFromTop (20));
        knob.slider.setBounds (cell);
    }

    auto envCell = bottom.removeFromLeft (230).reduced (6, 0);
    envLabel.setBounds (envCell.removeFromTop (20));
    envBox.setBounds (envCell.removeFromTop (30));

    auto divCell = bottom.removeFromLeft (170).reduced (6, 0);
    divisionLabel.setBounds (divCell.removeFromTop (20));
    divisionBox.setBounds (divCell.removeFromTop (30));

    syncToggle.setBounds (bottom.removeFromLeft (100).reduced (4, 0));
    pingPongToggle.setBounds (bottom.removeFromLeft (120).reduced (4, 0));
}
