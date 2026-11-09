# Spec Delta

## MODIFIED Requirements

### Requirement: Each arm owns one verified capture
Learn Arm and a verified external Start/Arm SHALL request the same one-take operation and pin its settings. The recorder SHALL accept original stereo input from the verified start sample inclusive through the verified end sample exclusive. Timing and role admission SHALL follow the Loop host admission contract.

#### Scenario: Confirm aligned timing
- **WHEN** Learn offers Arm for a compatible route
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

### Requirement: End signal requires an attributable request
End Signal SHALL require a verified start and a separately attributable Stop request. On the simplified route it SHALL record through the bar containing Stop and end immediately before the following downbeat. Learn SHALL disclose this behavior before arming.

#### Scenario: End signal is unavailable on the approved Rack wiring
- **WHEN** a route supplies only raw Host-FX gate release as a candidate end request
- **THEN** Learn explains why that release cannot finish an End Signal take and offers Duration when its prerequisites hold

#### Scenario: Verified request before or on a bar
- **WHEN** the simplified route receives Stop between downbeats or exactly on a downbeat
- **THEN** the take ends immediately before the next verified downbeat strictly after Stop
- **AND** a Stop on the last step of the bar includes the remainder of that step
- **AND** a Stop coincident with a downbeat includes the bar that begins there

#### Scenario: Reference or processing is lost before the boundary
- **WHEN** a missing clock, reset, timing change, or observed interruption prevents capture from reaching its verified closing boundary
- **THEN** the attempt cancels automatically and Learn explains the interruption
- **AND** the previous accepted source and dependent state remain intact

#### Scenario: Stop pending
- **WHEN** a recording End Signal attempt receives a valid Stop
- **THEN** Learn shows that recording continues until the closing beat
- **AND** further Stop or Start/Arm commands leave the same finish pending until the attempt settles or is interrupted

#### Scenario: Duration Stop at its configured boundary
- **WHEN** Stop and the configured Duration end arrive on the same sample
- **THEN** Stop cancels the attempt and the previous accepted source remains available

#### Scenario: Duration Stop while cadence is still unverified
- **WHEN** Arm is switched off or external Stop arrives after the Duration audio boundary while the armed take still awaits a corroborating beat
- **THEN** the take is cancelled and a later beat cannot publish it
- **AND** the previous accepted source remains available

## ADDED Requirements

### Requirement: Recapture Audio protects an accepted Loop
Learn SHALL offer Recapture Audio in Loop Mode. Both UI Arm and external Start/Arm SHALL require it to replace an accepted Loop. Permission SHALL be off on load and pinned for the attempt. A replacement SHALL still require successful settlement.

#### Scenario: First Loop capture
- **WHEN** there is no accepted Loop and Recapture Audio is off
- **THEN** either Arm source can request a first Loop if the other prerequisites hold

#### Scenario: Accepted Loop is protected
- **WHEN** a Loop is accepted and Recapture Audio is off
- **THEN** Learn explains that Recapture Audio is required to replace it
- **AND** neither Arm source can reserve a replacement attempt

#### Scenario: Sequenced replacement is enabled
- **WHEN** Recapture Audio is on and a fresh eligible Arm arrives after settlement
- **THEN** a new attempt can begin
- **AND** the last accepted Loop stays available until a valid replacement settles
