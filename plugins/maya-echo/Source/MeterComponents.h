#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <cmath>

#include "ui/CuteLookAndFeel.h"   // from plugins/chamber-reverb/Source/ui

namespace maya::echo::meters
{
constexpr float kVuMinDb = -20.0f;
constexpr float kVuMaxDb = 3.0f;
constexpr float kVuSweep = 0.8f;   // needle sweep, radians either side of centre (about 46 degrees)
constexpr float kLedMinDb = -36.0f;
constexpr float kLedMaxDb = 3.0f;
constexpr int kLedSegments = 18;
} // namespace maya::echo::meters

// Retro backlit analog VU meter. The needle is driven by setLevelDb(), which the editor calls from its timer.
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

        auto bounds = getLocalBounds().toFloat().reduced (4.0f);
        constexpr float radius = 12.0f;

        // Warm backlight glow behind the face.
        juce::ColourGradient glow (juce::Colour (0x66ff8fd0), bounds.getCentreX(), bounds.getBottom(),
                                   juce::Colours::transparentBlack, bounds.getCentreX(), bounds.getY() - 40.0f, true);
        g.setGradientFill (glow);
        g.fillRoundedRectangle (bounds.expanded (6.0f), radius + 6.0f);

        // Cream face with a slight vertical gradient.
        juce::ColourGradient face (juce::Colour (0xfffff6e8), bounds.getCentreX(), bounds.getY(),
                                   juce::Colour (0xffeedcc0), bounds.getCentreX(), bounds.getBottom(), false);
        g.setGradientFill (face);
        g.fillRoundedRectangle (bounds, radius);

        const auto ink = juce::Colour (0xff3b2a33);
        const float pivotX = bounds.getCentreX();
        const float pivotY = bounds.getBottom() - 14.0f;
        const float scaleR = pivotY - bounds.getY() - 26.0f;

        auto angleFor = [] (float db) { return juce::jmap (db, kVuMinDb, kVuMaxDb, -kVuSweep, kVuSweep); };
        auto pointAt = [&] (float angle, float r) {
            return juce::Point<float> (pivotX + r * std::sin (angle), pivotY - r * std::cos (angle));
        };

        // Scale arc and red zone (0 dB and above).
        juce::Path arc;
        arc.addCentredArc (pivotX, pivotY, scaleR, scaleR, 0.0f, -kVuSweep, kVuSweep, true);
        g.setColour (ink);
        g.strokePath (arc, juce::PathStrokeType (1.5f));

        juce::Path redZone;
        redZone.addCentredArc (pivotX, pivotY, scaleR, scaleR, 0.0f, angleFor (0.0f), kVuSweep, true);
        g.setColour (juce::Colour (0xffd9405f));
        g.strokePath (redZone, juce::PathStrokeType (4.0f));

        // Ticks and labels.
        static constexpr std::array<float, 7> ticks { -20.0f, -10.0f, -7.0f, -5.0f, -3.0f, 0.0f, 3.0f };
        g.setFont (CuteLookAndFeel::uiFont (11.0f, true));
        for (auto db : ticks)
        {
            const float a = angleFor (db);
            g.setColour (db >= 0.0f ? juce::Colour (0xffd9405f) : ink);
            g.drawLine (juce::Line<float> (pointAt (a, scaleR), pointAt (a, scaleR - 9.0f)), 1.5f);

            const auto labelCentre = pointAt (a, scaleR - 20.0f);
            const auto text = db > 0.0f ? "+" + juce::String ((int) db) : juce::String ((int) db);
            g.drawText (text, juce::Rectangle<float> (labelCentre.x - 14.0f, labelCentre.y - 7.0f, 28.0f, 14.0f),
                        juce::Justification::centred, false);
        }

        g.setColour (ink);
        g.drawText ("VU", juce::Rectangle<float> (bounds.getX() + 10.0f, bounds.getBottom() - 26.0f, 40.0f, 14.0f),
                    juce::Justification::left, false);

        // Needle and pivot cap.
        const float a = angleFor (levelDb);
        g.setColour (juce::Colour (0xffd9405f));
        g.drawLine (juce::Line<float> (juce::Point<float> (pivotX, pivotY), pointAt (a, scaleR - 4.0f)), 2.0f);
        g.fillEllipse (pivotX - 6.0f, pivotY - 6.0f, 12.0f, 12.0f);

        // Glass sheen across the top-left.
        juce::ColourGradient sheen (juce::Colours::white.withAlpha (0.35f), bounds.getX(), bounds.getY(),
                                    juce::Colours::transparentWhite, bounds.getCentreX(), bounds.getCentreY(), false);
        g.setGradientFill (sheen);
        g.fillRoundedRectangle (bounds, radius);

        g.setColour (juce::Colour (0xff6b4a3a));
        g.drawRoundedRectangle (bounds, radius, 2.0f);
    }

private:
    float levelDb = maya::echo::meters::kVuMinDb;
};

// Vertical LED peak ladder for the NAM stage, with a clip LED above it.
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

        auto bounds = getLocalBounds().toFloat().reduced (2.0f);
        g.setColour (juce::Colour (0xff2b1f2e));
        g.fillRoundedRectangle (bounds, 8.0f);

        // Clip LED: lit when the NAM stage peaks at or above 0 dBFS (held by the processor).
        auto clipArea = bounds.removeFromTop (24.0f).reduced (6.0f, 4.0f);
        const bool clipping = levelDb >= 0.0f;
        g.setColour (clipping ? juce::Colour (0xffff3b5c) : juce::Colour (0xff4a2f3f));
        g.fillRoundedRectangle (clipArea, 4.0f);
        g.setColour (juce::Colours::white.withAlpha (clipping ? 1.0f : 0.5f));
        g.setFont (CuteLookAndFeel::uiFont (10.0f, true));
        g.drawText ("CLIP", clipArea, juce::Justification::centred, false);

        auto ledArea = bounds.reduced (10.0f, 8.0f);
        const float segH = ledArea.getHeight() / (float) kLedSegments;
        const float span = kLedMaxDb - kLedMinDb;

        for (int i = 0; i < kLedSegments; ++i)
        {
            const float lowerDb = kLedMinDb + span * (float) i / (float) kLedSegments;
            const bool lit = levelDb >= lowerDb;
            const auto colour = lowerDb >= 0.0f ? juce::Colour (0xffff3b5c)
                              : lowerDb >= -12.0f ? juce::Colour (0xffffd166)
                              : juce::Colour (0xff7bd389);

            juce::Rectangle<float> seg (ledArea.getX(), ledArea.getBottom() - (float) (i + 1) * segH + 1.0f,
                                        ledArea.getWidth(), segH - 2.0f);
            g.setColour (lit ? colour : colour.withAlpha (0.15f));
            g.fillRoundedRectangle (seg, 2.0f);
        }
    }

private:
    float levelDb = maya::echo::meters::kLedMinDb;
};
