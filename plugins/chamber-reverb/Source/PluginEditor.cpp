#include "PluginEditor.h"

namespace
{
constexpr std::array<const char*, 10> knobIds { "size", "decay", "density", "diffusion", "resonance",
                                              "brightness", "modulation", "predelay", "width", "mix" };
constexpr std::array<const char*, 10> knobNames { "SIZE", "DECAY", "DENSITY", "DIFFUSION", "RESONANCE",
                                                 "BRIGHTNESS", "MODULATION", "PRE-DELAY", "WIDTH", "MIX" };
constexpr std::array<const char*, 4> sectionTitles { "SYSTEM SPEC // MC-24", "EARLY REFLECTIONS // CONV-01",
                                                     "FDN MATRIX // 8-LINE", "OUTPUT // STEREO" };

void drawBolt (juce::Graphics& g, juce::Point<float> c)
{
    g.setColour (juce::Colour (0xff0b0c0d));
    g.fillEllipse (c.x - 4.5f, c.y - 4.5f, 9.0f, 9.0f);
    g.setColour (juce::Colour (0xff6d6e70));
    g.fillEllipse (c.x - 3.5f, c.y - 3.5f, 7.0f, 7.0f);
    g.setColour (juce::Colour (0xffe9e6dc).withAlpha (0.6f));
    g.drawLine (c.x - 2.5f, c.y, c.x + 2.5f, c.y, 1.0f);
}
}

MayaChamberEditor::MayaChamberEditor (MayaChamberProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&lookAndFeel);
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
        label->setFont (MayaIndustrialLookAndFeel::silkFont (10.0f, true));
        addAndMakeVisible (label);

        attachments.add (new juce::AudioProcessorValueTreeState::SliderAttachment (processor.apvts, knobIds[(size_t) i], *slider));
    }
}

MayaChamberEditor::~MayaChamberEditor()
{
    setLookAndFeel (nullptr);
}

void MayaChamberEditor::paint (juce::Graphics& g)
{
    using namespace juce;
    const auto bounds = getLocalBounds().toFloat();

    // Matte powder-coated chassis with a faint vignette
    g.fillAll (Colour (0xff2b2c2e));
    ColourGradient vignette (Colours::transparentBlack, bounds.getCentre(), Colours::black.withAlpha (0.35f), { bounds.getX(), bounds.getY() }, true);
    g.setGradientFill (vignette);
    g.fillAll();

    // Recessed outer bezel
    auto faceplate = bounds.reduced (6.0f);
    g.setColour (Colour (0xff0b0c0d));
    g.drawRoundedRectangle (faceplate, 4.0f, 2.0f);

    // Title block: silkscreen header
    g.setColour (Colour (0xffe9e6dc));
    g.setFont (MayaIndustrialLookAndFeel::silkFont (16.0f, true));
    g.drawText ("MAYA CHAMBER", Rectangle<int> (0, 14, getWidth(), 20), Justification::centred);
    g.setFont (MayaIndustrialLookAndFeel::silkFont (9.0f));
    g.setColour (Colour (0xff8d8a80));
    g.drawText ("ACOUSTIC ENVIRONMENT ENGINE  //  MODEL MC-24", Rectangle<int> (0, 34, getWidth(), 12), Justification::centred);

    // Panel divisions: hairline bezel separators with catalog headers
    const auto sections = getSectionRects();
    for (int i = 0; i < (int) sections.size(); ++i)
    {
        const auto& r = sections[(size_t) i];
        g.setColour (Colour (0xff0b0c0d));
        g.fillRect (r.getX(), r.getY(), r.getWidth(), 1);
        g.setColour (Colour (0xffffffff).withAlpha (0.08f));
        g.fillRect (r.getX(), r.getY() + 1, r.getWidth(), 1);

        g.setColour (Colour (0xffb5532f));
        g.setFont (MayaIndustrialLookAndFeel::silkFont (8.5f, true));
        g.drawText (sectionTitles[(size_t) i], r.withTrimmedTop (4).removeFromTop (12).withTrimmedLeft (8), Justification::left);
    }

    // Corner mounting bolts
    drawBolt (g, { 16.0f, 16.0f });
    drawBolt (g, { (float) getWidth() - 16.0f, 16.0f });
    drawBolt (g, { 16.0f, (float) getHeight() - 16.0f });
    drawBolt (g, { (float) getWidth() - 16.0f, (float) getHeight() - 16.0f });
}

juce::Array<juce::Rectangle<int>> MayaChamberEditor::getSectionRects() const
{
    // Top section (environment selector) and two knob rows
    juce::Array<juce::Rectangle<int>> rects;
    auto area = getLocalBounds().reduced (12);
    area.removeFromTop (52);
    rects.add (area.removeFromTop (44));
    rects.add (area.removeFromTop (area.getHeight() / 2));
    rects.add (area);
    rects.add (juce::Rectangle<int>());
    return rects;
}

void MayaChamberEditor::resized()
{
    auto area = getLocalBounds().reduced (12);
    area.removeFromTop (52);

    auto selectorRow = area.removeFromTop (44);
    environmentSelector.setBounds (selectorRow.reduced (24, 10));

    const int cols = 5;
    const int cellW = area.getWidth() / cols;
    const int cellH = area.getHeight() / 2;
    for (int i = 0; i < (int) knobs.size(); ++i)
    {
        auto cell = juce::Rectangle<int> (area.getX() + (i % cols) * cellW, area.getY() + (i / cols) * cellH, cellW, cellH);
        knobLabels[i]->setBounds (cell.removeFromTop (18).reduced (2, 0));
        knobs[i]->setBounds (cell.reduced (4));
    }
}
