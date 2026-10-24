#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <cmath>

#include "ui/CuteLookAndFeel.h"   // from plugins/chamber-reverb/Source/ui

namespace maya::echo::meters
{
constexpr float kLevelMinDb = -60.0f;
constexpr float kLevelMaxDb = 0.0f;

// Bubblegum palette.
inline juce::Colour faceTop()    { return juce::Colour (0xfffff2f8); }
inline juce::Colour faceBottom() { return juce::Colour (0xfff6b9d1); }
inline juce::Colour rim()        { return juce::Colour (0xffd9739c); }
inline juce::Colour accent()     { return juce::Colour (0xff2fb8ac); }
inline juce::Colour ink()        { return juce::Colour (0xff3a1f33); }
} // namespace maya::echo::meters

// Slim horizontal output level bar across the top. The editor feeds it dBFS values via setLevelDb().
class LevelBar : public juce::Component
{
public:
    void setLevelDb (float db)
    {
        levelDb = juce::jlimit (maya::echo::meters::kLevelMinDb, maya::echo::meters::kLevelMaxDb, db);
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        using namespace maya::echo::meters;

        auto bounds = getLocalBounds().toFloat();
        const float radius = bounds.getHeight() * 0.5f;

        juce::ColourGradient face (faceTop(), bounds.getX(), bounds.getCentreY(),
                                   faceBottom(), bounds.getRight(), bounds.getCentreY(), false);
        g.setGradientFill (face);
        g.fillRoundedRectangle (bounds, radius);

        const float norm = juce::jmap (levelDb, kLevelMinDb, kLevelMaxDb, 0.0f, 1.0f);
        auto fill = bounds.reduced (2.0f);
        fill.setWidth (juce::jmax (0.0f, fill.getWidth() * norm));
        if (fill.getWidth() > 2.0f)
        {
            g.setColour (accent());
            g.fillRoundedRectangle (fill, fill.getHeight() * 0.5f);
        }

        g.setColour (rim());
        g.drawRoundedRectangle (bounds.reduced (0.5f), radius, 1.0f);

        g.setColour (ink());
        g.setFont (CuteLookAndFeel::uiFont (9.0f, true));
        g.drawText ("OUT", bounds.reduced (10.0f, 0.0f), juce::Justification::centredLeft, false);
        g.drawText (juce::String (levelDb, 1) + " dB", bounds.reduced (10.0f, 0.0f),
                    juce::Justification::centredRight, false);
    }

private:
    float levelDb = maya::echo::meters::kLevelMinDb;
};
