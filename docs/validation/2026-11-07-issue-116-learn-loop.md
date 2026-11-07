# Issue #116 Learn Loop host evidence

## Rack integration

VCV Rack Free 2.6.6, Host-FX 2.1.0 and Fundamental 2.6.4 on Windows x64 at 48 kHz. The isolated Rack profile loaded a copy of the first native Release ReverbArise VST3 preview, binary SHA-256 `274b31ec312a1fdf38930aac128d71c2c34395d0e896651ccd4c46a783c625fb`. The preview was built from the issue worktree before the integrated Source Capture layout and Loop waveform. The Song ruler reported unavailable timing in Rack. Both test patches mapped Note Gate D0 to MIDI note 14.

| Fixture | Stereo WAV frames | Peaks (L / R) | SHA-256 |
| --- | ---: | --- | --- |
| Boundary LFOs | 95,904 | 1.0 / 1.0 | `60741ee7e211ff081881b35e867ea5345b9a87ea5bbfa8219004ed1ec7ac5c8e` |
| SEQ 3 and VCO | 382,975 | 0.50 / 0.50 | `2f1788fef434c08b931b085b19d74ef81afc472ffdbfbfa5b0ab7b6e17a4c6c0` |

The synth WAV is byte-identical to the prior traced probe's WAV. The boundary take has the same length as the traced probe, a high left gate through the last frame and 48-frame pulses on the right. The free-running right oscillator began at a different phase, so this comparison does not establish an exact match to the earlier trace. Closing and reopening the VST3 editor retained the accepted boundary take. **Show captured WAV** selected the file in Explorer. The synth fixture still had only its first accepted WAV after a second arm was cancelled.

Stuart's full patch, physical DPI variations, FL Studio project rollback, and the final packaged binary require separate host evidence.

The integrated preview, binary SHA-256 `e164d2e99c0008aa65bf5806839e9a46e659910ffb24eb237da27fdb76afe111`, reopened the saved Rack fixture, verified the same 7.979-second WAV, and displayed its waveform and duration. The candidate and previous shared bundle had matching VST3 class IDs, names and categories. Host-FX's power switch left the previous WAV available and the armed status waiting for a gate edge while powered off. The VST3 cannot infer that processing stopped when the host sends no callbacks. Cancel before switching Host-FX off or unplugging the gate during a take.

## Windows Release and focused fixtures

The native Release wrapper completed after rebuilding all affected binaries from the same headers. Eight focused Release CTest checks passed for the Loop recorder, Learn bindings, existing Song processor workflow, portable assets, snapshots, recovery, guide currency and build version. A separate native Learn check exercised the 900×600 composition at both supported UI scales, 100% and 125%. The Loop fixture measured exact stereo samples across both same-block and split-block gate boundaries and checked callback allocations, stale settlement, rate and reset interruption, cancellation, storage failure and corrupt or missing WAVs. These fixtures do not establish FL Studio host acceptance.
