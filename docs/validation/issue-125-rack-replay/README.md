# Measured Rack timing replay

These compact fixtures preserve sample indices from the #123 alignment traces collected in Rack Free 2.6.6 with Host-FX 2.1.0, CLKD 2.5.0 and Fundamental 2.6.4. Both use 48 kHz. The 137 BPM run used 128-frame callbacks and the 280 BPM run used 256-frame callbacks. See the [alignment evidence](../2026-11-09-issue-123-rack-timing.md).

The source trace SHA-256 values were `1584b8f6ea64828cff74d7ccd3593a5a7ae74635269a769d6df62ef177a9fab3` at 137 BPM and `aa15047941e8c89988b5e003bcaecc267964c28a860bd640cb27b274bf306ec9` at 280 BPM. Extract positive `beat`, `bar`, `phrase`, `cue`, `stop`, `reset-signal` and `left` rows. Group them at the original sample index and OR their flags in that order, starting at bit 0. The `left` electrical rising edge is the dry phrase marker. All phrase rows in these fixtures have the same sample index as the dry marker.

CTest replays the compact fixtures through production `LoopPlayback` and compares the wet sum with an independent placement oracle using the measured marker samples. It also withholds one phrase observation while leaving its dry edge in the test input. This combines real host input timing with deterministic wet and dry signals. Hosted editor, hardware monitoring and FL Studio remain separate acceptance cases.

| Trace | Cue launches | Checked aligned downbeats | Peak absolute sample error |
| --- | ---: | ---: | ---: |
| 137 BPM | 2 | 4 | `1.49012e-08` |
| 280 BPM | 3 | 11 | `1.49012e-08` |

Both Release replays passed with unchanged dry contribution, complete lead-in, summed wet overlap, a missing-marker stop, and a fresh Cue after re-lock. The traces exercise a shared-clock 4/4 setup with a recurring four-bar phrase. The synthetic scheduler and processor tests cover other admitted phrase lengths and unsupported configurations.

## Windows Release validation

On 9 November 2026, `scripts/build.ps1 -Configuration Release -Test` configured and built the implementation in `43fbf75` with the existing MSVC build tree. CTest passed 100 of 101 cases in 360.89 seconds. The remaining test, `preverb_perform_readability_test`, timed out at the Windows foreground-focus check previously recorded in the [#123 baseline](../2026-11-09-issue-123-rack-timing.md#native-song-regression). `ctest -C Release --rerun-failed --output-on-failure` passed that test on the same build in 20.73 seconds. The logs are in `out/build-logs/Release/20261109-234450-f664210b/`.
