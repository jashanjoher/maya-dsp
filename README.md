# Maya DSP Plugin Suite & Acoustic Environments Engine

Private home for the Maya DSP plugin suite.

## Layout

- plugins/: one folder per plugin (for example the SonicWaveshaper waveshaper, and the reverb plugin).
- shared/: code and assets used by more than one plugin (DSP helpers, UI components, presets).
- docs/REVERB_SPEC.md: the acoustic environment specification for the reverb engine.

## Architecture

To be written. The reverb spec describes a hybrid convolution and FDN engine.

## Roadmap

- [ ] Finalize the reverb spec in docs/REVERB_SPEC.md
- [ ] Move shared DSP code into shared/
- [ ] Reverb plugin skeleton in plugins/
- [ ] Bundle packaging and licensing rules
