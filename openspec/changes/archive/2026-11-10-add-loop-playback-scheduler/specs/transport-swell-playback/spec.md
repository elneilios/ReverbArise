# Spec Delta

## MODIFIED Requirements

### Requirement: Transport discontinuities do not replay old positions
Song seeking and looping SHALL abandon the old wet position and join prepared audio at the new absolute offset. Stopping, disabling or losing valid timing SHALL stop wet with a bounded click-prevention transition. Old-position audio SHALL not continue beyond that transition. Host transport SHALL never be changed by plugin playback controls.

#### Scenario: Loop through a swell
- **WHEN** a warm loop wraps into the middle of a swell
- **THEN** playback joins its loop destination offset and neither restarts the render nor continues the previous cursor

#### Scenario: Stop on nonzero audio
- **WHEN** Song transport stops on a nonzero wet sample while processing continues
- **THEN** wet reaches zero through a disclosed ramp no longer than 5 ms and does not restart until a subsequent valid Play

### Requirement: Timing incompatibility is explicit
Song live wet SHALL require valid absolute host samples and fixed rate/tempo/meter compatible with accepted assets. A fresh valid playing run SHALL establish an ephemeral sample-to-PPQ mapping independent of parked/source origin. Each run SHALL retain its mapping within the existing one-sample origin tolerance; unsupported changes SHALL suppress wet with a reason. Fresh validation SHALL precede resumption. Stored sample positions, edits and assets SHALL remain unchanged.

#### Scenario: Change tempo or sample rate
- **WHEN** the host mapping becomes incompatible with prepared assets
- **THEN** those swells are unavailable for live playback and the workspace explains the incompatibility; default Dry + Wet continues passing dry

#### Scenario: Playing clock differs from parked source mapping
- **GIVEN** verified ready wet at fixed absolute sample positions and a compatible fixed rate, tempo and meter
- **WHEN** a fresh playing run supplies a valid mapping whose origin differs from the parked/source mapping, including the recorded approximately 16-sample difference
- **THEN** the playing mapping is validated independently and eligible wet plays at its stored absolute sample positions
- **AND** no saved mapping, source, render, placement or persistent identity is rewritten or offset, and a resident valid window remains eligible for a warm join

#### Scenario: Zero-offset transport remains compatible
- **WHEN** a fresh playing run has the same origin as parked/source timing
- **THEN** existing absolute-position output, routing and gain behavior remain unchanged

#### Scenario: Mapping changes inside a playing run
- **WHEN** origin changes by more than the existing one-sample allowance during an established playing run
- **THEN** affected live wet is suppressed with the actual timing reason and the established run mapping is retained
- **AND** seeking, looping, preparation misses and revision updates do not silently establish a replacement baseline

#### Scenario: Invalid timing does not create a new run
- **WHEN** host timing is missing, nonfinite or out of range, or a callback range is invalid
- **THEN** affected wet is unavailable and that observation cannot establish a valid playing mapping or masquerade as a confirmed Stop
- **AND** returning timing is freshly validated against the retained run mapping before wet resumes

#### Scenario: Confirmed Stop followed by a new Play
- **WHEN** valid stopped transport is followed by a fresh valid playing run
- **THEN** a new runtime mapping is validated independently, with no automatic audition and no persistent edit
- **AND** only complete current assets can join at the new absolute Song position without replaying elapsed audio

#### Scenario: Session and workflow admission are independent
- **WHEN** state is restored, the audio session is replaced, or Perform is re-entered while the host is already playing
- **THEN** live admission requires fresh runtime validation and current asset dependencies, and no stale run identity can admit old work
- **AND** disabled live output remains disabled and Capture, Audition, internal Export and host offline timing behavior retain their existing contracts

## ADDED Requirements

### Requirement: Loop Cue requires independent live phase
Loop live playback SHALL require a verified recurring reference with shared phrase origin and matching accepted rate, tempo, meter and beat length. Capture timing SHALL supply only the source grid and exact recorded extent. Standby SHALL keep checking the reference, pass dry and report the unmet prerequisite for Cue.

