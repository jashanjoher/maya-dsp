# Maya Chamber: Acoustic Environment Engine

## Concept
INPUT SOUND -> ENTERS A PHYSICAL/IMAGINARY ENVIRONMENT -> ENVIRONMENT CHANGES ITS REFLECTIONS, RESONANCE, DECAY AND SPECTRUM -> OUTPUT

Instead of generic room presets, the sound is placed inside another world or resonating object.

## Sound Library (24 Environments)

### 1. Natural Environments
- Deep Forest
- Cave
- Ice Cave
- Canyon
- Mountain Valley
- Underground Tunnel
Character: Organic, large, spatial reflection patterns.

### 2. Architectural Environments
- Abandoned Church
- Swimming Pool
- Concrete Atrium
- Theatre
- Factory
- Stairwell / Warehouse / Metro Station
Character: Distinct early reflection geometry and architectural acoustic signatures.

### 3. Resonant Objects
Sound resonates inside physical objects (acting as playable/tuned acoustic resonators):
- Steel Pipe
- Metal Tank
- Giant Spring
- Glass Chamber
- Wooden Box
- Instrument Bodies (Piano, Violin, Cello, Guitar)
- Giant Bell / Gong
- Concrete Tube
Character: Strong resonant modal frequencies with pitch tracking.

### 4. Impossible / Abstract Environments
Non-physical acoustic spaces with surreal modulation:
- Infinite Corridor
- Black Room
- Glass Forest
- Metal Cathedral
- Rotating Room
- Dream Chamber
- Machine Room
- Planetary Cave
Character: Diffusion loops, pitch movement, all-pass phase dispersion, dynamic decay.

## Technical Engine Architecture
Hybrid DSP Engine:
INPUT -> Pre-delay -> Early reflections (Partitioned Convolution) -> Diffusion -> Modal Resonator Banks -> Feedback Delay Network (FDN) -> Spectral Shaping -> Pitch/Modulation -> OUTPUT

### Main Musical Controls
- SIZE
- DECAY
- DENSITY
- DIFFUSION
- RESONANCE
- BRIGHTNESS
- MODULATION
- PRE-DELAY
- WIDTH
- MIX

## Asset & Licensing Discipline
1. Proprietary recordings
2. CC0 / verified commercially redistributable recordings
3. Formally licensed impulse responses
4. Synthesized/procedural raytraced impulse responses
5. In-house trained neural/NAM models

Maintain strict origin and license tracking for every included IR/model asset.
