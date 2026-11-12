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

At this stage the remaining desktop check was the matching Playlist repeat. The processor and playback fixtures verified that wet resumes at the destination phase after seeking into a placed Loop.

## PR review fixes

On `739bf2e` with the review fixes applied, the Windows MSVC Release wrapper passed Loop playback and Rack replay (3/3), the standard PR regression set (15/15) and the three Perform UI checks (3/3). The new assertions cover a queued placement joining on the first wet sample after a seek, a local or sample-zero MIDI Stop suppressing entry across that seek, and a stale stopped playhead preserving the previous saved start.

## Pattern capture with pre-roll

The owner reproduced the End on repeat cancellation in FL Studio PAT mode at 82 BPM and 48 kHz. The traced build of `7776f4c` recorded zero dropped rows. The first playing callback started at sample 0 with the declared downbeat at sample 96. At the wrap, the previous callback ended at sample 140,488 against a declared end of 140,584. The next callback began at sample −96 against a declared start of 0. Both callbacks were 96 samples ahead of the same musical boundary after accounting for the host's changed sample origin.

The two in-block downbeats were 140,488 frames apart. The processor regression uses the measured timing and asserts a four-beat take of that exact length. Existing processor fixtures still cover a changed repeat range and an early seek.

On `7776f4c` with the pre-roll fix applied, the normal Windows MSVC Release build passed the standard PR regression set (15/15) and the Loop playback and Rack replay checks (3/3).

The owner confirmed on installed Release `3f1533b` that PAT mode accepts the one-bar take after the first wrap in the same 82 BPM project. With the saved start cleared and a matching one-bar Playlist selection in Song mode, the swells started without Cue and stayed aligned across the wrap. These desktop checks and the automated seek assertions complete task 4.2.

## Merge verification

After integrating `main` at `46068f8`, Windows MSVC Release checks passed on `923e393` with the OpenSpec archive edits in progress. The standard PR regression set passed 15/15, the Loop playback and Rack replay checks passed 3/3, and the Perform UI checks passed 3/3. OpenSpec validated all 24 canonical specs after the sync.
