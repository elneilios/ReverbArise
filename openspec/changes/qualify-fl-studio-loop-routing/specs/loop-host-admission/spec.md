# FL Studio Loop host admission

## MODIFIED Requirements

### Requirement: Musical admission requires explicit meter and origin

Bars, beats and musical grids SHALL require verified beat timing and explicit meter. Capture SHALL establish an origin shared by the audio and clock. FL Studio Cue SHALL use the Playlist grid by default. The supported Rack Clock route SHALL use 4/4 and count the first pulse of the current clock run as beat one. A Phrase note MAY set a different playback downbeat while in standby; Rack requires it to be aligned with a Clock beat and also accepts an aligned Bar pulse. Legacy Phrase-only routes SHALL still require a compatible recurring marker.

#### Scenario: Beat and bar observations without a declared meter

- **WHEN** periodic inputs arrive on a route with no declared meter
- **THEN** musical duration and grid admission remain unavailable with a meter explanation

#### Scenario: A Rack patch supplies Clock only

- **WHEN** the Clock starts or restarts after timing loss
- **THEN** its first pulse sets beat one and compatible beat intervals make Cue available once wet is prepared
- **AND** no recurring Phrase pulse is required

#### Scenario: Clock and Phrase remain connected

- **WHEN** an aligned Phrase or Bar pulse arrives with a Clock beat while playback is in standby
- **THEN** the marker defines the downbeat for future Cues without waiting for recurring phrase intervals
- **AND** it does not move the origin of a recording already in progress or an already scheduled launch

#### Scenario: Short sequencer phrase matches the accepted take

- **WHEN** the supported recurring Phrase route supplies the only live reference
- **THEN** two matching intervals are required before Cue, including valid observations in Learn for that accepted take

#### Scenario: Capture length differs from the recurring reference

- **WHEN** a legacy Phrase-only route supplies recurrence that differs from the accepted take
- **THEN** the take remains valid and that route cannot enable Cue

#### Scenario: Verified take length rounds to an adjacent frame count

- **WHEN** a legacy Phrase-only route matches the take's sample rate, stable tempo, meter, and duration in beats
- **THEN** either adjacent integer frame count matches recurrence without changing the accepted extent
- **AND** an out-of-range frame count or mismatched timing leaves Cue unavailable on that route

#### Scenario: Capture uses one clock pulse per beat

- **WHEN** a compatible capture route provides stable sample-timed beat pulses and declares 4/4
- **THEN** Duration starts at the first Clock pulse of a new run or the next observed downbeat of a running Clock
- **AND** Duration and Cue share the same four-beat count, including after switching between Learn and Perform
- **AND** End Signal keeps its trigger-driven capture origin
- **AND** competing legacy beat/bar inputs still invalidate a simplified take

#### Scenario: Producer routes the wet origin separately

- **WHEN** a supported live route defines a downbeat different from the dry signal's origin
- **THEN** Cue uses the live grid without an audio alignment check or confirmation step
- **AND** capture retains its independent timing and input requirements

## ADDED Requirements

### Requirement: End on repeat captures one complete pass

Learn SHALL offer an opt-in End on repeat method. FL Studio SHALL use the verified host-declared repeat range. Rack SHALL use Clock for the beat and Phrase aligned with a Clock beat to mark the repeat. A take SHALL start at the first admitted repeat start after arming and close at the next admitted repeat boundary after whole bars. Existing Duration and End Signal semantics SHALL remain independent.

#### Scenario: Arm partway through a repeating pattern

- **WHEN** the musician arms End on repeat partway through a pass
- **THEN** Learn waits for the next admitted repeat start, then records one complete pass

#### Scenario: The repeat or capture is interrupted

- **WHEN** timing is lost, the declared range changes, the host seeks early, or Reset or Disarm arrives during the take
- **THEN** the candidate is discarded and the accepted Loop is preserved

### Requirement: FL Studio capture timing and live playback are independent

