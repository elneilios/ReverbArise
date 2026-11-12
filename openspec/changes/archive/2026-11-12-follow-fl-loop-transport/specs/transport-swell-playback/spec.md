# Spec Delta

## ADDED Requirements

### Requirement: Perform groups Audition and Loop actions with their states

Perform in Loop Mode SHALL show one row with Audition and Loop section headings and a divider. Audition SHALL contain one Play/Stop action joined to Audition status, then the mix control. Loop SHALL contain Set Loop Start joined to its position, then Cue/Cancel/Stop joined to Loop playback status.

#### Scenario: Song and Loop use the shared Audition strip

- **WHEN** Perform displays a Song or a Loop source
- **THEN** the same Audition action, state field and mix control serve both source types
- **AND** only a Loop displays the Loop group

#### Scenario: Playback status changes

- **WHEN** Audition or Loop playback changes availability or phase
- **THEN** its paired state field shows the corresponding status while the other pair retains its own state
- **AND** the Loop command and its state field retain their widths as the action changes among Cue, Cancel, Stop and Stop Now

#### Scenario: Audition starts or stops

- **WHEN** Audition begins playback
- **THEN** the same action shows the red Stop glyph and exposes the Stop audition accessible name
- **AND** stopping restores the primary Play glyph in the same position

#### Scenario: The Perform row uses a compact layout

- **WHEN** the row switches between standard and compact widths
- **THEN** both headings retain the shared section-heading text role

#### Scenario: A saved position is absent

- **WHEN** Set Loop Start has no accepted position
- **THEN** its read-only field explicitly shows that no start is set

### Requirement: FL Studio matching repeats play automatically

With Live playback enabled and a prepared Loop in Perform, a compatible declared FL Studio repeat SHALL start wet playback from the host repeat phase when there is no active Playlist placement. Beat one SHALL align to the repeat start.

#### Scenario: Pattern or Playlist repeat starts or resumes mid-cycle

- **WHEN** FL Studio starts or seeks inside an eligible repeat range
- **THEN** wet joins the current phase without replaying elapsed lead-in
- **AND** subsequent verified wraps include overlapping leads and tails exactly once

#### Scenario: Transport stops or repeat eligibility changes

- **WHEN** transport stops or the active repeat becomes invalid
- **THEN** wet stops with the existing short transition
- **AND** a later eligible playback run can start automatically

### Requirement: FL Studio Loop placement follows the Playlist timeline

Perform SHALL let the user save one active Playlist placement per instance for the accepted Loop. Set Loop Start SHALL use the current valid Playlist playhead while stopped or playing. Its context menu SHALL offer manual bar-and-beat entry and clearing. With Live playback enabled, placement SHALL schedule repeating wet against absolute song position and take precedence over automatic repeat and manual Cue.

#### Scenario: The musician moves the stopped playhead

- **WHEN** FL Studio reports a new valid Playlist position while stopped and the musician activates Set Loop Start
- **THEN** the saved position becomes the reported position and the button opens no dialog

#### Scenario: First swell has a negative placement

- **WHEN** a saved Loop starts at bar 17 and its earliest wet starts two beats earlier in 4/4
- **THEN** starting transport by bar 16 beat 3 plays the complete first lead-in
- **AND** starting later or seeking mixes only the wet that belongs at and after the new position

#### Scenario: The user checks the saved start

- **WHEN** a placement is saved, changed or restored
- **THEN** Perform displays its saved bar and beat in the joined read-only field to the right of Set Loop Start
- **AND** the readout is distinguishable from the moving transport and from hover help

#### Scenario: An eligible repeat overlaps a saved placement

- **WHEN** FL Studio reports an eligible repeat with an active saved placement
- **THEN** only the placed schedule supplies wet and the user can remove the placement to return to automatic repeat

#### Scenario: A Stop is scheduled during a placed pass

- **WHEN** a placed Loop plays through multiple passes and receives a Stop command
- **THEN** it finishes its current pass and plays its remaining tails without starting another pass

#### Scenario: A different Loop is accepted

- **WHEN** the accepted Loop identity changes
- **THEN** a placement for the previous source does not start the replacement
- **AND** reopening the same source restores its compatible saved placement

### Requirement: Stop applies to transport-driven Loop playback

Stop Loop SHALL finish the current admitted pass and suppress future passes. Stop Now SHALL apply the existing short wet fade. Either action SHALL suppress further automatic entry for the current transport run. Stopping and restarting the host transport SHALL permit a fresh transport-driven entry.

