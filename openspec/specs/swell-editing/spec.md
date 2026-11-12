# swell-editing Specification

## Purpose
Let users identify musical arrivals in sparse captured audio and visually edit multiple reverse-reverb swells without destructively changing their source or full convolution results.

## Requirements

### Requirement: Onset analysis proposes candidates without changing confirmed edits
The plugin SHALL analyze committed covered source intervals offline and expose sensitivity and minimum-separation controls for onset suggestions. Diagnostic unconfirmed candidates SHALL remain distinct from active swells; automatic target acceptance and generation SHALL follow automatic-swell-processing. Reanalysis SHALL preserve confirmed targets and their edits, and SHALL not bridge uncaptured or unavailable gaps.

#### Scenario: Analyze sparse capture
- **GIVEN** two captured sections are separated by an uncaptured gap
- **WHEN** the user requests onset suggestions
- **THEN** suggestions refer to absolute covered positions, the gap is not analyzed as silence, and the interface remains responsive

#### Scenario: Reanalyze after manual editing
- **WHEN** sensitivity changes and the user reruns analysis after confirming and moving targets
- **THEN** unconfirmed candidates are refreshed without moving, duplicating, deleting, or resetting the confirmed targets and their swell edits

### Requirement: Confirmed targets own independent swell definitions
The plugin SHALL allow confirming a candidate, adding a manual target, moving or deleting a confirmed target, and selecting multiple swells. Each confirmed target SHALL own a stable swell identity, arrival, editable source interval, IR assignment, name, enabled/mute/solo state, and shape settings. Unconfirmed suggestions SHALL not own active swells.

#### Scenario: Create and edit two swells
- **WHEN** the user confirms two arrivals and changes one swell's name, source interval, or IR
- **THEN** the other swell keeps its identity and settings, and either swell can be selected or deleted independently

#### Scenario: Move an arrival
- **WHEN** a confirmed arrival is moved by a sample offset
- **THEN** its source interval and audible bounds move by the same offset, its fades retain their lengths, and only that swell's source-dependent render becomes stale

#### Scenario: Invalid source interval
- **WHEN** a target edit would create an empty, overflowing, uncaptured, or unavailable source interval
- **THEN** the invalid interval is rejected or explicitly marked non-renderable without altering another swell or presenting missing audio as silence

### Requirement: Timeline navigation preserves absolute edit positions
The editor SHALL provide a global capture overview and shared detail lanes for source, arrivals, and wet previews, with 1/2/4/8/16-bar pages, continuous zoom/scroll, previous/next page, and overview navigation. It SHALL show fixed-tempo Playlist references and available host playhead position. Navigation SHALL never move, truncate, or invalidate audio or edits.

#### Scenario: Swell spans pages and song start
- **WHEN** the user navigates across a swell that starts before bar 1 or crosses a page boundary
- **THEN** its full absolute bounds remain intact, negative time is viewable, and no preview audio is trimmed at a page edge

#### Scenario: Snap and precise adjustment
- **WHEN** a marker or handle is dragged with snap enabled or disabled
- **THEN** grid snapping applies only when enabled, precise numeric adjustment remains available, and sample positions do not depend on the current page or playhead

### Requirement: Swell shape is nondestructive and independent of convolution
Each swell SHALL expose audible start/exclusive end, independent fades, linear/smooth/exponential curves and finite gain. Shape edits within verified support SHALL update preview/audition without reconvolution or raw-sample changes. Placement and support extension SHALL follow reverse-convolution; unsupported requests SHALL remain explicitly pending rather than fabricate audio. Invalid/non-finite bounds SHALL be rejected; full raw bounds SHALL remain inspectable.

#### Scenario: Crop and fade a ready swell
- **WHEN** the user adjusts audible bounds, fades, curve, and gain
- **THEN** only the shaped preview changes, its raw render identity remains unchanged, and controls can restore the full unedited response

#### Scenario: Fade overlap and zero length
- **WHEN** fade lengths overlap or either fade length is zero
- **THEN** the documented envelope remains finite and deterministic, overlap combines both fades, and a zero-length musical fade adds no musical ramp; disclosed monitoring safety ramps follow audition-monitoring

### Requirement: Selected-swell audition respects edits and activity controls

Manual audition SHALL anchor to the selected swell and apply edited shape/gain and protected levels. Membership, isolation, contextual dry and listening tails SHALL follow audition-monitoring. Disabled, muted or solo-excluded wet SHALL not audition. Selection or explicit audible-shape changes SHALL stop audition and require explicit restart. An accepted IR replacement and its fitted shape SHALL hand over active audition. Live input SHALL remain unchanged and preview SHALL never enter capture.

#### Scenario: Audition shaped selection
- **WHEN** a current enabled swell with cropped bounds and fades is auditioned
- **THEN** the prepared listening interval loops with those edits and the selected monitoring mode, without requesting host transport changes, and Stop ends only preview output

#### Scenario: Change selection or mute while playing
- **WHEN** the user selects another swell, explicitly changes shape, disables/mutes the active swell, or excludes it through solo
- **THEN** active audition stops without starting another swell automatically or freeing its audio on the callback

#### Scenario: Replace an IR while Audition is active
- **WHEN** an active audition contains the swell being updated and the latest compatible IR replacement completes
- **THEN** audition crossfades to the prepared replacement at the current listening position, wrapping into the new listening window if necessary
- **AND** neither a pending nor failed replacement interrupts the accepted mix
- **AND** a Stop or change of selection takes precedence over the handover

