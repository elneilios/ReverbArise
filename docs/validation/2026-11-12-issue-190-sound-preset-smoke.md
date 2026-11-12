# Sound Preset foundation smoke results

12 November 2026 · [Issue #190](https://github.com/elneilios/ReverbArise/issues/190) · `codex/190-sound-preset-foundation` working diff based on `a2096fd`.

The Windows MSVC Debug VST3 compiled and all six focused native smoke checks passed. Each check used `./scripts/build.ps1 -Configuration Debug -TestTarget <target>`. The build tree was already configured. The wrapper reused shared libraries and compiled the selected test targets.

| Outcome checked | Native test target | CTest elapsed |
| --- | --- | ---: |
| Existing gain and exact IR identity survive migration and reopen. Invalid new metadata leaves the accepted session intact. | `preverb_swell_session_test` | 0.35 s |
| Learn Level and preset are pinned for a capture pass. Locked or obsolete edits leave that configuration intact. | `preverb_generation_configuration_test` | 0.26 s |
| New automatic swells inherit the Level default. Matching an existing swell preserves its Level and identity. | `preverb_automatic_generation_test` | 0.20 s |
| The previous response stays available through a swap or failure. A shorter response fits around the same arrival, and a new manual swell inherits Learn Level. | `preverb_ir_handover_test` | 32.64 s |
| Sound metadata alone leaves the Export composition identity unchanged. | `preverb_song_export_test` | 14.96 s |
| Portable recovery preserves the accepted Sound, future Level and frozen swell. | `preverb_portable_recovery_test` | 12.73 s |

## Release PR regression

`./scripts/build.ps1 -Configuration Release -PrRegression` passed **15 of 15** at `c1991007d397fdca5e09c0099c8d158324a82a10`. CTest elapsed time was 11.91 s after the first Release compilation in this worktree. The six Debug checks above cover focused native fixtures, including simulated host callbacks.

Four additional Debug checks passed on the same implementation. The wrapper built each target in the configured tree.

| Additional coverage | Target and case | CTest elapsed |
| --- | --- | ---: |
| Creative versions 1–12 and transactional rejection of unsupported versions | `preverb_storage_audit` with `-TestFilter '^preverb_storage_audit_compatibility$'` | 0.11 s |
| Bundled choices and retained managed imports | `preverb_ir_choices_test` | 0.56 s |
| Learn Generate, Stop, supersession and protected swells | `preverb_learn_generation_test` | 65.18 s |
| Selected-swell, rendering and Reset All engine behavior | `preverb_swell_engine_test` | 97.44 s |

## FL Studio 2026 host smoke

The repository installer built and installed the Release VST3 at the system path and retained the previous bundle under `out/vst3-install-backups/20261112-150501-1259bdb3e420477ca01a0f2c3cda17c5`. The installed binary SHA-256 matched the Release output `A312956D1DB8A55EEB1F2150E0811526D6348569B19DC9E6A861706A621F86A8`.

- FL Studio 26.1.7.5654 opened a copy of the #183 test project with creative v11 state. Saving the copy wrote v12 for all four instances. The original project was used as the read-only comparison.
- Compared 215 swells by persistent ID. Exact IR and full-render references, arrival, gain and other shape values, wet timing intent and activity matched the v11 project. Capture references and default IRs matched, and each new future Level default was 0 dB.
- Manual Audition showed the running cursor and mixer output. Song playback advanced the DAW and plug-in cursors with mixer output. Stop restored Ready. A fresh FL Studio process reopened the v12 copy with the Piano instance in Perform, the custom response `06_Airy_Vowel.wav` ready and its arrival still at bar 3.1.
- The host smoke covered migration, playback and reopen. The automation lost focus when targeting the native IR popup, so the host swap attempt was inconclusive. The focused native handover test verified live/Audition swaps, the shorter-IR fit, failure retention and the offline snapshot.
