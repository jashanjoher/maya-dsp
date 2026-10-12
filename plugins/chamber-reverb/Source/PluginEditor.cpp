#include "PluginEditor.h"

namespace
{
constexpr int editorWidth = 620;
constexpr int editorHeight = 400;
constexpr int outerMargin = 16;
constexpr int titleBandHeight = 34;
constexpr int footerHeight = 16;
constexpr int captionHeight = 14;
constexpr int knobRowHeight = 112;
constexpr int sectionGap = 10;
constexpr int knobColumns = 5;
constexpr int environmentBoxWidth = 320;
constexpr int environmentBoxHeight = 28;
}

MayaChamberEditor::MayaChamberEditor (MayaChamberProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&lookAndFeel);
    setSize (editorWidth, editorHeight);

    // Section captions
    setupSectionLabel (environmentLabel, "ENVIRONMENT");
    setupSectionLabel (coreLabel, "CORE REVERB");
    setupSectionLabel (neuralLabel, "NEURAL DRIVE");

    // Environment selector: one item per environment, bound to the "environment" choice parameter.
    for (int i = 0; i < (int) maya::Environment::Count; ++i)
        environmentBox.addItem (maya::getEnvironmentInfo ((maya::Environment) i).name, i + 1);
    addAndMakeVisible (environmentBox);
    environmentAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (processor.apvts, "environment", environmentBox);

    // Core reverb knobs. Damping is bound to the "brightness" parameter, which is the damping control in the DSP.
    setupKnob (preDelaySlider, preDelayLabel, "PRE-DELAY");
    preDelaySlider.setTextValueSuffix (" ms");
    setupKnob (sizeSlider, sizeLabel, "SIZE");
    sizeSlider.textFromValueFunction = [] (double v) { return juce::String (juce::roundToInt (v * 100.0)) + " %"; };
    sizeSlider.valueFromTextFunction = [] (const juce::String& t) { return t.getDoubleValue() / 100.0; };
    setupKnob (decaySlider, decayLabel, "DECAY");
    decaySlider.setTextValueSuffix (" s");
    setupKnob (dampingSlider, dampingLabel, "DAMPING");
    dampingSlider.textFromValueFunction = [] (double v) { return juce::String (juce::roundToInt (v * 100.0)) + " %"; };
    dampingSlider.valueFromTextFunction = [] (const juce::String& t) { return t.getDoubleValue() / 100.0; };
    setupKnob (mixSlider, mixLabel, "MIX");
    mixSlider.textFromValueFunction = [] (double v) { return juce::String (juce::roundToInt (v * 100.0)) + " %"; };
    mixSlider.valueFromTextFunction = [] (const juce::String& t) { return t.getDoubleValue() / 100.0; };

    // Neural drive knobs and enable switch
    setupKnob (driveSlider, driveLabel, "DRIVE");
    driveSlider.setTextValueSuffix (" dB");
    setupKnob (outputSlider, outputLabel, "OUTPUT");
    outputSlider.setTextValueSuffix (" dB");

    neuralToggle.setButtonText ("NEURAL MODEL");
    addAndMakeVisible (neuralToggle);
    neuralAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processor.apvts, "neural_model", neuralToggle);

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (processor.apvts, "predelay", preDelaySlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (processor.apvts, "size", sizeSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (processor.apvts, "decay", decaySlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (processor.apvts, "brightness", dampingSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (processor.apvts, "mix", mixSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (processor.apvts, "drive", driveSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (processor.apvts, "neural_output", outputSlider));
}

MayaChamberEditor::~MayaChamberEditor()
{
    // Attachments are destroyed before the sliders they point to, so clear the look and feel first.
    setLookAndFeel (nullptr);
}

void MayaChamberEditor::setupKnob (juce::Slider& slider, juce::Label& label, const juce::String& caption)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 16);
    addAndMakeVisible (slider);

    label.setText (caption, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (CuteLookAndFeel::uiFont (9.0f, true));
    label.setColour (juce::Label::textColourId, CuteLookAndFeel::ink());
    addAndMakeVisible (label);
}

void MayaChamberEditor::setupSectionLabel (juce::Label& label, const juce::String& caption)
{
    label.setText (caption, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (CuteLookAndFeel::uiFont (9.0f, true));
    label.setColour (juce::Label::textColourId, CuteLookAndFeel::tealDeep());
    addAndMakeVisible (label);
}

void MayaChamberEditor::paint (juce::Graphics& g)
{
    auto body = getLocalBounds().toFloat().reduced (6.0f);

    // soft drop shadow under the pedal
    g.setColour (juce::Colour (0x40501a33));
    g.fillRoundedRectangle (body.translated (0.0f, 6.0f), 20.0f);

    // bubblegum faceplate
    juce::ColourGradient face (CuteLookAndFeel::pinkTop(), 0.0f, body.getY(),
                               CuteLookAndFeel::pinkBottom(), 0.0f, body.getBottom(), false);
    g.setGradientFill (face);
    g.fillRoundedRectangle (body, 20.0f);

    g.setColour (CuteLookAndFeel::pinkDeep());
    g.drawRoundedRectangle (body.reduced (1.0f), 20.0f, 2.0f);

    g.setColour (CuteLookAndFeel::ink());
    g.setFont (CuteLookAndFeel::uiFont (18.0f, true));
    g.drawText ("maya chamber", getLocalBounds().removeFromTop (titleBandHeight + 4), juce::Justification::centred);

    g.setColour (CuteLookAndFeel::tealDeep());
    g.setFont (CuteLookAndFeel::uiFont (8.0f, true));
    g.drawText ("MAYA DSP  CHAMBER REVERB  MC-24", getLocalBounds().removeFromBottom (footerHeight), juce::Justification::centred);
}

void MayaChamberEditor::resized()
{
    auto area = getLocalBounds().reduced (outerMargin);
    area.removeFromTop (titleBandHeight);
    area.removeFromBottom (footerHeight);

    // Environment selector
    environmentLabel.setBounds (area.removeFromTop (captionHeight));
    environmentBox.setBounds (area.removeFromTop (environmentBoxHeight + 4).withSizeKeepingCentre (environmentBoxWidth, environmentBoxHeight));
    area.removeFromTop (sectionGap);

    // Core reverb knob row
    coreLabel.setBounds (area.removeFromTop (captionHeight));
    auto coreRow = area.removeFromTop (knobRowHeight);
    const int coreCell = coreRow.getWidth() / knobColumns;
    auto placeKnob = [] (juce::Rectangle<int> cell, juce::Label& label, juce::Slider& slider)
    {
        label.setBounds (cell.removeFromTop (captionHeight));
        slider.setBounds (cell.reduced (2));
    };
    placeKnob (coreRow.removeFromLeft (coreCell), preDelayLabel, preDelaySlider);
    placeKnob (coreRow.removeFromLeft (coreCell), sizeLabel, sizeSlider);
    placeKnob (coreRow.removeFromLeft (coreCell), decayLabel, decaySlider);
    placeKnob (coreRow.removeFromLeft (coreCell), dampingLabel, dampingSlider);
    placeKnob (coreRow, mixLabel, mixSlider);
    area.removeFromTop (sectionGap);

    // Neural drive: drive knob, enable switch in the centre, output trim on the right
    neuralLabel.setBounds (area.removeFromTop (captionHeight));
    auto neuralRow = area.removeFromTop (knobRowHeight);
    const int neuralCell = neuralRow.getWidth() / 3;
    placeKnob (neuralRow.removeFromLeft (neuralCell), driveLabel, driveSlider);
    auto toggleCell = neuralRow.removeFromLeft (neuralCell);
    neuralToggle.setBounds (toggleCell.withSizeKeepingCentre (110, 46));
    placeKnob (neuralRow, outputLabel, outputSlider);
}