The system SHALL use a verified capture grid for FL Studio Learn. In Perform, an accepted Loop with prepared swells SHALL be cueable while the host transport runs with valid Playlist timing and live output enabled. The first eligible downbeat on the default or Phrase-aligned playback grid with room for the full wet lead-in SHALL establish the origin for repeats at the accepted Loop length. The FL Studio Cue route SHALL require no Phrase notes.

#### Scenario: Cue during running transport

- **WHEN** the musician clicks Cue Loop or a supported MIDI Cue note arrives at any point during running transport in Perform
- **THEN** the command queues the first downbeat on the active playback grid with enough time for the full wet lead-in, measured from the next callback for the button or the note's sample offset for MIDI
- **AND** the lead-in begins early enough for beat one of the Loop to land on that downbeat, and subsequent cycles use the accepted Loop length

#### Scenario: A Phrase note shifts the FL Studio playback origin

- **WHEN** a Phrase note arrives while the Playlist runs and Loop playback is in standby
- **THEN** future cues count playback bars from the note's sample using the verified Playlist tempo
- **AND** the Learn grid, queued launches, and active repeats are unchanged
- **AND** stopping, seeking, wrapping, or losing valid timing restores the default Playlist origin until a fresh Phrase note arrives

#### Scenario: A deliberate End Signal reaches a verified downbeat

- **WHEN** the musician requests End Signal and the closing downbeat is observed while the capture timing remains valid
- **THEN** the candidate settles at that boundary

#### Scenario: Duration is armed on the Playlist grid

- **WHEN** the musician arms Duration before starting the transport at a bar line, including bar 1
- **THEN** capture starts at that bar line, subject to continuity verification before the take can be accepted
- **AND** if playback starts or Arm is pressed partway through a bar, capture waits for the next bar line

#### Scenario: The host repeats its declared capture range

- **WHEN** FL Studio remains playing with a stable timing map, one callback ends exactly at the declared repeat end, and the next starts at the same range's beginning
- **THEN** Learn counts the wrapped beat on its continuous capture timeline and settles Duration or End Signal when that beat completes the take
- **AND** an early jump or a change to the declared repeat bounds cannot qualify as a seamless repeat

#### Scenario: MIDI drives an End Signal take

- **WHEN** Learn receives the configured Start/Arm note in End Signal mode
- **THEN** it starts on that bar line or waits for the next bar line if the note arrived mid-bar
- **AND** the configured Learn Stop note requests the following bar line without requiring a UI click

#### Scenario: Timing is lost before the closing downbeat

- **WHEN** the capture route loses timing or callbacks before the requested closing downbeat
- **THEN** the incomplete candidate is discarded and the previous accepted Loop is preserved

#### Scenario: FL Studio cannot provide a safe End Signal boundary

- **WHEN** the setup cannot provide a verified end request and closing downbeat
- **THEN** End Signal is reported unavailable and Duration is offered where verified

### Requirement: FL Studio event-driven Loop commands use verified delivery

The system SHALL admit external commands only for a measured FL Studio configuration. FL Studio and Rack SHALL use the same channel 1 note numbers for Phrase (12), Perform Cue (16), Perform Stop (17), Perform Stop Now (18), Learn Start/Arm (24) and Learn Stop (25). The documented configuration SHALL identify the tested routing, command edges, and reset and offline behavior. An unverified external route SHALL leave the corresponding UI workflow available where timing permits.

#### Scenario: A verified command route is used at multiple Playlist positions

- **WHEN** verified Cue and Stop events arrive at distinct positions during running transport
- **THEN** prepared wet follows the existing Cue and Stop behavior without interrupting dry audio
- **AND** duplicate or released inputs do not generate extra commands

#### Scenario: Transport interrupts a live take

- **WHEN** transport stops, seeks, wraps, or resets
- **THEN** wet playback stops and requires a fresh Cue while transport is running
- **AND** missed note-on events are not reconstructed from Playlist position

#### Scenario: External delivery is unavailable

- **WHEN** the tested FL Studio route fails to deliver usable external commands
- **THEN** the documentation identifies the unavailable route and gives the verified UI or automation workflow
