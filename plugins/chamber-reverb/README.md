# Maya Chamber (JUCE 8)

Hybrid acoustic environment reverb, specified in docs/REVERB_SPEC.md.

## Status: skeleton

The project builds the processor, editor, and parameter layout. DSP stages are scaffolded but not yet tuned:

- Early reflections (Source/dsp/EarlyReflections.*): JUCE partitioned convolution is wired. IRs are not loaded yet, and SIZE does nothing.
- Late tail (Source/dsp/LateFDN.*): 8-line FDN with Hadamard mixing and damping filters. Decay and brightness are wired. Density and modulation are not.
- Resonator bank (Source/dsp/ResonatorBank.*): 12 parallel band-pass modes on a harmonic series. Per-environment mode tables and pitch tracking are TODO.
- Environments (Source/dsp/Environments.*): all 24 environments are listed. Per-environment parameter presets are TODO.

## Build

    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release

Set -DJUCE_DIR=/path/to/JUCE to use a local JUCE 8 checkout instead of FetchContent.

## Asset discipline

Every impulse response or model must have its origin and license recorded, per docs/REVERB_SPEC.md.
