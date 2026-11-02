# Spec Delta

## ADDED Requirements

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
New swells SHALL receive the default name Swell followed by their one-based Playlist arrival bar from valid fixed musical timing, rather than creation count. Naming SHALL use the dry target, not wet placement, and SHALL NOT invent unavailable timing. Existing, restored and user-edited names SHALL be preserved; repeated arrivals within one bar MAY share the default name while retaining independent identities.

#### Scenario: Create in a later section
- **GIVEN** valid fixed 120 BPM, 4/4 timing with Song origin zero
- **WHEN** a new target is created at 34 seconds, inside Playlist bar 18
- **THEN** its default name is Swell 18 regardless of how many swells already exist or the wet placement offset

#### Scenario: Preserve names and distinguish shared bars
- **WHEN** two new targets occur in bar 18 and an existing target was renamed Chorus lift
- **THEN** both new names identify bar 18 with separate IDs and precise arrival information, Chorus lift stays unchanged, and reanalysis, movement, reset or reopening does not rewrite saved names

#### Scenario: Different meter or musical origin
- **WHEN** valid fixed meter or the sample/PPQ origin differs from 4/4 at sample zero
- **THEN** the creation name uses that actual one-based Playlist bar mapping, agrees with the ruler, and unknown timing is unavailable rather than fabricated as bar 1
