# Spec Delta

## MODIFIED Requirements

### Requirement: Learn owns next capture intent
Learn Source Capture SHALL expose a Loop Mode toggle for the next capture and one shared Arm above the Song and Loop settings. The accepted source kind and revision SHALL determine the source offered to Perform and Export. Selecting a capture type, arming or cancelling SHALL leave the accepted source and its dependent creative state intact.

#### Scenario: Select Loop while a Song is accepted
- **WHEN** the user enables Loop and arms a capture with an accepted Song source
- **THEN** Learn shows that the next capture is Loop and the accepted source remains Song
- **AND** the existing Song source and arrangement remain authoritative until a replacement succeeds

#### Scenario: Re-arm for a replacement
- **WHEN** a take has settled successfully and the user arms again
- **THEN** Learn starts one new replacement attempt and continues to identify the prior take as accepted

#### Scenario: Detect events in an accepted Loop
- **WHEN** Detect events is enabled and Learn has an accepted Loop
- **THEN** Learn shows the automatic marker controls and detects markers in the accepted phrase at the selected sensitivity
- **AND** Learn keeps the accepted phrase's markers visible while the replacement waits to record and hides them while the provisional waveform is displayed
- **AND** the explicit Learn generation action can use the accepted Loop's current eligible markers when generation prerequisites hold, while Generate on Stop retains its Song behavior

#### Scenario: Generate while a Loop clock continues
- **WHEN** Learn is idle with an accepted Loop, matching marker analysis and valid generation settings
- **THEN** the explicit Learn generation action can publish definitions tied to that source revision while the clock continues
- **AND** a capture restart, source replacement, changed marker inputs or incompatible workflow transition rejects stale work and leaves all accepted definitions intact

#### Scenario: Regenerate while FL Studio repeats
- **WHEN** the musician leaves Perform for Learn, explicitly regenerates from the accepted Loop, and a matching FL Studio repeat is still running
- **THEN** automatic wet playback resumes at the host repeat phase once the rebuilt plan is ready, subject to Live playback and the existing route requirements
- **AND** an explicit Loop Stop still suppresses automatic playback until a later host transport run

### Requirement: Each arm owns one verified capture
The shared Learn Arm in Loop Mode and a verified external Start/Arm SHALL request the same one-take operation and pin its settings. The recorder SHALL accept original stereo input from the verified start sample inclusive through the verified end sample exclusive. Timing and role admission SHALL follow the Loop host admission contract.

#### Scenario: Confirm aligned timing
- **WHEN** Learn offers the shared Arm for a compatible Loop route
- **THEN** Learn explains the aligned audio and clock setup and the meter used for capture
- **AND** arming confirms the setup
- **AND** the plug-in separately verifies timing from the received evidence before accepting the take

#### Scenario: Verify host-declared timing
- **WHEN** a host supplies stable tempo, 4/4 meter and the Loop MIDI beat, bar and phrase markers at their actual sample offsets on the existing marker route
- **THEN** Duration admission verifies the observed cadence against that declaration without identifying the host or requiring a fixed callback size
- **AND** loss of a selected timing reference invalidates the attempt before another reference can take over
- **AND** support claims for individual hosts remain limited to their integration evidence

#### Scenario: Switch Arm off
- **WHEN** the user switches Arm off or a verified external Stop arrives for the current waiting or recording attempt
- **THEN** a waiting attempt is withdrawn and a recording Duration attempt is cancelled
- **AND** a recording End Signal attempt instead queues completion under its closing-boundary policy
- **AND** the existing accepted source remains available until a candidate settles successfully
- **AND** a completed attempt resets Arm without arming another take

#### Scenario: Capture a configured duration
- **WHEN** an armed Duration attempt observes a usable start with the evidence required by its timing route
- **THEN** the existing marker route starts on a verified phrase and the simplified route starts on the first beat at or after the arm
- **AND** recording ends at the observed musical boundary for the configured Bars or Beats while the host continues running
- **AND** later phrase starts and repeated Start/Arm commands cannot record again without a fresh arm after the current attempt settles

#### Scenario: Held-high start input
- **WHEN** a gate-based End Signal route is armed with a high or unknown start gate
- **THEN** it waits for a verified low followed by a fresh rise before recording

#### Scenario: Duration timing is unavailable
- **WHEN** the route lacks the verified beat timing, explicit meter, or usable origin needed for Duration
- **THEN** Learn explains the missing prerequisite
- **AND** the simplified route can wait for the clock after arming but cannot publish an unverified take

#### Scenario: Start and first sequencer step coincide
- **WHEN** an eligible external Start/Arm arrives on the same sample as a beat on the simplified route
- **THEN** capture starts at that sample and uses it as the local downbeat
- **AND** a Start/Arm arriving between beats waits for the next beat
