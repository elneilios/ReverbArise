# Spec Delta

## ADDED Requirements

### Requirement: Loop cues use the verified phrase reference

With a verified Loop at its native rate, Cue SHALL schedule the earliest future verified phrase boundary that allows the full negative lead-in. The fixed-period Rack route SHALL require two consecutive marker intervals equal to the captured sample count. Stop SHALL cancel a queued cue or fade active wet within 5 ms. Dry passthrough SHALL continue in Standby.

#### Scenario: Cue while the patch is running

- **WHEN** the user cues a ready Loop
- **THEN** the complete lead-in SHALL run into the agreed phrase start and wet playback SHALL continue in phase
- **AND** a Cue arriving after the lead-in deadline SHALL wait for the next eligible phrase

#### Scenario: Stop or lose the phrase reference

- **WHEN** Stop is received or the phrase reference is lost
- **THEN** queued wet SHALL be cancelled and sounding wet SHALL fade
- **AND** reacquisition SHALL require a fresh Cue before playback
- **AND** simultaneous Stop and Cue SHALL resolve to Stop

### Requirement: Verified host routes can cue and stop Loop Perform

Loop Perform SHALL accept Cue and Stop from supported Rack Host-FX routes and DAW MIDI routes. External triggers SHALL use the same scheduler and Stop precedence as the shared editor controls. DAW Loop cueing SHALL use phrase timing while existing Song playback retains host-playhead timing.

#### Scenario: Schedule a Loop from the FL Studio Piano Roll

- **WHEN** a supported Piano Roll route sends the configured Cue or Stop note to an accepted Loop during playback
- **THEN** Perform SHALL schedule the lead-in and repeat or stop wet according to the same rules as its Cue and Stop controls

### Requirement: Loop playback uses the shared prepared swell set

Loop playback SHALL combine the admitted swell set using the same activity, solo, shaping, IR and gain semantics as Perform. Selection SHALL govern editing and manual audition independently of live membership. The first launch SHALL include the full lead-in. Repeats SHALL sum each overlapping contribution once at its phrase-relative position.

#### Scenario: Cue several swells with overlapping lead-ins and tails

- **WHEN** several eligible prepared swells overlap across a phrase boundary
- **THEN** the wet output SHALL include their combined contributions at the expected samples
- **AND** the first launch SHALL contain only contributions from scheduled repeats

### Requirement: Loop playback publishes bounded complete revisions

Loop playback SHALL mix complete prepared revisions within the existing working budget. A revision SHALL carry its source and swell identities. Edits during playback SHALL publish at a safe phrase boundary once ready and SHALL retain the full lead-in needed by the new revision. Pending, incompatible or over-budget work SHALL be visible.

#### Scenario: Change a swell while the Loop is repeating

- **WHEN** an edit or activity change supersedes the prepared set during playback
- **THEN** the accepted revision SHALL remain coherent until the replacement can be admitted with a bounded transition
- **AND** a stale revision SHALL never replace the current source or edit

#### Scenario: Prepared coverage exceeds the working budget

- **WHEN** the requested complete Loop plan cannot fit within the budget
- **THEN** Perform SHALL explain why the revision cannot be cued
- **AND** the real-time callback SHALL remain free of allocation, file reads and blocking
