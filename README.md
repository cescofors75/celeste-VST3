# CELESTE Parallel — September 2026

Native stereo effect for Windows x64 and macOS Universal (Apple Silicon + Intel), with optional **Tang Control** for CELESTE FPGA.

- Native DSP: dual delay in series/parallel, wavefolder/filter, motion and space; five presets.
- Tang Control: 44 automatable hardware parameters, explicit READ TANG / SEND SESSION, routes, bypass, mute and solo. USB carries control and telemetry, not the analog audio return.
- Windows VST3 and macOS VST3/AU build and host checks pass. The final Windows C++ controller was tested against the real Tang Nano 20K.
- macOS serial hardware, commercial DAW GUI and analog return remain untested. Mac binaries use an ad-hoc development signature, without notarization. Intermittent FPGA ADC clock losses remain an open hardware issue.

[Downloads and installation](https://cesco.dev/plugins/celeste-parallel) · [Tang Control guide](vst3/TANG-CONTROL.md) · [macOS build](vst3/README-MAC.md) · [FPGA + web repository](https://github.com/cescofors75/celeste-fpga)

Validated code: `6d2f4ed6e946fffef6ec776c5f2e12d61a63c7d1` (merged through PR #1).

[Windows CI](https://github.com/cescofors75/celeste-VST3/actions/runs/36051641681) · [macOS CI](https://github.com/cescofors75/celeste-VST3/actions/runs/36051641756)
