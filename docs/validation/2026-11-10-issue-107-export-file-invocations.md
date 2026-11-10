# #107 P8 Export file invocations

Windows x64 with MSVC and JUCE, on `codex/107-export-file-invocations` based on
`7f39588`. Change `guard-export-file-invocations`.

## Owner review

The owner approved the native JUCE Export fixture on 10 November 2026. The
fixture let the owner move between accepting and rejecting targets, release
over either target and cancel, and see the dotted border return. Its targets
simulate the drag results.

## Production and automated verification

- Export's preparation worker verifies and publishes the ready WAV before the
  tile enables. The drag binding pins the file, session and pass, checks that
  the original primary press is still active, and hands the ready path to
  Windows without reading WAV contents again.
- `preverb_swell_ui_test` covers the original gesture, readiness changes,
  release and re-press before handoff, missing paths, one handoff per press,
  the solid-to-dotted pointer border transition, and keyboard/accessibility
  Save fallback. Its Export workspace case uses the real engine with a
  controlled chooser and native handoff, verifies that drag does not queue
  delivery, and checks cancelled Save and a byte-identical Save copy.
- `preverb_song_export_engine_test` checks the ready path against the file,
  session and pass and exercises existing Save overwrite and integrity guards.
  The native chooser keeps JUCE's `warnAboutOverwriting` flag.
- A separate Windows Release check of the production adapter passed. The Shell
  `CF_HDROP` data contained exactly the ready WAV path. The native
  `IDropSource` used the WAV cursor for copy and the Windows stop cursor for
  rejected targets, switched back to WAV on acceptance, and returned the
  expected result on release, Escape, secondary-button cancel and stale scope.
  Cursor creation passed at 96 and 120 logical DPI.
- `scripts/build.ps1 -Configuration Release -Test` passed all 99 CTest cases
  after the editor fixture was corrected to dispatch the JUCE message queue.
  The final run reused the configured Windows build tree and took 300.07 s
  in CTest. Local logs are under `out/build-logs/Release/20261110-080728-3f37357e`.
- `scripts/check-ui-standards.ps1`, strict OpenSpec validation,
  `scripts/check-user-guide.ps1` and `git diff --check` passed.

## Coverage

The owner exercised simulated targets. The Windows check exercised the real
Shell payload and cursor callbacks directly. A live drop into the FL Studio
Playlist and visual inspection on a physical high-DPI display remain untested.
