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
- **AND** Learn keeps the accepted phrase's markers visible while the replacement waits to record and hides them while the provisional waveform is displayed
- **AND** Generate Swells can use the accepted Loop's current eligible markers when generation prerequisites hold, while Generate on Stop retains its Song behavior

#### Scenario: Generate while a Loop clock continues
- **WHEN** Learn is idle with an accepted Loop, matching marker analysis and valid generation settings
- **THEN** explicit Generate Swells may publish definitions tied to that source revision while the clock continues
- **AND** a capture restart, source replacement, changed marker inputs or incompatible workflow transition rejects stale work and preserves unrelated manual and frozen definitions

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

### Requirement: Learn shows provisional Loop waveform progress
Learn SHALL show the current Loop take's recorded waveform and growing extent from the verified phrase start before acceptance. The waveform SHALL remain provisional until the take settles. Preview data SHALL be display-only and SHALL NOT grant access to the take for generation, playback, export or project save.

#### Scenario: Show audio before a Loop ends
- **WHEN** the active Loop take starts recording and receives audio
- **THEN** Learn shows the take's waveform in phrase-local coordinates starting at sample zero
- **AND** the displayed extent grows with the recorded audio while the take is still in progress
- **AND** any previous source retains its accepted identity until the new take settles successfully

#### Scenario: Detailed coverage reaches its preview limit
- **WHEN** detailed coverage reaches its display limit during an otherwise valid Loop take
- **THEN** Learn continues to show the growing recorded extent and a bounded waveform summary derived from the recorded audio
- **AND** inspection reports that coverage detail is unavailable
- **AND** capture and settlement continue under the existing acceptance rules

#### Scenario: Replace the provisional waveform after settlement
- **WHEN** the Loop take is accepted
- **THEN** Learn replaces the provisional display with the accepted waveform at its verified phrase extent

#### Scenario: Abandon a provisional waveform
- **WHEN** the current Loop take is cancelled, rejected or invalidated by restore
- **THEN** Learn removes its provisional waveform and shows the applicable accepted source waveform when available

### Requirement: Project state preserves accepted and requested capture separately
Saved projects SHALL retain verified accepted-source metadata and the next-capture preference separately, using managed audio references. Older Song projects SHALL restore as Song with existing identities. Restore SHALL invalidate pending attempts and require a new explicit arm for Loop.

#### Scenario: Save during a pending replacement
- **WHEN** a project is saved while a new Loop attempt is armed, recording or settling
- **THEN** reopen restores the previously accepted source and saved preference without resuming that attempt
- **AND** a late completion from an invalidated attempt cannot replace the restored take or its dependent edits

#### Scenario: Open a project with an accepted Loop
- **WHEN** a saved Loop project is reopened with a verified managed source
- **THEN** the same accepted identity, revision, sample rate, extent and musical timing are restored
- **AND** the exact referenced audio, swell edits and compatible verified asset references are retained
- **AND** missing or invalid referenced material is reported as unavailable

#### Scenario: Instance isolation
- **WHEN** one instance replaces or cancels a take while another instance has a source or pending attempt
- **THEN** the second instance retains its own accepted and pending state

#### Scenario: Reopen protected and in Standby
- **WHEN** a current-build project with an accepted Loop reopens after a save with Recapture Audio enabled or Loop playback queued or active
- **THEN** Learn starts disarmed with Recapture Audio off, and neither UI nor external Start/Arm can replace the accepted Loop until permission is enabled
- **AND** live output starts in dry Standby, and wet playback requires a fresh Cue and verified live timing
- **AND** the saved capture-local grid and a held-high control at restore cannot grant live timing, Arm or Cue

### Requirement: Loop capture keeps processing bounded
Loop capture SHALL bound event processing, buffer use and queued work per callback. The audio callback SHALL perform no allocation, deallocation, disk access, locking, waiting or UI work for capture. Learn SHALL distinguish waiting, recording, settling, accepted, cancelled and failed states.

#### Scenario: Storage or queue pressure during capture
- **WHEN** the worker is delayed or a capture resource is exhausted
- **THEN** dry audio continues and the attempt either settles within its verified limits or reports failure without accepting a partial take

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
