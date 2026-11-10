## MODIFIED Requirements

### Requirement: Loop playback controls report actual readiness

An accepted Loop SHALL show one playback command and separate status beside Audition in Perform. Cue Loop SHALL require valid live timing and coherent prepared wet. Cancel Loop SHALL cancel a queued start. Stop Loop SHALL finish the current pass and suppress the next lead-in. Stop Now SHALL fade wet immediately while dry continues. These commands SHALL act independently of Audition. Song SHALL retain its existing composition.

#### Scenario: Perform reads the current Loop position

- **WHEN** an accepted Loop is in Perform
- **THEN** the existing footer reports its captured tempo and meter
- **AND** during the pass Bar and Beat follow the Loop playhead, returning to Bar 1, Beat 1 each time the Loop repeats, including in Rack without host song position
- **AND** position is unavailable until beat one and after playback ends

#### Scenario: Phase, preparation and playback change

- **WHEN** a Loop enters Perform with unavailable live timing, unavailable preparation or disabled live output
- **THEN** Cue is disabled with the specific actionable reason and dry remains available
- **AND** valid live timing and prepared wet enable Cue Loop automatically with Ready status
- **AND** the command becomes Cancel Loop with Waiting for bar status when queued, then Stop Loop with Playing lead-in or Playing loop status when wet starts
- **AND** Stop Loop becomes Stop Now with Finishing loop status until completion or an immediate stop
- **AND** the command is disabled while Fading out and offers Cue Loop again when ready

#### Scenario: Playback controls explain why they are disabled

- **WHEN** Audition or Cue Loop is unavailable in Perform
- **THEN** the control is disabled and its hover help explains the current prerequisite
- **AND** if the host rate differs from the accepted Loop's rate, both controls report the rate required to play the captured audio

#### Scenario: Lead-in wraps into the previous pass

- **WHEN** Loop playback repeats with a scheduled lead-in for the next pass
- **THEN** one yellow playhead follows the current phrase and wraps at the observed boundary
- **AND** the wet lane shows a faint waveform of the next pass's prepared wet mix near the end of the phrase, regardless of the selected swell
- **AND** a pending Stop hides the suppressed next lead-in

#### Scenario: Compact review layout

- **WHEN** Song and Loop Perform are viewed at 900×600 at 100% and 125%
- **THEN** the Loop command and status fit on the Audition row with a separator and the command remains keyboard accessible
- **AND** the rest of the shared editor stays usable and Song adds no Loop controls
