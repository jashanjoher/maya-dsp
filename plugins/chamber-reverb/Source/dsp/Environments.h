#pragma once
#include <juce_core/juce_core.h>

// The 24 Maya Chamber environments from docs/REVERB_SPEC.md.
namespace maya
{
enum class EnvironmentCategory { Natural, Architectural, ResonantObject, Impossible };

enum class Environment
{
    // 1. Natural
    DeepForest, Cave, IceCave, Canyon, MountainValley, UndergroundTunnel,
    // 2. Architectural
    AbandonedChurch, SwimmingPool, ConcreteAtrium, Theatre, Factory, StairwellWarehouseMetro,
    // 3. Resonant objects
    SteelPipe, MetalTank, GiantSpring, GlassChamber, WoodenBox, InstrumentBody,
    GiantBellGong, ConcreteTube,
    // 4. Impossible / abstract
    InfiniteCorridor, BlackRoom, GlassForest, MetalCathedral, RotatingRoom,
    DreamChamber, MachineRoom, PlanetaryCave,
    Count
};

struct EnvironmentInfo
{
    juce::String name;
    EnvironmentCategory category;
};

// Returns metadata for one environment.
EnvironmentInfo getEnvironmentInfo (Environment env);
} // namespace maya
