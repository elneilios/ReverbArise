# Tasks

## 1. Shared capture authorization

- [ ] 1.1 Route Song and Loop through an explicit one-take reservation. Extend workflow/processor fixtures for fresh, restored and re-entered Learn during playback, arm before Play and during compatible playback, duplicate requests, settlement, cancellation, stale commands and instance isolation.
- [ ] 1.2 Preserve Song additive/mapped-silence behavior and pin Recapture Audio permission. Extend capture transaction coverage for partial overlaps, disarm during recording and a second Play after Stop. Exercise existing Loop timing, external arm, cancellation, finish and replacement protection with the shared control binding.
- [x] 1.3 Update workflow and capture guidance to match the new Arm, DAW Stop and Recapture Audio semantics. Review the guidance against the state transitions for both modes.

## 2. Explicit Learn rebuild

- [ ] 2.1 Use the pinned explicit or Stop policy in the existing generation transaction. Extend planner tests for lower sensitivity, deletion of the entire previous set, frozen collisions, legacy/manual/edited and out-of-coverage unfrozen definitions, empty outcomes and the existing automatic Stop protection rules.
- [ ] 2.2 Verify generation authority and asset safety in coordinator and processor fixtures. Cover stale candidates, capture arming, restore, overlapping render completion, failed/cancelled rebuild, retired jobs, frozen render identity, valid selection and save/reopen. Exercise accepted Song and Loop, including Loop with continuing transport.
- [x] 2.3 Update the Learn generation and source-context guidance. Verify it explains frozen-only retention, explicit rediscovery after deletion and automatic Stop behavior.

## 3. Learn controls and native review

- [ ] 3.1 Bind the shared Arm latch and status at the top of Source Capture and update idle and in-progress guidance. Add component/binding coverage for stable placement, Song and Loop routing, keyboard/pointer activation, disabled feedback and unchanged Recapture permission semantics.
- [ ] 3.2 Prepare and directly launch one native preview for owner review of the shared Arm placement and #178's taller primary action and icon. Show the shared button on both Learn tabs, both labels, the single helper line, footer hover help, keyboard focus and disabled and busy states. Check the minimum size at 100% and 125% UI scale where supported. Record the owner's decision and actual scale coverage before adoption.
- [ ] 3.3 Adopt approved shared command presentation, update the production style guide and affected standards revision, and add component coverage for accessible naming, focus, bounds, readiness and equivalent explicit command routing. Verify that Sound remains reachable with no clipping or layout regression.
- [x] 3.4 Update and review the offline user guide and its currency record for shared Arm, Recapture Audio, frozen-only rebuilding and the approved Learn presentation.
- [ ] 3.5 Verify the visible Learn tab and Sound editor edges for #177 with Loop Mode on and off at compact 100% and 125% where supported. Check the Custom IR row spacing and keyboard focus, and retain the shared Sound editor's Perform presentation.

## 4. Integrated verification

- [ ] 4.1 Follow the contribution test cadence. After owner approval to mark the draft ready, run focused Windows wrapper/CTest checks and the standard PR regression set, including the UI standards checker. Record revisions, commands, results and limitations with native and fixture results distinguished.
- [ ] 4.2 Consolidate FL Studio checks into one brief smoke test for a fresh insert during PAT playback, intentional capture and lower-sensitivity rebuilding from the same accepted source after deleting swells. Record actual Song/Loop and DPI coverage, including any unverified combinations.
