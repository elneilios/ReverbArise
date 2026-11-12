# Tasks

## 1. Host timing and route selection

- [x] 1.1 Check FL Studio parked position updates while stopped and record whether the effect receives different positions. The owner confirmed that moving the parked playhead changes the reported values.
- [ ] 1.2 Resolve Loop playback eligibility from declared repeat bounds, compatible song timing and the selected placement. Extend the processor fixtures to cover matching and mismatched lengths, unavailable bounds, exact wraps, early seeks and distinct instances. Document the route in the host admission guide.

## 2. Loop playback and session state

- [ ] 2.1 Implement automatic repeat playback and keep overlap mixing bounded. Add sample assertions for startup at the boundary, mid-repeat entry, negative lead and tail at a wrap, timing loss and restart after Stop.
- [ ] 2.2 Persist one editable placement against the accepted Loop identity. Verify old sessions, save/reopen, source replacement, edit and clear, and isolation between instances.
- [ ] 2.3 Schedule placement by absolute host position with the current prepared revision. Verify full first lead-in, mid-swell seek, repeat overlap, final tails, stop suppression and precedence over a matching host repeat with processor or mixer fixtures. Document the supported routing and placement in the Loop playback guidance.
- [ ] 2.4 Keep Rack cueing manual and use the accepted phrase length from a valid Phrase origin with Clock. Verify that a four-bar Loop skips intervening bar lines, that Cue allows the full lead-in and that Clock-only, legacy Phrase-only and reset behavior still pass.

## 3. Perform controls

The owner approved the shared control and its three-pair layout on 12 November 2026. Native scaling and integrated checks are tracked below.

- [ ] 3.1 Build the reusable Action Button + State Field in the control library and launch a native preview for owner review. Cover primary, neutral and destructive commands, focus overlay, disabled contrast, the single seam at 100%, 125% and 150%, and the three pairs at 900 × 600 and 1100 × 850. Record approval of the family, layout and any new metric, variant or exception before integration.
- [x] 3.2 Add component checks for the seam hit boundary, Tab and Shift+Tab, Return, Space, accessible focus and press, pointer focus visibility, one-shot commands, and steady field state on pending, rejected, disabled and stale invocation. Record the approved family and metrics in the UI standards.
- [ ] 3.3 Integrate the three pairs and bind playhead placement, menu entry and clearing. Verify accepted position and playback status remain independent, command and field geometry stays stable across state changes, overflow fades and scrolls on hover, and unsupported hosts show disabled Set Loop Start with fixed-footer hover help and an accessible explanation. Cover restoration, stale host timing, source change and shared Song and Loop Audition behavior in binding checks. Update the offline user guide for FL Studio and Rack.

## 4. Integrated verification

- [x] 4.1 Follow the contribution test cadence. After approval to mark the draft ready, run the targeted processor and component checks and the standard PR regression set with the supported build wrapper. Record results and uncovered configurations.
- [ ] 4.2 Run a consolidated FL Studio desktop smoke test for a matching Pattern, a declared Playlist repeat, a placed Loop with lead-in and a seek. Record the transport and audible results and the scope of Rack verification.