### Requirement: Wet reset restores creation transition defaults
A visible one-click Reset wet shape action SHALL restore automatic wet start, zero placement, zero fade-in, neutral tensions, 0 dB gain and the current 20 ms positive overlap/20 ms Smooth fade-out defaults using valid sample rounding. It SHALL preserve source, arrival, IR, name, identity, activity/freeze and unrelated settings. Required support SHALL remain genuine and explicitly pending when unavailable.

#### Scenario: Reset an edited wet shape
- **GIVEN** compatible 48000 Hz timing and sufficient verified support
- **WHEN** the selected swell's wet reset is invoked after moving and shaping it
- **THEN** placement and gain are zero, start is automatic, fade-in is zero with neutral Linear curve, tensions are zero, end offset and Smooth fade-out are each 960 samples
- **AND** source, arrival, IR, name, UUID, enabled/mute/solo/freeze and other swells are unchanged; raw assets are reused when dependencies/support already match

#### Scenario: Reset lacks support
- **WHEN** restored wet defaults require unsupported input or compatible rate is unavailable
- **THEN** the UI explains pending support or unavailable reset without padding audio, changing dry or silently unfreezing the swell

### Requirement: Full reset is explicitly scoped to one swell
An adjacent reset dropdown SHALL offer Reset all swell settings. It SHALL transactionally restore creation-policy source, current valid future-default IR, wet defaults, enabled/unmuted/un-soloed/unfrozen activity at the selected swell's current arrival. Identity, name, target, capture and all other swells/global preferences SHALL remain unchanged. Invalid prerequisites SHALL prevent partial reset.

#### Scenario: Reset the complete selected definition
- **WHEN** a user explicitly chooses Reset all swell settings with compatible capture and a verified default IR
- **THEN** the selected source starts at its current arrival and ends under the existing creation/next-arrival/coverage policy, its IR becomes the current future default and its wet/activity settings return to defaults
- **AND** its ID/name/arrival remain intact, affected rendering refreshes under existing rules and another swell's source/IR/shape, global settings and captured assets remain unchanged

#### Scenario: Default IR or capture is unavailable
- **WHEN** a full reset is requested without the required compatible source or valid default IR
- **THEN** no part of the definition changes, the unavailable action explains why, and wet-only reset remains independently available when its own prerequisites permit

#### Scenario: Reset does not mean full response
- **WHEN** Reset wet shape or Reset all swell settings is selected
- **THEN** the gentle creation release is applied rather than exposing the full untrimmed response, and Full response remains a separately labelled operation

### Requirement: New swell names identify their target bar
New swells SHALL use unique `Swell Bar <bar>.<beat>[.<fraction>]` labels from one-based dry-target Playlist timing. Use the shared musical-position formatter and a numbered suffix for collisions. Naming SHALL NOT use creation count or wet placement, nor invent unavailable timing. Existing, restored and user-edited names SHALL remain unchanged.

#### Scenario: Create in a later section
- **GIVEN** valid fixed 120 BPM, 4/4 timing with Song origin zero
- **WHEN** a new target is created at 34 seconds, inside Playlist bar 18
- **THEN** its default name is `Swell Bar 18.1` regardless of how many swells already exist or the wet placement offset

#### Scenario: Preserve names and distinguish shared bars
- **WHEN** two new targets occur in bar 18 and an existing target was renamed Chorus lift
- **THEN** both new names identify bar 18 and their own beat/fractional-beat positions with separate IDs and unique labels, Chorus lift stays unchanged, and reanalysis, movement, reset or reopening does not rewrite saved names

#### Scenario: Different meter or musical origin
- **WHEN** valid fixed meter or the sample/PPQ origin differs from 4/4 at sample zero
- **THEN** the creation name uses that actual one-based Playlist bar mapping, agrees with the ruler, and unknown timing is unavailable rather than fabricated as bar 1

#### Scenario: Distinguish targets within one beat
- **GIVEN** valid fixed 120 BPM, 4/4 timing with Song origin zero
- **WHEN** new targets are created at 34 seconds and 34.125 seconds
- **THEN** their names are `Swell Bar 18.1` and `Swell Bar 18.1.25`; a repeated target at 34 seconds is named `Swell Bar 18.1 (2)` without renaming existing swells

### Requirement: IR replacement fits wet shape to the available response

Replacing the IR of an editable swell SHALL derive a valid effective shape from the user's shape intent and the verified response bounds. The swap SHALL preserve the swell identity, source interval, arrival, placement, activity, gain, curves and compatible timing edits. It SHALL recover the intended timing wherever supported by a subsequent response.

#### Scenario: Explicit pre-roll exceeds a shorter response
- **WHEN** a supported replacement produces wet audio with a later start than the requested explicit wet start
- **THEN** the swell accepts the IR and uses the earliest supported start while keeping any compatible end and fitting fade lengths to the usable interval
- **AND** a wet region with no overlap uses the available full response and a coherent fitted envelope, with the arrival marker fixed

#### Scenario: Return to a longer response
- **WHEN** a later replacement supports the original intended bounds and fades
- **THEN** those bounds and fades are restored, independent of intervening IR selections

#### Scenario: Edit after a fit
- **WHEN** the user changes a fitted timing control
- **THEN** the explicit edit becomes the new intent for that control, and unrelated timing intent remains available for future swaps

#### Scenario: Freeze while browsing
- **WHEN** the user tries to replace the IR on a frozen swell
- **THEN** the frozen render is preserved and the user is told to unfreeze before changing the response
- **AND** after unfreezing the edit is eligible under current source and timing checks
