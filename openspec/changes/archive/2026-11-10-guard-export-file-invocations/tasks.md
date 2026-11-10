# Tasks

## 1. Gesture and invocation ownership

- [x] 1.1 Extend the existing `FileDragTile` gesture handling in isolation. Verify a primary press can cross the current threshold once, a right-button or unrelated pointer cannot claim it, release cancels the original token, and a completed drag suppresses only its own pointer Save. Exercise fresh keyboard and accessibility Save and retain the approved visual variant.
- [x] 1.2 Prepare the Export binding with the real Save continuation and verified-drag handoff behind controllable chooser and OS seams. Verify the captured session, pass and file are checked at dispatch and completion. Cover revision changes before the threshold or queued Save, chooser cancellation, owner teardown, stale delivery, release then re-press, and exactly-once current drag. Check that rejected invocations do not write or replace a file.
- [x] 1.3 Open the native JUCE review and record owner approval of immediate ready-WAV drag, WAV and stop cursors over simulated accepting and rejecting targets, and dotted-border restoration. Distinguish the fixture from an actual FL Studio drop.

## 2. Export rollout

- [x] 2.1 Adopt the guarded binding for Save WAV, the tile's Save action and immediate external drag. Verify ready-path ownership and absence of a second full-file scan, stale and missing file rejection, exactly-once handoff, Windows copy payload and target cursor mapping, and dotted-border restoration on drop, cancellation and failure. Verify Save chooser overwrite confirmation and fallback.
- [x] 2.2 Record the adopted E2 and Export N3 bindings, tests and retained native variant in the C8 inventory and production guidance. Review the offline user guide against the delivered Save and drag behavior and verify guide currency. Keep the other P8 scopes and P7 lists and scrolling visible in the roadmap.

## 3. Integrated verification and review

- [x] 3.1 Run `scripts/build.ps1 -Configuration Release -Test`, `scripts/check-ui-standards.ps1`, `openspec validate guard-export-file-invocations --strict` and `git diff --check`. Record native/automated results and actual host and physical DPI coverage separately in dated evidence.
- [x] 3.2 Prepare the review PR with `Refs #107`, the scoped native approval, the covered delivery paths and validation results.
