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
Manual audition SHALL anchor to the selected swell and apply edited shape/gain and protected levels. Membership, isolation, contextual dry and listening tails SHALL follow audition-monitoring. Disabled, muted or solo-excluded wet SHALL not audition. Selection or audible-shape changes SHALL stop audition and require explicit restart. Live input SHALL remain unchanged; preview SHALL never enter capture.

#### Scenario: Audition shaped selection
- **WHEN** a current enabled swell with cropped bounds and fades is auditioned
- **THEN** the prepared listening interval plays once with those edits and the selected monitoring mode, without requesting host transport changes, and Stop ends only preview output

#### Scenario: Change selection or mute while playing
- **WHEN** the user selects another swell, changes shape, disables/mutes the active swell, or excludes it through solo
- **THEN** active audition stops without starting another swell automatically or freeing its audio on the callback
