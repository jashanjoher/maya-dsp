#include "Environments.h"
#include <array>

namespace maya
{
EnvironmentInfo getEnvironmentInfo (Environment env)
{
    using C = EnvironmentCategory;
    switch (env)
    {
        case Environment::DeepForest:               return { "Deep Forest", C::Natural };
        case Environment::Cave:                    return { "Cave", C::Natural };
        case Environment::IceCave:                 return { "Ice Cave", C::Natural };
        case Environment::Canyon:                  return { "Canyon", C::Natural };
        case Environment::MountainValley:          return { "Mountain Valley", C::Natural };
        case Environment::UndergroundTunnel:       return { "Underground Tunnel", C::Natural };
        case Environment::AbandonedChurch:        return { "Abandoned Church", C::Architectural };
        case Environment::SwimmingPool:            return { "Swimming Pool", C::Architectural };
        case Environment::ConcreteAtrium:          return { "Concrete Atrium", C::Architectural };
        case Environment::Theatre:                 return { "Theatre", C::Architectural };
        case Environment::Factory:                 return { "Factory", C::Architectural };
        case Environment::StairwellWarehouseMetro: return { "Stairwell / Warehouse / Metro Station", C::Architectural };
        case Environment::SteelPipe:               return { "Steel Pipe", C::ResonantObject };
        case Environment::MetalTank:               return { "Metal Tank", C::ResonantObject };
        case Environment::GiantSpring:             return { "Giant Spring", C::ResonantObject };
        case Environment::GlassChamber:            return { "Glass Chamber", C::ResonantObject };
        case Environment::WoodenBox:               return { "Wooden Box", C::ResonantObject };
        case Environment::InstrumentBody:          return { "Instrument Bodies", C::ResonantObject };
        case Environment::GiantBellGong:           return { "Giant Bell / Gong", C::ResonantObject };
        case Environment::ConcreteTube:            return { "Concrete Tube", C::ResonantObject };
        case Environment::InfiniteCorridor:        return { "Infinite Corridor", C::Impossible };
        case Environment::BlackRoom:               return { "Black Room", C::Impossible };
        case Environment::GlassForest:             return { "Glass Forest", C::Impossible };
        case Environment::MetalCathedral:          return { "Metal Cathedral", C::Impossible };
        case Environment::RotatingRoom:            return { "Rotating Room", C::Impossible };
        case Environment::DreamChamber:            return { "Dream Chamber", C::Impossible };
        case Environment::MachineRoom:             return { "Machine Room", C::Impossible };
        case Environment::PlanetaryCave:           return { "Planetary Cave", C::Impossible };
        case Environment::Count:                   break;
    }
    jassertfalse;
    return { {}, C::Natural };
}

namespace
{
// Tuning table in enum order. Fields: roomX, roomY, roomZ, wallAbsorption, t60Scale,
// dampingScale, fdnScale, fundamentalHz, inharmonicity, stereoSpread, tapDensity.
constexpr std::array<EnvironmentProfile, (size_t) Environment::Count> profiles {{
    { 40.0f, 30.0f, 20.0f, 0.85f, 0.6f, 0.5f, 1.6f,   80.0f, 0.05f, 0.9f, 0.4f },  // DeepForest
    { 25.0f, 12.0f,  8.0f, 0.35f, 1.6f, 0.8f, 1.2f,   60.0f, 0.10f, 0.6f, 0.6f },  // Cave
    { 18.0f, 10.0f,  7.0f, 0.15f, 2.2f, 1.6f, 1.0f,  220.0f, 0.15f, 0.7f, 0.5f },  // IceCave
    { 80.0f, 40.0f, 30.0f, 0.50f, 1.8f, 1.0f, 2.4f,   70.0f, 0.05f, 1.0f, 0.3f },  // Canyon
    {120.0f, 60.0f, 40.0f, 0.60f, 1.4f, 0.9f, 2.6f,   90.0f, 0.05f, 1.0f, 0.25f }, // MountainValley
    { 60.0f,  5.0f,  4.0f, 0.70f, 1.0f, 0.6f, 1.1f,  110.0f, 0.08f, 0.2f, 0.8f },  // UndergroundTunnel
    { 40.0f, 20.0f, 18.0f, 0.25f, 3.5f, 1.1f, 1.9f,  100.0f, 0.10f, 0.8f, 0.5f },  // AbandonedChurch
    { 25.0f, 12.0f,  4.0f, 0.10f, 1.2f, 1.7f, 0.9f,  300.0f, 0.20f, 0.5f, 0.7f },  // SwimmingPool
    { 30.0f, 30.0f, 25.0f, 0.10f, 2.6f, 1.4f, 1.8f,  140.0f, 0.12f, 0.8f, 0.4f },  // ConcreteAtrium
    { 20.0f, 15.0f,  8.0f, 0.40f, 1.5f, 0.9f, 1.2f,  120.0f, 0.05f, 0.6f, 0.5f },  // Theatre
    { 50.0f, 25.0f, 12.0f, 0.20f, 2.4f, 1.3f, 1.7f,  150.0f, 0.25f, 0.7f, 0.6f },  // Factory
    { 12.0f,  8.0f, 30.0f, 0.20f, 2.0f, 1.2f, 1.0f,  180.0f, 0.20f, 0.4f, 0.9f },  // StairwellWarehouseMetro
    {  6.0f,  0.3f,  0.3f, 0.05f, 3.0f, 1.8f, 0.5f,  260.0f, 0.35f, 0.2f, 0.2f },  // SteelPipe
    {  2.0f,  2.0f,  2.5f, 0.08f, 2.8f, 1.6f, 0.7f,  180.0f, 0.30f, 0.3f, 0.3f },  // MetalTank
    {  4.0f,  0.5f,  0.5f, 0.05f, 3.2f, 1.4f, 0.6f,  130.0f, 0.45f, 0.4f, 0.6f },  // GiantSpring
    {  2.0f,  2.0f,  2.0f, 0.03f, 3.8f, 2.2f, 0.6f,  520.0f, 0.20f, 0.6f, 0.4f },  // GlassChamber
    {  0.6f,  0.4f,  0.3f, 0.40f, 0.7f, 0.7f, 0.4f,  165.0f, 0.12f, 0.3f, 0.5f },  // WoodenBox
    {  0.4f,  0.2f,  0.1f, 0.50f, 0.9f, 0.8f, 0.35f, 196.0f, 0.08f, 0.4f, 0.5f },  // InstrumentBody
    {  2.0f,  2.0f,  2.0f, 0.02f, 4.5f, 2.0f, 0.8f,  330.0f, 0.50f, 0.5f, 0.3f },  // GiantBellGong
    { 10.0f,  1.5f,  1.5f, 0.10f, 2.2f, 1.5f, 0.8f,   95.0f, 0.20f, 0.2f, 0.7f },  // ConcreteTube
    {200.0f,  4.0f,  4.0f, 0.30f, 3.0f, 1.0f, 3.0f,   75.0f, 0.02f, 0.1f, 0.9f },  // InfiniteCorridor
    { 10.0f, 10.0f, 10.0f, 0.98f, 0.4f, 0.4f, 0.8f,   50.0f, 0.05f, 0.9f, 0.1f },  // BlackRoom
    { 20.0f, 20.0f, 12.0f, 0.12f, 2.8f, 2.0f, 1.4f,  440.0f, 0.30f, 0.9f, 0.7f },  // GlassForest
    { 50.0f, 30.0f, 25.0f, 0.08f, 5.0f, 1.7f, 2.2f,  210.0f, 0.25f, 0.9f, 0.4f },  // MetalCathedral
    {  9.0f,  9.0f,  4.0f, 0.30f, 1.6f, 1.2f, 1.0f,  125.0f, 0.10f, 1.0f, 0.5f },  // RotatingRoom
    { 14.0f, 14.0f, 14.0f, 0.20f, 4.0f, 0.9f, 1.5f,  285.0f, 0.15f, 1.0f, 0.6f },  // DreamChamber
    { 15.0f, 10.0f,  6.0f, 0.15f, 1.8f, 1.5f, 0.8f,  200.0f, 0.40f, 0.5f, 0.8f },  // MachineRoom
    { 30.0f, 20.0f, 15.0f, 0.20f, 4.5f, 0.6f, 2.0f,   45.0f, 0.05f, 0.7f, 0.3f },  // PlanetaryCave
}};
} // namespace

const EnvironmentProfile& getEnvironmentProfile (Environment env)
{
    return profiles[(size_t) env];
}
} // namespace maya
