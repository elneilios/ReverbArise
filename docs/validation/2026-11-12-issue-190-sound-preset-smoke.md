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

These results cover native fixtures, including simulated host callbacks. The full PR regression set and the FL Studio host smoke remain pending.
