# Tasks

Prepare focused automated cases with each task. Follow the contribution workflow for execution after the owner approves marking the draft ready. Record deferred checks explicitly.

## 1. Fit and persist shape intent

- [x] 1.1 Implement checked wet-shape fitting in the shared swell model using the prepared response bounds. Add focused cases for explicit pre-roll and crops, automatic start, a fully disjoint interval, one-sample support, fades and curves, nonzero placement, and long-to-short-to-long recovery without cumulative loss.
- [x] 1.2 Connect explicit shape edits and resets to remembered intent and extend compatible state encoding. Add session cases for a legacy shape, a fitted round trip, invalid optional intent, and long-IR recovery after reopening. Verify established asset identities and effective saved audio survive.

## 2. Stage and publish IR replacements

- [x] 2.1 Route bundled, new-file and existing managed IR selections through worker-owned candidates. Check source/support, resource limits and generation ownership before publishing. Replace the Monumental-to-Whisper rejection expectation in `ir_handover_test` with acceptance and add a custom long/short pair across two swells, a swap without an eligible retained voice, rapid supersession, explicit edit invalidation, and real decode/source/budget failures.
- [x] 2.2 Integrate acceptance with calibration, headroom, audition and the existing live handover. Verify accepted audio before publication, the bounded transition at the same position, callback allocation safety, and independent neighbouring audio. Cover pending/failed and accepted save/reopen, internal Export and host bounce with matching audio and shape. Verify freeze/unfreeze and any shared Loop path affected by the worker changes.
- [x] 2.3 Prepare active Audition replacements off-thread and publish at the current absolute cursor with a bounded crossfade. Add audio cases for a shorter window, group Dry + Wet and Wet Only, rapid supersession, failed preparation, Stop/selection races and no callback allocations or deallocations.
- [x] 2.4 Update the IR library and affected editing/playback guidance to describe fitting, intent and acceptance. Verify the old Full response or Reset instruction is removed from current swapping guidance.

## 3. Show the result and make failures actionable

- [x] 3.1 Bind Perform selector, list status, waveform, handles and inspector to the accepted revision and pending request. Reuse shared controls and feedback. Add component/binding cases for pending, fitted and superseded selections, specific recovery copy, retained/unavailable audio, and freeze. Prepare and launch a native preview for owner review of fit and error feedback before proceeding to draft review.
- [x] 3.2 Update the offline guide through the maintain-user-guide workflow and record currency for the delivered behavior. Verify the user-facing instructions match the final controls and explain fit status and actionable errors.

## 4. Validate the full workflow

- [ ] 4.1 Use the saved `PreverbTesting_3.flp` session and the [fixture record](https://github.com/elneilios/ReverbArise/issues/183#issuecomment-6810691860) as the primary FL Studio reference. Preserve the original and exercise a working copy with verified access to its source and managed IR assets. At bar 3.1, test `02_Bell_Ghost_Crescendo.wav` to `13_Pitched_Comb_E4_Crescendo.wav`, then `12_Stereo_Ping_Pong.wav` and back. Compare on a second swell and include bundled IRs of different lengths. Check arrival, fitted handles and status, audible handover during manual Audition and Song playback, save/reopen and export in one focused host pass. Record the build, host, inputs, results and any missing assets or unchecked coverage in dated validation evidence.
- [x] 4.2 At the contribution workflow's ready gate, run the focused model, handover, persistence, export and UI checks plus the standard PR regression set. Record exact revision, commands, outcomes and any pending acceptance in the PR.