#### Scenario: Stop Now before a placed passage

- **WHEN** a placed Loop is due later in the current playing transport run and the user invokes Stop Now
- **THEN** that passage stays silent for this run and remains saved for future playback

## MODIFIED Requirements

### Requirement: Loop Cue requires independent live phase

Manually cued Loop playback SHALL use the running FL Studio host grid, the verified Rack Clock grid, or a verified legacy Phrase reference. Capture timing SHALL supply only the source grid and exact recorded extent. Manual Cue standby SHALL check the active live route, pass dry and explain an unmet Cue prerequisite. Manually cued Clock and FL Studio host playback SHALL repeat at the accepted sample length.

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
- **THEN** wet follows the live timing without manual confirmation, and Dry + Wet keeps the live dry at unity when the output guard is idle

#### Scenario: Cue with a running grid

- **WHEN** a coherent prepared Loop is in Perform with manual Cue active and FL Studio Pattern or Playlist playback or Rack Clock supplies valid bar timing
- **THEN** UI and supported MIDI Cue are eligible without a recurring Phrase input

### Requirement: Loop Cue schedules a complete future entry

An eligible Cue SHALL target the earliest future downbeat with room for the full negative lead-in. Manual FL Studio Cue and Rack Clock without a Phrase reference SHALL use their respective bar grids. Rack with a usable Phrase reference SHALL use the phrase grid. A legacy Phrase-only route SHALL use the observed phrase boundary. Playback SHALL repeat until stopped or invalidated.

#### Scenario: Cue arrives too late for the next lead-in

- **WHEN** the next possible boundary leaves insufficient time for the complete lead-in
- **THEN** Cue targets the earliest later boundary that fits it and plays no partial launch at the missed boundary

#### Scenario: Marker and Cue share an offset

- **WHEN** a timing marker and Cue arrive at the same sample
- **THEN** the marker is evaluated before Cue and the launch targets a future boundary

#### Scenario: Rack Clock predicts a phrase-aligned entry

- **WHEN** a usable Phrase pulse supplies beat one alongside verified Rack Clock and the musician cues a prepared Loop
- **THEN** the scheduler projects phrase starts at the accepted Loop length from that origin and picks the first one with room for the full lead-in
- **AND** intermediate bar lines do not start the Loop

### Requirement: Perform Stop and live sync loss end Loop wet

In manual Cue, MIDI Perform Stop SHALL cancel a queued launch or finish the current pass without mixing the next lead-in. Repeated Stop notes SHALL leave an existing finish request in place. The Stop Loop button SHALL finish the current pass and then offer Stop Now. MIDI and UI Stop Now SHALL cancel a queued launch or start the short wet fade immediately. Live timing loss or playback authority loss SHALL cancel queued launches and fade active wet. When MIDI commands coincide, Stop Now SHALL take precedence over Stop, and either stop SHALL take precedence over Cue.

#### Scenario: Capture commands and boundaries are independent

- **WHEN** Capture Start/Arm, Capture Stop or an ordinary phrase marker arrives during live Loop playback
- **THEN** the capture roles and marker do not request Perform Stop or Cue and a valid scheduled repeat continues

#### Scenario: Lost reference, seek or reset

- **WHEN** a required clock or marker is late or invalid, a seek or reset occurs, or the live timing or session becomes incompatible
- **THEN** any queued launch is cancelled and active wet fades. Dry + Wet keeps dry at unity after the wet fade ends
- **AND** manually cued playback stays in Standby after timing returns until a fresh Cue

#### Scenario: Stop Loop is clicked during playback

- **WHEN** the musician clicks Stop Loop during the lead-in or current pass
- **THEN** the current pass continues to its end and the next lead-in is suppressed
- **AND** clicking Stop Now before completion starts the short fade immediately

#### Scenario: MIDI Stop and Stop Now are distinct

- **WHEN** a Stop note arrives while the Loop is playing
- **THEN** it finishes the current pass and suppresses the next lead-in even if another Stop note arrives
- **AND** a Stop Now note arriving before completion starts the short fade immediately

#### Scenario: FL Studio reaches a declared repeat boundary

- **WHEN** consecutive playing callbacks retain their musical mapping and repeat range and meet its exact end and start samples
- **THEN** a queued manual Cue and an active manually cued Loop retain their scheduled phase across the wrap
- **AND** an early jump to the same start sample is handled as a seek
