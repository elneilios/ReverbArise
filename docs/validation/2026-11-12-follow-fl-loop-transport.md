# FL Studio Loop transport and Perform review

12 November 2026 · Windows x64 · MSVC Debug and Release

- The owner approved the native Action Button + State Field and integrated Loop Perform layout. At 900 × 600, all three pairs fit on one row. The native review showed the disabled Set Loop Start help in the fixed footer, the matched disabled outline and the Audition button switching to a red Stop action.
- The native review checks both heading roles at 900 × 600 and 1100 × 850. The Debug VST3 and native preview built with `scripts/build.ps1` on the feature worktree. Strict OpenSpec validation passed.
- The native review uses the Rack timing fixture. In FL Studio, the owner confirmed Pattern auto-play and different reported positions when moving the parked Playlist playhead in Release build `316551c`.
- The owner confirmed in Release build `451bc77` that Set Loop Start reads the stopped Playlist playhead, the full first lead-in plays, the placed Loop repeats until Stop, and Audition has the same presentation in Song and Loop.

## Release checks

Commands use `scripts/build.ps1 -Configuration Release`.

| Arguments | Tested revision | Result |
| --- | --- | --- |
| `-PrRegression` | `8587137` | 15/15 passed, including the workflow processor and UI standards checks |
| `-TestTarget preverb_loop_playback_test -TestFilter '^(preverb_loop_playback_test\|preverb_loop_rack_replay_.*)$'` | `451bc77` | 3/3 passed, including lead-in and repeat samples and two recorded Rack timing replays |
| `-TestTarget preverb_perform_composition_preview -TestFilter '^(preverb_loop_perform_ui_test\|preverb_perform_audition_test\|preverb_perform_audition_strip_test)$'` | `451bc77` | 3/3 passed |
| `-TestTarget preverb_portable_snapshot_test` | `451bc77` with fixture and documentation edits | Passed |
| `-TestTarget preverb_boolean_toggle_test` | `8587137` with final fixture edits | Passed, including the shared control at simulated 100%, 125% and 150% scales |
| `-TestTarget preverb_processor_test` | `8587137` with final fixture edits | Passed |

The component fixture attaches to a native peer before querying accessibility and samples the outer outline at fractional scales. The processor fixtures wait for the accepted IR to become audition-ready and verify automatic maintenance of existing renders after recapture with detection off.

FL Studio verification covered the Pattern and placement workflows reported above. Declared Playlist repeats and seeks were exercised in processor fixtures. Native fractional-display DPI and a live Rack host session were outside this review.

## OpenSpec checklist verification

On `7aa11d9` with the added test fixtures, the Windows MSVC Release wrapper passed the Loop playback and recorded Rack replay checks (3/3), the Loop Perform UI check (1/1), and the standard PR regression set (15/15). The fixtures also cover omitted repeat bounds, a missing playhead position preserving the saved start, placement migration and source replacement, the final tail after Stop, restart, timing-loss fade, and the unsupported-host footer.

The remaining desktop check is to confirm that a matching Playlist repeat auto-plays across the wrap with the saved start cleared, then seek into a placed Loop and confirm the wet resumes at the destination phase. The existing FL Studio owner acceptance remains recorded above.

## PR review fixes

On `739bf2e` with the review fixes applied, the Windows MSVC Release wrapper passed Loop playback and Rack replay (3/3), the standard PR regression set (15/15) and the three Perform UI checks (3/3). The new assertions cover a queued placement joining on the first wet sample after a seek, a local or sample-zero MIDI Stop suppressing entry across that seek, and a stale stopped playhead preserving the previous saved start.