#### Scenario: Accepted capture lacks matching recurrence
- **WHEN** a Clock-only take has a verified local grid and the live phrase reference is absent or repeats every four bars for a three-bar take
- **THEN** the accepted loop and its grid remain valid and Cue stays unavailable with the corresponding reason
- **AND** an aligned matching reference can admit Cue after two complete matching intervals and coherent prepared wet

#### Scenario: Valid fractional reference
- **WHEN** aligned marker intervals alternate within the verified adjacent integer lengths
- **THEN** live phase stays verified within the epoch drift bound and each observed marker fixes its actual downbeat
- **AND** the accepted audio extent and render sample rate are preserved

### Requirement: Loop Cue schedules a complete future entry
An eligible Cue SHALL target the earliest future matching boundary with sufficient time for the full negative lead-in. Once queued, it SHALL start the lead-in in time for the earliest allowed arrival and place the wet downbeat at the observed boundary. Playback SHALL repeat until stopped or invalidated.

#### Scenario: Cue arrives too late for the next lead-in
- **WHEN** the next possible boundary leaves insufficient time for the complete lead-in
- **THEN** Cue targets the earliest later boundary that fits it and plays no partial launch at the missed boundary

#### Scenario: Marker and Cue share an offset
- **WHEN** a verified marker and Cue arrive at the same sample
- **THEN** the marker is evaluated before Cue and the launch targets a future boundary

### Requirement: Perform Stop and live sync loss end Loop wet
Perform Stop SHALL cancel queued playback or begin an immediate click-prevention transition of at most 5 ms. It SHALL take precedence over coincident Cue. Loss of live phase or playback authority SHALL cancel queued launches, fade active wet and require fresh authority and Cue as applicable.

#### Scenario: Capture commands and boundaries are independent
- **WHEN** Capture Start/Arm, Capture Stop or an ordinary phrase marker arrives during live Loop playback
- **THEN** the capture roles and marker do not request Perform Stop or Cue and a valid scheduled repeat continues

#### Scenario: Lost reference, seek or reset
- **WHEN** a required marker is late or invalid, a seek or reset occurs, or the live rate, tempo or session becomes incompatible
- **THEN** any queued launch is cancelled and active wet fades without altering dry in Dry + Wet
- **AND** re-lock alone leaves the scheduler in Standby until a fresh Cue

### Requirement: Loop wet composition counts each cycle contribution once
Loop playback SHALL mix every eligible prepared swell with source-relative placement and exclusive end bounds. All scheduled negative leads and tails that overlap another cycle SHALL be included exactly once. Phase correction SHALL be bounded and keep the prepared content at its native rate.

#### Scenario: Lead and tail span multiple cycles
- **WHEN** an eligible swell has a lead or tail longer than one phrase and overlaps other swells or repeats
- **THEN** the wet sum equals the sample-level sum of the intersecting scheduled contributions
- **AND** the first entry contains only contributions from the first scheduled cycle and later cycles

#### Scenario: A downbeat differs from the earliest prediction
- **WHEN** a valid marker arrives inside its rounding window
- **THEN** bounded correction aligns the wet downbeat to the observed sample and retains the complete lead and tail content

### Requirement: Loop playback pins coherent prepared revisions
Loop wet SHALL use a complete prepared revision associated with the accepted source and current render dependencies. Publication and retirement SHALL occur outside the callback. Resource admission SHALL account for the pinned data and worst-case overlapping contributions within the shared preparation budget.

#### Scenario: Edits invalidate a queued or active revision
- **WHEN** a source, placement or render dependency of the pinned revision is revoked
- **THEN** the obsolete queued launch is cancelled or the obsolete wet fades safely
- **AND** a fresh Cue can use the complete successor once it is ready

#### Scenario: Prepared workload exceeds the supported bound
- **WHEN** the pinned memory or worst-case scheduled overlap exceeds the admission budget
- **THEN** Cue reports the limiting resource and dry continues without partial or truncated wet
- **AND** the callback performs no allocation, deallocation, disk I/O, convolution, blocking or locking
