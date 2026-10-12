#include "PluginEditor.h"

namespace
{
constexpr std::array<const char*, 10> knobIds { "size", "decay", "density", "diffusion", "resonance",
                                              "brightness", "modulation", "predelay", "width", "mix" };
constexpr std::array<const char*, 10> knobNames { "Size", "Decay", "Density", "Diffusion", "Resonance",
                                                 "Brightness", "Modulation", "Pre-Delay", "Width", "Mix" };
}

MayaChamberEditor::MayaChamberEditor (MayaChamberProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (720, 420);

    // Environment selector: one entry per Maya Chamber environment (24 total).
    environmentSelector.addSectionHeading ("Natural");
    environmentSelector.addSectionHeading ("Architectural");
    environmentSelector.addSectionHeading ("Resonant Objects");
    environmentSelector.addSectionHeading ("Impossible / Abstract");
    for (int i = 0; i < (int) maya::Environment::Count; ++i)
        environmentSelector.addItem (maya::getEnvironmentInfo ((maya::Environment) i).name, i + 1);
    environmentSelector.setSelectedId ((int) processor.getEnvironment() + 1, juce::dontSendNotification);
    environmentSelector.onChange = [this]
    {
        processor.setEnvironment ((maya::Environment) (environmentSelector.getSelectedId() - 1));
    };
    addAndMakeVisible (environmentSelector);

    for (int i = 0; i < (int) knobIds.size(); ++i)
    {
        auto* slider = knobs.add (new juce::Slider (juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow));
        slider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 16);
        addAndMakeVisible (slider);

        auto* label = knobLabels.add (new juce::Label ({}, knobNames[(size_t) i]));
        label->setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);

        attachments.add (new juce::AudioProcessorValueTreeState::SliderAttachment (processor.apvts, knobIds[(size_t) i], *slider));
    }
}

void MayaChamberEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1a1a1f));
    g.setColour (juce::Colours::white);
    g.setFont (18.0f);
    g.drawText ("MAYA CHAMBER", getLocalBounds().removeFromTop (36), juce::Justification::centred);
}

void MayaChamberEditor::resized()
{
    auto area = getLocalBounds().reduced (12);
    area.removeFromTop (36);
    environmentSelector.setBounds (area.removeFromTop (28));
    area.removeFromTop (8);

    const int cols = 5;
    const int cellW = area.getWidth() / cols;
    const int cellH = area.getHeight() / 2;
    for (int i = 0; i < (int) knobs.size(); ++i)
    {
        auto cell = juce::Rectangle<int> (area.getX() + (i % cols) * cellW, area.getY() + (i / cols) * cellH, cellW, cellH);
        knobLabels[i]->setBounds (cell.removeFromTop (18));
        knobs[i]->setBounds (cell.reduced (4));
    }
}
