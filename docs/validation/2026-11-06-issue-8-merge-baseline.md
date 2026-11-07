# Issue #8 / PR #31 merged baseline

Resolved `origin/main` at `717c890` into the issue branch based on `3bad735`, before applying `separate-learning-from-swell-rendering`. Preserved incoming shared IR controls, force-regeneration results/feedback and host-reactivation fixes alongside Storage/recovery state. No recovery-only prototype stash was applied.

`scripts/build.ps1 -Configuration Release -Test` configured and compiled successfully using the existing populated MSVC tree. The first complete test run passed 82/84 in 342.39 s. Logs: `out/build-logs/Release/20261106-170232-cdbe44af/` (local, ignored).

The incoming About test assumed six Settings tabs; this branch adds Storage. The fixture now locates About by its actual tab name rather than a hardcoded count/index. Its focused native rebuild and CTest passed 1/1 in 5.45 s. Thus 83 baseline tests have passing results.

The remaining `preverb_perform_knob_reset_test` fails before opening its real popup: native peer focus is true, but Windows foreground-process activation is false. Isolated retries reproduce that prerequisite failure. Its assertions are retained. This is an unresolved native interaction coverage limitation, not a passing full wrapper or a waived final gate. No system focus policy or production control was changed.

UI standards passed (66 files, 59 bounded occurrences); merged guide semantic review/read-only currency and whitespace checks passed. The first full Release build compiled previously unused test runners; subsequent work reuses those outputs. This evidence is distinct from later engine tests, FL Studio checks and separate-profile restoration.

Association, writer migration, deletion/cleanup, compression, release, canonical sync/archive and PR merge remain gated. No system VST3 installation was performed.
