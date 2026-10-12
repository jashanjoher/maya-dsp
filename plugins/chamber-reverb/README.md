# Maya Chamber (JUCE 8)

Hybrid acoustic environment reverb, specified in docs/REVERB_SPEC.md.

## Status: skeleton

The project builds the processor, editor, and parameter layout. DSP stages are scaffolded but not yet tuned:

- Early reflections (Source/dsp/EarlyReflections.*): JUCE partitioned convolution with a procedurally synthesised IR per environment. SIZE does nothing.
- Late tail (Source/dsp/LateFDN.*): 8-line FDN with Hadamard mixing and damping filters. Decay, brightness, and per-environment shape are wired. Density and modulation are not.
- Resonator bank (Source/dsp/ResonatorBank.*): 12 parallel band-pass modes on a harmonic series, stretched per environment. Pitch tracking is TODO.
- Environments (Source/dsp/Environments.*): all 28 environments are listed. Per-environment tuning is a first estimate, not measured.
- Neural drive (Source/dsp/NeuralDrive.*): NAM Core inference, one model per channel, placed after the FDN and before the dry/wet mix. Off by default. Parameters: Drive (0 to 24 dB), Neural Model (on/off), Neural Output (-24 to +12 dB). The default model is the embedded WaveNet A1 from NAM Core. Custom model loading and editor controls are not implemented yet.

## Build

    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release

Set -DJUCE_DIR=/path/to/JUCE to use a local JUCE 8 checkout instead of FetchContent.

The build downloads, at configure time: JUCE 8.0.4, Eigen 3.4.0, nlohmann/json 3.11.3, NeuralAmpModelerCore at commit 0b3d3c97b0859a3a8c92a8628c4dd89a25eb5842, and the example model wavenet_a1_standard.nam from that commit. It needs network access on the first configure.

## Asset discipline

Every impulse response or model must have its origin and license recorded, per docs/REVERB_SPEC.md.

- NeuralAmpModelerCore: MIT License, Copyright (c) 2023 Steven Atkinson (verified from the LICENSE file at the pinned commit).
- wavenet_a1_standard.nam: origin is sdatkinson/NeuralAmpModelerCore example_models at the pinned commit. Its own license has not been separately verified.
- Eigen 3.4.0 and nlohmann/json 3.11.3: not yet recorded here.
