## MODIFIED Requirements

### Requirement: Loop Cue requires independent live phase

Loop live playback SHALL use the running FL Studio host grid, the verified Rack Clock grid, or a verified legacy Phrase reference. Capture timing SHALL supply only the source grid and exact recorded extent. Standby SHALL check the active live route, pass dry and explain an unmet Cue prerequisite. Clock and FL Studio host playback SHALL repeat at the accepted sample length.

#### Scenario: Cue with a running grid

- **WHEN** a coherent prepared Loop is in Perform and FL Studio Pattern or Playlist playback or Rack Clock supplies valid bar timing
- **THEN** UI and supported MIDI Cue are eligible without a recurring Phrase input

#### Scenario: Accepted capture lacks matching recurrence

- **WHEN** a legacy Phrase-only patch lacks recurrence matching the accepted take
- **THEN** the take remains valid and the legacy route cannot enable Cue

#### Scenario: Verify Phrase while still in Learn

- **WHEN** Rack receives valid Clock pulses in Learn
- **THEN** the current clock run can establish the grid used in Perform
- **AND** a legacy Phrase-only route may still verify recurrence for the accepted take in Learn
- **AND** Learn plays no Loop wet and entering Perform requires a fresh Cue

#### Scenario: Valid fractional reference

- **WHEN** required legacy marker intervals alternate within the verified adjacent integer lengths
- **THEN** phase stays verified within the epoch drift bound and each observed marker fixes its actual downbeat
- **AND** the accepted audio extent and render sample rate are preserved

#### Scenario: Dry audio is deliberately out of phase

- **WHEN** prepared wet is cued using valid live timing with dry audio offset from the target downbeat
- **THEN** wet follows the live timing without manual confirmation, and Dry + Wet preserves the dry input

### Requirement: Loop Cue schedules a complete future entry

An eligible Cue SHALL target the earliest future downbeat with room for the full negative lead-in. Playlist and Rack Clock routes SHALL schedule that downbeat from their respective bar grids. A legacy Phrase-only route SHALL use the observed phrase boundary. Playback SHALL repeat until stopped or invalidated.

#### Scenario: Cue arrives too late for the next lead-in

- **WHEN** the next possible boundary leaves insufficient time for the complete lead-in
- **THEN** Cue targets the earliest later boundary that fits it and plays no partial launch at the missed boundary

#### Scenario: Marker and Cue share an offset

- **WHEN** a timing marker and Cue arrive at the same sample
- **THEN** the marker is evaluated before Cue and the launch targets a future boundary

### Requirement: Perform Stop and live sync loss end Loop wet

MIDI Perform Stop SHALL cancel a queued launch or finish the current pass without mixing the next lead-in. Repeated Stop notes SHALL leave an existing finish request in place. The Stop Loop button SHALL finish the current pass and then offer Stop Now. MIDI and UI Stop Now SHALL cancel a queued launch or start the short wet fade immediately. Live timing loss or playback authority loss SHALL cancel queued launches and fade active wet. When MIDI commands coincide, Stop Now SHALL take precedence over Stop, and either stop SHALL take precedence over Cue.

#### Scenario: Stop Loop is clicked during playback

- **WHEN** the musician clicks Stop Loop during the lead-in or current pass
- **THEN** the current pass continues to its end and the next lead-in is suppressed
- **AND** clicking Stop Now before completion starts the short fade immediately

#### Scenario: MIDI Stop and Stop Now are distinct

- **WHEN** a Stop note arrives while the Loop is playing
- **THEN** it finishes the current pass and suppresses the next lead-in even if another Stop note arrives
- **AND** a Stop Now note arriving before completion starts the short fade immediately

#### Scenario: Capture commands and boundaries are independent

- **WHEN** Capture Start/Arm, Capture Stop or an ordinary phrase marker arrives during live Loop playback
- **THEN** the capture roles and marker do not request Perform Stop or Cue and a valid scheduled repeat continues

#### Scenario: Lost reference, seek or reset

- **WHEN** a required clock or marker is late or invalid, a seek or reset occurs, or the live timing or session becomes incompatible
- **THEN** any queued launch is cancelled and active wet fades without altering dry in Dry + Wet
- **AND** restored timing leaves the scheduler in Standby until a fresh Cue

#### Scenario: FL Studio reaches a declared repeat boundary

- **WHEN** consecutive playing callbacks retain their musical mapping and repeat range and meet its exact end and start samples
- **THEN** a queued Cue and an active Loop retain their scheduled phase across the wrap
- **AND** an early jump to the same start sample is handled as a seek

### Requirement: Loop playback pins coherent prepared revisions

Loop wet SHALL use a complete prepared revision of the accepted source at the host sample rate. Queued launches SHALL require current render dependencies. An audible pass SHALL retain its last complete mix while a compatible edit prepares, then crossfade to the complete successor at the current Loop position without re-cueing. An intentionally silent mix SHALL allow a playing Loop to keep its position. Publication and retirement SHALL occur outside the callback. Resource admission SHALL account for pinned data and worst-case overlapping contributions within the shared preparation budget.

#### Scenario: Fixed-grid Loop peak protection follows the rendered mix

- **WHEN** the host or Rack Clock grid plays prepared Loop wet audio at a fixed sample period
- **THEN** wet protection uses measured aggregate peaks, including the partial overlap at launch and finish, while preserving the requested balance below the ceiling
- **AND** variable phase playback retains a conservative peak bound

#### Scenario: Edits invalidate a queued or active revision

- **WHEN** placement or render dependencies change while a Loop is queued or playing
- **THEN** a queued launch with obsolete dependencies is cancelled
- **AND** an audible Loop keeps its phase and uses the last complete mix until a compatible successor crossfades in during repeating playback
- **AND** a change to the accepted source identity or sample-rate compatibility cancels or fades the old playback

#### Scenario: Prepared workload exceeds the supported bound

- **WHEN** the pinned memory or worst-case scheduled overlap exceeds the admission budget
- **THEN** Cue reports the limiting resource and dry continues without partial or truncated wet
- **AND** an existing compatible pass retains its last complete mix
- **AND** the callback performs no allocation, deallocation, disk I/O, convolution, blocking or locking
