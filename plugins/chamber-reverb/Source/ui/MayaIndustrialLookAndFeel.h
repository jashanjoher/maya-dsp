#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// Maya Industrial LookAndFeel: matte powder-coated chassis, silkscreen typography,
// fluted knobs with knurled skirts and etched notch indicators.
namespace maya::industrial
{
struct Palette
{
    static juce::Colour chassis()     { return juce::Colour (0xff2b2c2e); }
    static juce::Colour chassisDeep() { return juce::Colour (0xff1d1e20); }
    static juce::Colour bezel()       { return juce::Colour (0xff0b0c0d); }
    static juce::Colour highlight()   { return juce::Colour (0xffffffff).withAlpha (0.12f); }
    static juce::Colour silk()        { return juce::Colour (0xffe9e6dc); }
    static juce::Colour silkDim()     { return juce::Colour (0xff8d8a80); }
    static juce::Colour oxide()       { return juce::Colour (0xffb5532f); }
};
}

class MayaIndustrialLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MayaIndustrialLookAndFeel()
    {
        using P = maya::industrial::Palette;
        setColour (juce::Slider::textBoxTextColourId, P::silk());
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::Label::textColourId, P::silk());
        setColour (juce::ComboBox::backgroundColourId, P::chassisDeep());
        setColour (juce::ComboBox::textColourId, P::silk());
        setColour (juce::ComboBox::outlineColourId, P::bezel());
        setColour (juce::ComboBox::arrowColourId, P::oxide());
        setColour (juce::PopupMenu::backgroundColourId, P::chassisDeep());
        setColour (juce::PopupMenu::textColourId, P::silk());
        setColour (juce::PopupMenu::highlightedBackgroundColourId, P::oxide());
        setColour (juce::PopupMenu::highlightedTextColourId, P::silk());
    }

    static juce::Font silkFont (float height, bool bold = false)
    {
        return juce::Font (juce::FontOptions ("Menlo", height, bold ? juce::Font::bold : juce::Font::plain));
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        using namespace juce;
        using P = maya::industrial::Palette;

        auto bounds = Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (4.0f);
        const float outerR = jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto centre = bounds.getCentre();
        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // Point at angle a (0 = 12 o'clock, clockwise) and radius r.
        auto pointAt = [centre] (float a, float r) { return Point<float> (centre.x + std::sin (a) * r, centre.y - std::cos (a) * r); };

        // Recessed well
        g.setColour (P::bezel());
        g.fillEllipse (Rectangle<float> (centre.x - outerR, centre.y - outerR, outerR * 2.0f, outerR * 2.0f));

        // Scale ticks with major/minor marks
        g.setColour (P::silkDim());
        for (int i = 0; i <= 10; ++i)
        {
            const float a = rotaryStartAngle + (rotaryEndAngle - rotaryStartAngle) * (float) i / 10.0f;
            g.drawLine (Line<float> (pointAt (a, outerR * 0.98f), pointAt (a, outerR * (i % 5 == 0 ? 0.80f : 0.88f))), 1.0f);
        }

        // Value arc in oxide
        Path valueArc;
        valueArc.addCentredArc (centre.x, centre.y, outerR * 0.90f, outerR * 0.90f, 0.0f, rotaryStartAngle, angle, true);
        g.setColour (slider.isEnabled() ? P::oxide() : P::silkDim());
        g.strokePath (valueArc, PathStrokeType (2.0f));

        // Knurled skirt
        const float skirtR = outerR * 0.78f;
        g.setColour (P::chassisDeep());
        g.fillEllipse (Rectangle<float> (centre.x - skirtR, centre.y - skirtR, skirtR * 2.0f, skirtR * 2.0f));
        constexpr int knurls = 64;
        for (int i = 0; i < knurls; ++i)
        {
            const float a = (float) i / (float) knurls * MathConstants<float>::twoPi;
            g.setColour (i % 2 ? P::highlight().withAlpha (0.35f) : Colours::black.withAlpha (0.55f));
            g.drawLine (Line<float> (pointAt (a, skirtR * 0.96f), pointAt (a, skirtR * 0.86f)), 1.0f);
        }

        // Fluted cap
        const float capR = skirtR * 0.82f;
        ColourGradient capGrad (Colour (0xff58595c), centre.x, centre.y - capR,
                                Colour (0xff242527), centre.x, centre.y + capR, false);
        g.setGradientFill (capGrad);
        g.fillEllipse (Rectangle<float> (centre.x - capR, centre.y - capR, capR * 2.0f, capR * 2.0f));

        constexpr int flutes = 24;
        for (int i = 0; i < flutes; ++i)
        {
            const float a = (float) i / (float) flutes * MathConstants<float>::twoPi;
            g.setColour (Colours::black.withAlpha (0.45f));
            g.drawLine (Line<float> (pointAt (a, capR * 0.30f), pointAt (a, capR * 0.95f)), 1.2f);
            g.setColour (P::highlight().withAlpha (0.20f));
            g.drawLine (Line<float> (pointAt (a + 0.045f, capR * 0.30f), pointAt (a + 0.045f, capR * 0.95f)), 0.8f);
        }

        g.setColour (Colours::black.withAlpha (0.5f));
        g.drawEllipse (Rectangle<float> (centre.x - capR, centre.y - capR, capR * 2.0f, capR * 2.0f), 1.0f);

        // Etched white notch indicator (dark etch shadow offset, then silk line)
        const float notchInner = capR * 0.22f;
        const float notchOuter = capR * 0.88f;
        g.setColour (Colours::black.withAlpha (0.6f));
        g.drawLine (Line<float> (pointAt (angle + 0.02f, notchInner), pointAt (angle + 0.02f, notchOuter)), 2.2f);
        g.setColour (P::silk());
        g.drawLine (Line<float> (pointAt (angle, notchInner), pointAt (angle, notchOuter)), 2.2f);
    }

    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int, int, int, int, juce::ComboBox& box) override
    {
        using namespace juce;
        using P = maya::industrial::Palette;

        auto b = Rectangle<float> (0.0f, 0.0f, (float) width, (float) height);
        g.setColour (P::bezel());
        g.fillRoundedRectangle (b, 3.0f);
        g.setColour (box.findColour (ComboBox::backgroundColourId).withAlpha (isButtonDown ? 0.8f : 1.0f));
        g.fillRoundedRectangle (b.reduced (1.0f), 2.5f);
        g.setColour (P::highlight());
        g.drawLine (b.getX() + 3.0f, b.getBottom() - 1.0f, b.getRight() - 3.0f, b.getBottom() - 1.0f, 1.0f);

        Path arrow;
        const float ax = (float) width - 16.0f;
        const float ay = (float) height * 0.5f - 2.5f;
        arrow.addTriangle (ax, ay, ax + 7.0f, ay, ax + 3.5f, ay + 5.0f);
        g.setColour (P::oxide());
        g.fillPath (arrow);
    }

    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds (box.getLocalBounds().reduced (8, 1).withTrimmedRight (18));
        label.setFont (silkFont (11.0f));
    }
};
