# Tasks

## 1. Current-build host state

- [ ] 1.1 Reuse the current host and associated-state records, fixing any mismatch between save and restore. Extend the processor round-trip fixture to compare accepted kind, source ID and revision, exact stereo audio, native rate, exclusive extent, local grid, swell edits and assets. Change the next-capture preference before save to prove it is independent.
- [ ] 1.2 Ensure restore discards armed, recording and settling replacements, permission to recapture, and pending live commands. Extend native cases for a late worker completion, held-high external controls, both Arm routes with Recapture Audio off, cancellation with permission on, and dry Standby until fresh timing and Cue.
- [ ] 1.3 Exercise editor close/reopen, suspend/bypass, multiple instances, and a current-build Song project in the affected lifecycle fixtures. Verify that instances cannot replace each other's state, and that save stays reference-based with the existing VST3 identity. Update the Loop capture state guidance to match the verified lifecycle.

## 2. Verified asset recovery

- [ ] 2.1 Reuse the existing snapshot and association flows for Loop Locate and backup Restore. Extend portable snapshot/recovery cases to compare the exact accepted dependencies after relocation, backup restoration and a second save/reopen, and to verify the local grid and dry Standby after recovery.
- [ ] 2.2 Cover missing and corrupt source or wet assets, cancelled or conflicting candidates, and stale recovery adoption in native fixtures. Verify that unresolved assets remain visible, verified survivors and edits stay intact, and incomplete recovery cannot certify a complete backup. Update the guide where Loop-specific behavior needs explanation and check its currency at the readiness gate.

## 3. Host acceptance and PR readiness

- [x] 3.1 In VCV Rack, create a new project on the candidate VST3. Capture and edit a Loop, save and reopen, then exercise Locate and verified Restore on disposable copies. Check asset identity and checksums, disarmed Learn, the restored grid, and wet playback only after fresh timing and Cue. Record host versions, rate, build and results.
- [x] 3.2 In FL Studio, repeat the save, reopen, Locate and verified Restore checks with a new Playlist project and an edited Loop. Verify a second save/reopen after recovery and record the same evidence. Record native and actual host coverage separately.
- [ ] 3.3 After owner approval for the draft-to-ready transition, run the targeted native cases and `./scripts/build.ps1 -Configuration Release -PrRegression` for the final revision. Record the results and remaining limitations in the PR targeting `feature/loop-mode`.
