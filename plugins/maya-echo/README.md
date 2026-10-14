# Maya Echo ME-24 (JUCE 8)

Dual-delay tape echo with a NAM (Neural Amp Modeler) saturation stage inside the feedback loop.

## Status: scaffold

The processor, editor, and parameter layout are in place. This has not been compiled or tuned yet.

## Signal flow

INPUT (stereo) -> per-channel fractional delay (cubic Hermite) with wow and flutter modulation on the delay time -> taps L and R -> feedback path: optional ping-pong cross -> colour (1 kHz tilt split, then environment high-cut) -> NAM NeuralDrive (drive in dB, one model per channel, default embedded wavenet_a1_standard) -> tanh soft limiter -> feedback gain (0 to 110 percent) -> summed with the input and written back into the delay line -> equal-power dry/wet mix with the taps.

The delay minimum is clamped to the block size plus 4 samples so that the NAM block processing never reads ahead of the write pointer. Reported latency is 0 samples.

## Parameters

- Time (1 to 2000 ms) and Sync plus Division (1/16 to 1/1, dotted variants). Tempo comes from the host, with 120 BPM as the fallback.
- Feedback (0 to 110 percent)
- Wow/Flutter (0 to 100 percent)
- Neural Drive (0 to 24 dB, input gain into NAM)
- Color (-1 to +1 tilt)
- Mix (0 to 100 percent, equal power)
- Ping-Pong (on or off)
- Environment (24 choices, each with a high-cut, wow scale, and right-channel offset)

## Build

    cmake -S plugins/maya-echo -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release

Set -DJUCE_DIR=/path/to/JUCE to use a local JUCE 8 checkout instead of FetchContent. The build needs network access on the first configure.

## Shared code

NeuralDrive.* and ui/CuteLookAndFeel.h are included from plugins/chamber-reverb/Source. Move them into shared/ before a third plugin depends on them.
