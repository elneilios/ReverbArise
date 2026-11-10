# Live output peak guard

## ADDED Requirements

### Requirement: Realtime Dry + Wet output has linked peak protection

Realtime Song and Loop Dry + Wet SHALL use one linked gain on the combined stereo output, with a −1 dBFS sample-peak ceiling when a wet contribution is mixed. The guard SHALL add no latency and return to exactly unity after recovery. Existing wet calibration, including the approved +6 dB target, and existing wet overlap protection SHALL remain independent.

#### Scenario: Dry and wet peaks coincide

- **WHEN** prepared wet and incoming dry sum above the ceiling in active live Dry + Wet
- **THEN** both channels receive the same gain and the emitted sample peaks stay within the ceiling
- **AND** the gain reduction keeps the dry-to-wet ratio at that sample and does not move the swell

#### Scenario: The live sum has headroom

- **WHEN** the summed peak remains below the ceiling and the guard is idle
- **THEN** incoming dry is at unity and prepared wet follows the saved gain and protection plan

#### Scenario: Protection recovers

- **WHEN** a peak subsides during active live playback
- **THEN** guard gain returns to unity without exceeding the ceiling on later peaks
- **AND** the result is independent of audio callback block boundaries

#### Scenario: Dry-only passthrough and other output paths

- **WHEN** there is no active live wet contribution or its stop transition has finished
- **THEN** Dry + Wet passes the incoming dry unchanged and a subsequent live run starts at unity
- **AND** Wet Only, Audition, Learn capture, host offline rendering and internal Export keep their existing protection and timing

## MODIFIED Requirements

### Requirement: Live routing is independent of manual monitoring
Within Live, saved enable, Dry + Wet/Wet Only routing and wet level SHALL remain independent of audition, defaulting to enabled Dry + Wet at 0 dB. Dry + Wet SHALL add prepared wet to incoming dry at unity before applying live output peak protection. Wet Only SHALL suppress dry only during enabled valid Live playback. Capture/Finalise SHALL preserve dry passthrough without modifying saved Live preferences.

#### Scenario: Play in context
- **WHEN** a prepared project plays in Live default Dry + Wet
- **THEN** output is the live input plus protected positioned wet, with the linked live output guard acting only when needed and with no captured-dry replay or audition blend/trim

#### Scenario: Route wet only and disable
- **WHEN** Wet Only is selected during Live playback and live playback is subsequently disabled
- **THEN** enabled Wet Only outputs prepared wet and disabling restores dry/removes wet with disclosed short routing ramps

#### Scenario: Enter Finalise from Wet Only
- **WHEN** Finalise is entered from Live with Wet Only selected
- **THEN** scheduled wet stops through the existing bounded transition, normal DAW output becomes dry passthrough and returning to Live restores the saved preference

### Requirement: Loop Cue requires independent live phase

Loop live playback SHALL use the running FL Studio host grid, the verified Rack Clock grid, or a verified legacy Phrase reference. Capture timing SHALL supply only the source grid and exact recorded extent. Standby SHALL check the active live route, pass dry and explain an unmet Cue prerequisite. Clock and FL Studio host playback SHALL repeat at the accepted sample length.

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

- **WHEN** a coherent prepared Loop is in Perform and FL Studio Pattern or Playlist playback or Rack Clock supplies valid bar timing
- **THEN** UI and supported MIDI Cue are eligible without a recurring Phrase input

### Requirement: Perform Stop and live sync loss end Loop wet

MIDI Perform Stop SHALL cancel a queued launch or finish the current pass without mixing the next lead-in. Repeated Stop notes SHALL leave an existing finish request in place. The Stop Loop button SHALL finish the current pass and then offer Stop Now. MIDI and UI Stop Now SHALL cancel a queued launch or start the short wet fade immediately. Live timing loss or playback authority loss SHALL cancel queued launches and fade active wet. When MIDI commands coincide, Stop Now SHALL take precedence over Stop, and either stop SHALL take precedence over Cue.

#### Scenario: Capture commands and boundaries are independent

- **WHEN** Capture Start/Arm, Capture Stop or an ordinary phrase marker arrives during live Loop playback
- **THEN** the capture roles and marker do not request Perform Stop or Cue and a valid scheduled repeat continues

#### Scenario: Lost reference, seek or reset

- **WHEN** a required clock or marker is late or invalid, a seek or reset occurs, or the live timing or session becomes incompatible
- **THEN** any queued launch is cancelled and active wet fades. Dry + Wet keeps dry at unity after the wet fade ends
- **AND** restored timing leaves the scheduler in Standby until a fresh Cue

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
- **THEN** a queued Cue and an active Loop retain their scheduled phase across the wrap
- **AND** an early jump to the same start sample is handled as a seek
