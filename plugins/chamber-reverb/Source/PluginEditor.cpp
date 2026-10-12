#include "PluginEditor.h"

namespace
{
constexpr std::array<const char*, 10> knobIds { "size", "decay", "density", "diffusion", "resonance",
                                              "brightness", "modulation", "predelay", "width", "mix" };
constexpr std::array<const char*, 10> knobNames { "SIZE", "DECAY", "DENSITY", "DIFFUSION", "RESONANCE",
                                                 "BRIGHTNESS", "MODULATION", "PRE-DELAY", "WIDTH", "MIX" };

// Layout: title band, then three sections (selector, early reflections row, FDN matrix row).
// Each section reserves a header band at its top so headers never overlap controls.
constexpr std::array<const char*, 3> sectionTitles { "SYSTEM SPEC // MC-24",
                                                     "EARLY REFLECTIONS // CONV-01",
                                                     "FDN MATRIX // 8-LINE" };
constexpr int outerMargin = 12;
constexpr int titleBandHeight = 52;
constexpr int sectionHeaderHeight = 14;
constexpr int selectorHeight = 32;
constexpr int sectionGap = 4;
constexpr int knobLabelHeight = 18;
constexpr int knobCols = 5;

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

    // Environment selector: one item per environment, bound to the "environment" choice parameter.
    for (int i = 0; i < (int) maya::Environment::Count; ++i)
        environmentSelector.addItem (maya::getEnvironmentInfo ((maya::Environment) i).name, i + 1);
    addAndMakeVisible (environmentSelector);
    environmentAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (processor.apvts, "environment", environmentSelector);

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

juce::Array<juce::Rectangle<int>> MayaChamberEditor::getSectionRects() const
{
    // Single source of truth for paint() and resized(). Returns exactly three rectangles,
    // each starting with its header band, and none overlapping the title band.
    auto body = getLocalBounds().reduced (outerMargin);
    body.removeFromTop (titleBandHeight);

    juce::Array<juce::Rectangle<int>> rects;
    rects.add (body.removeFromTop (sectionHeaderHeight + 6 + selectorHeight));
    body.removeFromTop (sectionGap);

    const int rowHeight = (body.getHeight() - 2 * sectionGap) / 2;
    rects.add (body.removeFromTop (rowHeight));
    body.removeFromTop (sectionGap);
    rects.add (body);
    return rects;
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
    g.setColour (Colour (0xff0b0c0d));
    g.drawRoundedRectangle (bounds.reduced (6.0f), 4.0f, 2.0f);

    // Title band (y 0 to titleBandHeight): silkscreen header, kept clear of sections
    g.setColour (Colour (0xffe9e6dc));
    g.setFont (MayaIndustrialLookAndFeel::silkFont (16.0f, true));
    g.drawText ("MAYA CHAMBER", Rectangle<int> (0, 12, getWidth(), 20), Justification::centred);
    g.setFont (MayaIndustrialLookAndFeel::silkFont (9.0f));
    g.setColour (Colour (0xff8d8a80));
    g.drawText ("ACOUSTIC ENVIRONMENT ENGINE  //  MODEL MC-24", Rectangle<int> (0, 32, getWidth(), 12), Justification::centred);

    // Panel divisions: hairline bezel separator at the top of each section, then its header
    const auto sections = getSectionRects();
    for (int i = 0; i < sections.size(); ++i)
    {
        const auto r = sections[i];
        g.setColour (Colour (0xff0b0c0d));
        g.fillRect (r.getX(), r.getY(), r.getWidth(), 1);
        g.setColour (Colour (0xffffffff).withAlpha (0.08f));
        g.fillRect (r.getX(), r.getY() + 1, r.getWidth(), 1);

        g.setColour (Colour (0xffb5532f));
        g.setFont (MayaIndustrialLookAndFeel::silkFont (8.5f, true));
        g.drawText (sectionTitles[(size_t) i],
                    Rectangle<int> (r.getX() + 8, r.getY() + 3, r.getWidth() - 16, sectionHeaderHeight - 4),
                    Justification::left, true);
    }

    // Corner mounting bolts
    drawBolt (g, { 16.0f, 16.0f });
    drawBolt (g, { (float) getWidth() - 16.0f, 16.0f });
    drawBolt (g, { 16.0f, (float) getHeight() - 16.0f });
    drawBolt (g, { (float) getWidth() - 16.0f, (float) getHeight() - 16.0f });
}

void MayaChamberEditor::resized()
{
    const auto sections = getSectionRects();

    // Section 0: environment selector sits below its header band
    environmentSelector.setBounds (sections[0].withTrimmedTop (sectionHeaderHeight + 6).reduced (24, 0));

    // Sections 1 and 2: knob rows, each cell has a label on top and the knob below
    for (int i = 0; i < (int) knobs.size(); ++i)
    {
        const auto sec = sections[1 + i / knobCols];
        const int col = i % knobCols;
        const int cellW = sec.getWidth() / knobCols;
        auto cell = juce::Rectangle<int> (sec.getX() + col * cellW, sec.getY() + sectionHeaderHeight,
                                          cellW, sec.getHeight() - sectionHeaderHeight);
        knobLabels[i]->setBounds (cell.removeFromTop (knobLabelHeight).reduced (2, 0));
        knobs[i]->setBounds (cell.reduced (4));
    }
}
