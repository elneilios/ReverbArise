# Spec Delta

## ADDED Requirements

### Requirement: FL Studio repeat and placement use independent host evidence

FL Studio automatic repeat SHALL use compatible running transport with a host-declared repeat matching the accepted Loop's musical length and sample extent within the supported rounding tolerance. Playlist placement SHALL use a valid absolute song position for the current instance.

#### Scenario: Pattern and Playlist expose the same repeat bounds

- **WHEN** either FL Studio playback context reports an eligible repeat range
- **THEN** the range is eligible for automatic repeat without requiring a Song or Pattern mode flag

#### Scenario: Repeat information is unavailable or incompatible

- **WHEN** FL Studio omits repeat bounds, the range length differs from the Loop, or the running timing is incompatible
- **THEN** automatic repeat is unavailable and Perform explains the reason
- **AND** manual Cue remains available where its own timing requirements are met

#### Scenario: Playhead position is unavailable or stale

- **WHEN** a current absolute position cannot be established for the Set Loop Start action
- **THEN** Perform does not save a stale position and explains the limitation
- **AND** the user can enter an explicit bar and beat through the Set Loop Start context menu

#### Scenario: The host cannot supply an absolute song position

- **WHEN** the host does not support Playlist placement, including a Rack setup supplying only Clock or Phrase timing
- **THEN** Perform keeps Set Loop Start visible and disabled, with “Requires a host song position” in the fixed footer on hover and in the accessible description
- **AND** verified Clock and Phrase remain available for the Rack Cue workflow

## MODIFIED Requirements

### Requirement: FL Studio capture timing and live playback are independent

The system SHALL use a verified capture grid for FL Studio Learn. When manual Cue is the active Perform route, an accepted Loop with prepared swells SHALL be cueable while Pattern or Playlist playback supplies valid bar timing and live output is enabled. The first eligible downbeat on the default or Phrase-aligned playback grid with room for the full wet lead-in SHALL establish the origin for repeats at the accepted Loop length. The FL Studio Cue route SHALL require no Phrase notes.

#### Scenario: Cue during running transport

- **WHEN** the musician clicks Cue Loop or a supported MIDI Cue note arrives at any point during running transport in Perform with manual Cue active
- **THEN** the command queues the first downbeat on the active playback grid with enough time for the full wet lead-in, measured from the next callback for the button or the note's sample offset for MIDI
- **AND** the lead-in begins early enough for beat one of the Loop to land on that downbeat, and subsequent cycles use the accepted Loop length

#### Scenario: A Phrase note shifts the FL Studio playback origin

- **WHEN** a Phrase note arrives while the Playlist runs and manual Loop Cue is in standby
- **THEN** future cues count playback bars from the note's sample using the verified Playlist tempo
- **AND** the Learn grid, queued launches, and active repeats are unchanged
- **AND** stopping, seeking, resetting, or losing valid timing restores the default host origin until a fresh Phrase note arrives
- **AND** a verified repeat preserves the current origin

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

#### Scenario: FL Studio reports the repeat downbeat inside a pre-roll callback

- **WHEN** the declared repeat downbeat is inside the callback and consecutive passes report the same musical range, phase and length after accounting for a shifted sample origin
- **THEN** Learn uses that downbeat to start or close the take and captures exactly the audio between the two downbeats
- **AND** a seek with a mismatched endpoint phase still interrupts the take

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

- **WHEN** verified Cue and Stop events arrive at distinct positions during running transport with manual Cue active
- **THEN** prepared wet follows the existing Cue and Stop behavior without interrupting dry audio
- **AND** duplicate or released inputs do not generate extra commands

#### Scenario: Transport interrupts a live take

- **WHEN** transport stops, seeks, or resets during manually cued Loop playback
- **THEN** wet playback stops and requires a fresh Cue while transport is running
- **AND** missed note-on events are not reconstructed from Playlist position

#### Scenario: Transport crosses a verified repeat

- **WHEN** consecutive playing callbacks keep the declared repeat range and musical mapping and meet its exact end and start samples
- **THEN** queued and active Loop playback keep their scheduled phase

#### Scenario: External delivery is unavailable

- **WHEN** the tested FL Studio route fails to deliver usable external commands
- **THEN** the documentation identifies the unavailable route and gives the verified UI or automation workflow
