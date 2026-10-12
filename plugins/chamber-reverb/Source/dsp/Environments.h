#pragma once
#include <juce_core/juce_core.h>

// The Maya Chamber environments from docs/REVERB_SPEC.md (28 entries in the spec's own lists).
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

// Acoustic tuning for one environment. Each field drives one DSP stage.
struct EnvironmentProfile
{
    float roomX;          // room length in metres (image-source geometry)
    float roomY;          // room width in metres
    float roomZ;          // room height in metres
    float wallAbsorption; // 0..1 average wall absorption, sets early reflection level
    float t60Scale;       // multiplies the DECAY control
    float dampingScale;   // multiplies the BRIGHTNESS damping cutoff
    float fdnScale;       // scales FDN delay line lengths with room volume
    float fundamentalHz;  // base frequency of the modal resonator bank
    float inharmonicity;  // 0 = harmonic modes, 1 = strongly stretched
    float stereoSpread;   // 0..1 lateral separation of the left and right early taps
    float tapDensity;     // 0..1 fraction of image sources kept as early reflections
};

// Returns the tuning profile for one environment.
const EnvironmentProfile& getEnvironmentProfile (Environment env);
} // namespace maya
