#include "Environments.h"

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
} // namespace maya
