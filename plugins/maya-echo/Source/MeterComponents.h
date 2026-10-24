#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <cmath>

#include "ui/CuteLookAndFeel.h"   // from plugins/chamber-reverb/Source/ui

namespace maya::echo::meters
{
constexpr float kVuMinDb = -20.0f;
constexpr float kVuMaxDb = 3.0f;
constexpr float kVuSweep = 0.62f;  // needle sweep, radians either side of centre (about 36 degrees)
constexpr float kLedMinDb = -36.0f;
constexpr float kLedMaxDb = 3.0f;
constexpr int kLedSegments = 14;

// Bubblegum palette.
inline juce::Colour faceTop()   { return juce::Colour (0xfffff2f8); }
inline juce::Colour faceBottom(){ return juce::Colour (0xfff6b9d1); }
inline juce::Colour rim()       { return juce::Colour (0xffd9739c); }
inline juce::Colour ink()       { return juce::Colour (0xff3a1f33); }
} // namespace maya::echo::meters

// Compact pastel VU meter. The needle is driven by setLevelDb(), which the editor calls from its timer.
class VuMeter : public juce::Component
{
public:
    void setLevelDb (float db)
    {
        levelDb = juce::jlimit (maya::echo::meters::kVuMinDb, maya::echo::meters::kVuMaxDb, db);
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        using namespace maya::echo::meters;

        auto bounds = getLocalBounds().toFloat().reduced (2.0f);
        constexpr float radius = 10.0f;

        // Faceplate with a soft vertical gradient.
        juce::ColourGradient face (faceTop(), bounds.getCentreX(), bounds.getY(),
                                   faceBottom(), bounds.getCentreX(), bounds.getBottom(), false);
        g.setGradientFill (face);
        g.fillRoundedRectangle (bounds, radius);

        const float pivotX = bounds.getCentreX();
        const float pivotY = bounds.getBottom() - 8.0f;
        const float scaleR = pivotY - bounds.getY() - 14.0f;

        auto angleFor = [] (float db) { return juce::jmap (db, kVuMinDb, kVuMaxDb, -kVuSweep, kVuSweep); };
        auto pointAt = [&] (float angle, float r) {
            return juce::Point<float> (pivotX + r * std::sin (angle), pivotY - r * std::cos (angle));
        };

        // Thin scale arc in the rim colour, with the 0 VU and above zone drawn heavier.
        juce::Path arc;
        arc.addCentredArc (pivotX, pivotY, scaleR, scaleR, 0.0f, -kVuSweep, kVuSweep, true);
        g.setColour (rim().withAlpha (0.55f));
        g.strokePath (arc, juce::PathStrokeType (1.0f));

        juce::Path redZone;
        redZone.addCentredArc (pivotX, pivotY, scaleR, scaleR, 0.0f, angleFor (0.0f), kVuSweep, true);
        g.setColour (rim());
        g.strokePath (redZone, juce::PathStrokeType (2.0f));

        // Fine ticks and small labels, only at the marks that matter.
        static constexpr std::array<float, 5> ticks { -20.0f, -10.0f, -5.0f, 0.0f, 3.0f };
        g.setFont (CuteLookAndFeel::uiFont (8.0f, true));
        for (auto db : ticks)
        {
            const float a = angleFor (db);
            g.setColour (db >= 0.0f ? rim() : ink().withAlpha (0.75f));
            g.drawLine (juce::Line<float> (pointAt (a, scaleR), pointAt (a, scaleR - 5.0f)), 1.0f);

            const auto labelCentre = pointAt (a, scaleR - 12.0f);
            const auto text = db > 0.0f ? "+" + juce::String ((int) db) : juce::String ((int) db);
            g.drawText (text, juce::Rectangle<float> (labelCentre.x - 9.0f, labelCentre.y - 5.0f, 18.0f, 10.0f),
                        juce::Justification::centred, false);
        }

        g.setColour (ink().withAlpha (0.8f));
        g.setFont (CuteLookAndFeel::uiFont (8.0f, true));
        g.drawText ("VU", juce::Rectangle<float> (bounds.getX() + 6.0f, bounds.getBottom() - 14.0f, 24.0f, 10.0f),
                    juce::Justification::left, false);

        // Hairline needle with a small pivot dot.
        const float a = angleFor (levelDb);
        g.setColour (rim());
        g.drawLine (juce::Line<float> (juce::Point<float> (pivotX, pivotY), pointAt (a, scaleR - 2.0f)), 1.2f);
        g.fillEllipse (pivotX - 2.5f, pivotY - 2.5f, 5.0f, 5.0f);

        // Thin bezel.
        g.setColour (rim());
        g.drawRoundedRectangle (bounds, radius, 1.5f);
    }

private:
    float levelDb = maya::echo::meters::kVuMinDb;
};

// Slim vertical LED ladder for the NAM stage, with a clip LED above it.
class LedBarMeter : public juce::Component
{
public:
    void setLevelDb (float db)
    {
        levelDb = juce::jlimit (maya::echo::meters::kLedMinDb, maya::echo::meters::kLedMaxDb, db);
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        using namespace maya::echo::meters;

        auto bounds = getLocalBounds().toFloat().reduced (1.0f);
        g.setColour (faceTop());
        g.fillRoundedRectangle (bounds, 6.0f);
        g.setColour (rim());
        g.drawRoundedRectangle (bounds, 6.0f, 1.0f);

        // Clip LED: lit when the NAM stage peaks at or above 0 dBFS (held by the processor).
        auto clipArea = bounds.removeFromTop (14.0f).reduced (4.0f, 3.0f);
        const bool clipping = levelDb >= 0.0f;
        g.setColour (clipping ? rim() : rim().withAlpha (0.25f));
        g.fillRoundedRectangle (clipArea, 3.0f);

        auto ledArea = bounds.reduced (6.0f, 6.0f);
        const float segH = ledArea.getHeight() / (float) kLedSegments;
        const float span = kLedMaxDb - kLedMinDb;

        for (int i = 0; i < kLedSegments; ++i)
        {
            const float lowerDb = kLedMinDb + span * (float) i / (float) kLedSegments;
            const bool lit = levelDb >= lowerDb;
            const auto colour = lowerDb >= 0.0f ? rim()
                              : lowerDb >= -12.0f ? juce::Colour (0xfff08fb5)
                              : juce::Colour (0xffe9a9c6);

            juce::Rectangle<float> seg (ledArea.getX(), ledArea.getBottom() - (float) (i + 1) * segH + 1.0f,
                                        ledArea.getWidth(), segH - 1.5f);
            g.setColour (lit ? colour : colour.withAlpha (0.18f));
            g.fillRoundedRectangle (seg, 1.5f);
        }
    }

private:
    float levelDb = maya::echo::meters::kLedMinDb;
};
