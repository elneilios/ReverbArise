# loop-phrase-capture Specification

## Purpose

Define how Learn records and accepts a phrase as a managed source while preserving the previous source and its dependencies during replacement.

## Requirements

### Requirement: Learn owns next capture intent
Learn Source Capture SHALL expose a Loop Mode toggle for the next capture. The accepted source kind and revision SHALL determine the source offered to Perform and Export. Selecting a capture type, arming or cancelling SHALL leave the accepted source and its dependent creative state intact.

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
- **AND** a pending replacement keeps the accepted phrase's markers visible
- **AND** Song generation actions remain visible with their availability explained

### Requirement: Each arm owns one verified capture
An explicit Loop arm SHALL pin its capture settings and admit at most one start and one candidate. The recorder SHALL accept original stereo input from the verified start sample inclusive through the verified end sample exclusive. Timing and role admission SHALL follow the Loop host admission contract.

#### Scenario: Confirm aligned timing
- **WHEN** Learn offers Arm for a compatible route
- **THEN** Learn shows the shared 4/4 timing requirements beside Arm
- **AND** arming confirms the shared clock, reset and aligned origin for audio and timing
- **AND** the plug-in separately verifies the timing evidence it receives before recording

#### Scenario: Verify host-declared timing
- **WHEN** a host supplies stable tempo, 4/4 meter and the Loop MIDI beat, bar and phrase markers at their actual sample offsets
- **THEN** Duration admission verifies the observed cadence against that declaration without identifying the host or requiring a fixed callback size
- **AND** loss of a selected timing reference invalidates the attempt before another reference can take over
- **AND** support claims for individual hosts remain limited to their integration evidence

#### Scenario: Switch Arm off
- **WHEN** an attempt is waiting, recording or settling and the user switches Arm off
- **THEN** Learn requests cancellation through the capture publication fence
- **AND** the existing accepted source remains available
- **AND** a completed attempt resets Arm without arming another take

#### Scenario: Capture a configured duration
- **WHEN** an armed Duration attempt observes a fresh verified phrase origin and the verified beat and meter evidence supports the configured Bars and Beats
- **THEN** it starts at that origin and ends at the observed musical boundary for the configured duration while the host continues running
- **AND** later phrase starts do not record again without a new arm

#### Scenario: Held-high start input
- **WHEN** an End signal attempt is armed with a high or unknown start gate
- **THEN** it waits for a verified low followed by a fresh rise before recording

#### Scenario: Duration timing is unavailable
- **WHEN** the route lacks verified beat timing, explicit meter or a usable phrase origin
- **THEN** Learn withholds Duration admission and explains the missing prerequisite

### Requirement: End signal requires an attributable request
End signal SHALL require a verified start and a separately verified end request on the configured route. On a bar-quantised route, completion SHALL use the first verified bar at or after the request, including a coincident bar. Learn SHALL disclose that policy before arming.

#### Scenario: End signal is unavailable on the approved Rack wiring
- **WHEN** the only candidate end signal is raw Host-FX gate release
- **THEN** Learn explains why End signal is unavailable and offers Duration when its prerequisites hold

#### Scenario: Verified request before or on a bar
- **WHEN** a verified route delivers a request between bars or exactly on a verified bar
- **THEN** the take ends immediately before the first verified bar sample at or after the request

#### Scenario: Reference or processing is lost before the boundary
- **WHEN** an observed interruption or missing marker makes the pending end unverifiable
- **THEN** the attempt fails without accepting audio and Learn explains the interruption

### Requirement: Validated takes replace the accepted source atomically
Settlement SHALL publish only complete, verified and useful phrase audio. The kind, managed source reference, revision, sample rate, exclusive extent and verified timing SHALL become visible together. Only the current instance, session and attempt may publish. A failed or obsolete candidate SHALL leave accepted source, edits, IR and render revisions intact.

#### Scenario: Replace an accepted source
- **WHEN** a current replacement passes validation and storage publication
- **THEN** Learn shows one newly accepted take and downstream source selection follows its kind and revision
- **AND** old source-dependent work cannot claim compatibility through coincident sample coordinates

#### Scenario: Invalid or cancelled replacement
- **WHEN** a replacement is cancelled or is partial, silent, corrupt, over budget or unable to publish
- **THEN** the prior accepted source and arrangement remain available and Learn shows the outcome

#### Scenario: Settlement races a restore or newer arm
- **WHEN** an old worker completion arrives after its session or attempt was invalidated
- **THEN** it cannot publish audio, update the accepted revision or change the accepted source kind

### Requirement: Project state preserves accepted and requested capture separately
Saved projects SHALL retain verified accepted-source metadata and the next-capture preference separately, using managed audio references. Older Song projects SHALL restore as Song with existing identities. Restore SHALL invalidate pending attempts and require a new explicit arm for Loop.

#### Scenario: Save during a pending replacement
- **WHEN** a project is saved while a new Loop attempt is armed, recording or settling
- **THEN** reopen restores the previously accepted source and saved preference without resuming that attempt

#### Scenario: Open a project with an accepted Loop
- **WHEN** a saved Loop project is reopened with a verified managed source
- **THEN** the same accepted identity, revision, sample rate, extent and musical timing are restored
- **AND** missing or invalid referenced material is reported as unavailable

#### Scenario: Instance isolation
- **WHEN** one instance replaces or cancels a take while another instance has a source or pending attempt
- **THEN** the second instance retains its own accepted and pending state

### Requirement: Loop capture keeps processing bounded
Loop capture SHALL bound event processing, buffer use and queued work per callback. The audio callback SHALL perform no allocation, deallocation, disk access, locking, waiting or UI work for capture. Learn SHALL distinguish waiting, recording, settling, accepted, cancelled and failed states.

#### Scenario: Storage or queue pressure during capture
- **WHEN** the worker is delayed or a capture resource is exhausted
- **THEN** dry audio continues and the attempt either settles within its verified limits or reports failure without accepting a partial take
