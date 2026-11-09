# Tasks

## 1. Accepted source and persistence

- [x] 1.1 Extend the accepted capture snapshot with Song or Loop kind and verified Loop timing. Reuse managed references and bounded reads. Test contiguous phrase-local coverage, malformed bounds, exact revision identity, and the separation of Song and Loop coordinates.
- [x] 1.2 Version and validate the accepted record and next-capture preference independently. Test old Song state, Loop save/reopen, save while a replacement is pending, invalid or missing assets, disarmed restore and independent instances. Document the contract handed to #129.

## 2. Capture and settlement

- [x] 2.1 Admit bounded MIDI and timing observations through #123's contract without altering Song admission. Port the spike's sample-boundary cases to the approved channel and roles. Test valid and invalid offsets, coincident events, missing authority, reset, rate/tempo changes, callback interruption and route isolation.
- [x] 2.2 Implement one armed Duration and qualified End signal lifecycle. Verify exclusive musical ends for Bars and Beats, held-high input, mid-bar and on-bar end requests, budget and queue exhaustion, and late or missing markers. Test that raw Rack gate release remains unavailable and that duplicate starts and ends cannot repeat a capture. Check for callback allocations and blocking work.
- [x] 2.3 Stage and publish only a verified current take through the managed worker transaction. Test stale and cancelled completions, rejected or silent audio, storage failure, Song/Loop replacement, revision and IR dependency fencing, and continued availability of the prior source and arrangement after failure.

## 3. Learn presentation

- [x] 3.1 Integrate the Loop Mode and Arm toggles, method and duration into the existing Learn Source Capture surface using approved shared controls and the #116 mockups. Test actual bindings, accepted versus pending display, disabled reasons, quantised end wording, capture status and existing Song Learn behavior. Run `scripts/check-ui-standards.ps1`.
- [ ] 3.2 Launch the native Learn preview for owner review at 900×600 and 100%/125% user scale. Record appearance and interaction approval and any remaining host/DPI limitations. Obtain approval before adopting any new control family, token or styling exception.

## 4. Integration verification

- [x] 4.1 Build and test Windows Release through `scripts/build.ps1 -Configuration Release -Test`. Validate a supported Rack Duration take and its saved managed audio, record tested settings and distinguish hosted evidence from fixtures and native tests. Record Song regression results and remaining host checks. Update the technical guidance and offline user guide currency, then validate the OpenSpec change.
