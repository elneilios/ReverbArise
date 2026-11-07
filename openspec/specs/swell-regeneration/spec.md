# swell-regeneration Specification

## Purpose
Keep each edited swell tied to its captured source and IR while supporting selective background regeneration and explicit range-limited recapture without disrupting unrelated song sections.

## Requirements

### Requirement: Source-edge upgrades require explicit regeneration
Verified saved renders from before source-edge conditioning SHALL remain usable on restore with unchanged inputs. Regenerate Selected SHALL upgrade selected unfrozen renders. Regenerate All SHALL force fresh renders for eligible older and current Ready swells; frozen renders SHALL remain unchanged. Explicit regeneration SHALL preserve capture, swell identities and edits and SHALL not require recapture.

#### Scenario: Reopen an older saved project
- **WHEN** a project with verified unchanged older renders is restored
- **THEN** its saved wet assets remain current without automatic replacement solely due to the conditioning upgrade

#### Scenario: Upgrade existing overlapping swells
- **WHEN** Regenerate All is requested with older, current and frozen ready swells
- **THEN** eligible older and current unfrozen swells receive fresh render requests, frozen renders retain their assets, and capture samples and swell edits remain unchanged

#### Scenario: Failed upgrade
- **WHEN** regeneration fails or is superseded
- **THEN** the previous complete wet asset and captured source remain stored and no partial result becomes ready

### Requirement: Each swell tracks its own render dependencies and state
The system SHALL identify each render by swell identity, source interval/content dependency, IR content/preparation, fixed timing, and engine version. It SHALL expose per-swell queued/rendering/ready/stale/error states. Source or IR changes SHALL invalidate only affected swells; shape, name, and navigation changes SHALL not reconvolve. Stale output SHALL remain inspectable but not auditionable as current.

#### Scenario: Partial recapture
- **WHEN** a committed recapture changes samples used by one of two independent swells
- **THEN** only the dependent swell becomes stale and the other retains its valid render

#### Scenario: Identical recapture
- **WHEN** a committed recapture contains exactly the same samples over a swell's source interval
- **THEN** the unchanged content does not invalidate or regenerate that swell merely because capture revisions or chunk boundaries changed

### Requirement: Regeneration operates on the requested set
The plugin SHALL provide Regenerate Selected and Regenerate All using committed capture without host playback. Selected SHALL process only selected confirmed swells. All SHALL force fresh work for eligible confirmed swells regardless of current readiness, including disabled or muted definitions, while skipping frozen or importing-IR targets. Jobs SHALL freeze source/IR inputs, report progress, and publish only complete results matching the latest request for each swell.

#### Scenario: Generate selected targets
- **WHEN** the user regenerates one selected swell while another is already current
- **THEN** only the selected swell is rendered and the other's audio, edits, and identity remain unchanged

#### Scenario: Batch contains an invalid source
- **WHEN** Regenerate All includes one swell with a source gap and another with complete valid source
- **THEN** the first reports its own failure without publishing partial output or preventing the second from completing

#### Scenario: Edit while queued or rendering
- **WHEN** a swell is deleted or its source/IR changes before its job finishes
- **THEN** that completion cannot recreate the deleted swell or replace the newly requested result

#### Scenario: Repeat Regenerate All on current results
- **WHEN** eligible swells are Ready using current processing and the user explicitly invokes Regenerate All again
- **THEN** each eligible target receives a fresh render request through bounded background work, even though its inputs and processing version are unchanged
- **AND** the action does not recapture source, alter edits or override frozen/importing ownership

### Requirement: Recapture and regenerate has an explicit armed interval
Recapture & Regenerate SHALL arm one finite absolute source interval and identify dependent swells. The plugin SHALL instruct the user where to move the FL Studio playhead and press Play, admit dry input only inside that interval, and preserve all outside capture. It SHALL not command transport. Regeneration SHALL wait for committed complete fresh coverage of the armed interval; previous coverage SHALL not conceal missing fresh input.

#### Scenario: Host block crosses an armed boundary
- **WHEN** playback processes a block partly inside and partly outside the armed interval
- **THEN** only the intersecting dry samples replace capture and outside samples remain unchanged

#### Scenario: Stop, seek, or drop before completion
- **WHEN** a recapture stops early, skips samples, or drops input blocks
- **THEN** its incomplete fresh coverage remains visible, missing portions are not counted from the old map, and automatic regeneration waits until all required fresh samples have committed or the user cancels

#### Scenario: Successful manual recapture
- **WHEN** the user plays the entire armed range and its fresh source commits successfully
- **THEN** only affected dependent swells are queued for regeneration and unrelated captured sections and swells remain intact

### Requirement: Automatic regeneration is explicit and bounded
The swell engine SHALL asynchronously maintain verified renders for accepted unfrozen definitions whenever material dependencies change, independently of selected mode, detection settings, marker Generate on Stop and legacy refresh preferences. Only affected work SHALL queue, obsolete requests SHALL coalesce, and failures SHALL retain prior assets without claiming current readiness.

#### Scenario: Diagnostic recapture preference does not disable automatic workspace
- **WHEN** ordinary mapping changes a swell's source with the legacy recapture preference off
- **THEN** the normal automatic dependency policy still applies; the saved diagnostic preference does not suppress workspace refresh

#### Scenario: Rapid repeated recapture
- **WHEN** several commits update the same source regardless of detection settings
- **THEN** obsolete pending work is coalesced and only results matching the latest accepted dependencies can become ready

### Requirement: Multiple swells do not create unbounded active render memory
The plugin SHALL keep analysis, asset I/O, preparation, convolution, and waveform construction outside the audio callback, enforce the displayed 256 MiB working budget across resident audio and active jobs, and report resource failures without truncating inputs. Stored swells SHALL not require every full render to be resident simultaneously. Active audition buffers SHALL remain pinned until callback acknowledgement.

#### Scenario: Many stored swells exceed resident capacity
- **WHEN** the session contains more full-render audio than the working budget permits in RAM
- **THEN** metadata remains editable, inactive render audio can be unloaded, and selecting or regenerating a swell either fits the budget or reports a specific error without damaging stored assets

### Requirement: Explicit bulk regeneration provides visible command feedback
Regenerate All SHALL promptly show visible, accessible feedback on its action surface stating whether work was queued, skipped or rejected. Feedback SHALL distinguish admission from completed rendering and explain requests with no eligible targets. It SHALL preserve existing higher-priority errors and SHALL not become saved musical state.

#### Scenario: Work was accepted
- **WHEN** Regenerate All queues eligible targets and skips frozen or importing targets
- **THEN** the action surface shows the queued count and applicable skipped counts even if rendering finishes before the next routine UI refresh
- **AND** per-swell progress/readiness remains authoritative for completion and failures

#### Scenario: Nothing can be queued
- **WHEN** the batch is empty, all targets are frozen/importing, or workflow admission rejects the command
- **THEN** visible feedback explains why no work was queued without suggesting successful regeneration

#### Scenario: Feedback ownership changes
- **WHEN** another bulk request is made or the owning session is replaced
- **THEN** feedback is replaced by the new command outcome or cleared for the new session, without modifying source, saved edits or higher-priority notices

### Requirement: One engine owns render readiness across workflows
Source capture, marker detection/application and asset recovery SHALL supply accepted inputs to one render-maintenance pipeline. No workflow screen or recovery controller SHALL require a separate generation request to make an existing eligible swell current. Maintenance SHALL NOT create markers, change assignments or start audition/transport.

#### Scenario: Re-learn with detection and marker generation off
- **WHEN** complete replacement source is accepted for an existing unfrozen swell with a valid assigned IR while detection and Generate on Stop are off
- **THEN** its material change queues automatic render maintenance without a Generate click, marker replacement or saved-policy changes

#### Scenario: Partial or rejected source
- **WHEN** a source pass is provisional, incomplete, cancelled or rejected
- **THEN** it cannot invalidate accepted render authority or publish a replacement; recovery retains accepted definitions and surviving assets

#### Scenario: Editor closed or another mode selected
- **WHEN** accepted material dependencies change while the editor is closed or a different workflow screen is selected
- **THEN** the same background eligibility and publication rules apply without granting new capture, editing or wet-output permissions

### Requirement: Full wet material and presentation dependencies are separate
The full wet dependency identity SHALL include verified source content/footprint, assigned IR/preparation and material timing. Fades, curves, level, name, selection and pure wet placement SHALL reuse full wet audio. A genuine source-footprint extension SHALL require complete extra input and normal material maintenance, not silence or truncation.

#### Scenario: Shape and name edits
- **WHEN** fades, envelope curves, level or name change within the prepared footprint
- **THEN** convolution and IR resampling counts stay unchanged; only derived presentation/playback data updates

#### Scenario: Material and identical input changes
- **WHEN** accepted source, assigned IR or compatible material timing changes, or identical audio is republished with different chunk boundaries
- **THEN** only genuinely changed material queues work and equivalent unchanged inputs retain verified full wet audio

### Requirement: Restoration resolves ordinary engine dependencies
Verified compatible saved renders SHALL be reused without recapture, resampling or convolution. Missing unfrozen renders SHALL be rebuilt through the common engine only when source, IR, coverage and timing verify. Missing source/IR or incompatible timing SHALL remain explicit and SHALL NOT be replaced with fabricated silence or implicit source resampling.

#### Scenario: Complete portable restore
- **WHEN** an exact compatible backup restores all accepted dependencies
- **THEN** hashes/metadata remain exact and recapture, convolution and resampling counters remain zero

#### Scenario: Missing wet file with intact inputs
- **WHEN** an unfrozen swell lacks its wet file but all material inputs verify
- **THEN** ordinary engine maintenance rebuilds it without a recovery-specific render command

#### Scenario: Frozen or older supported sound
- **WHEN** a render is frozen or an unchanged verified older supported render is reopened
- **THEN** the engine preserves its exact saved sound; a missing frozen result requires explicit unfreezing or exact restoration, and algorithm upgrades retain their existing explicit opt-in rule

### Requirement: Publication retains current input authority
Render jobs SHALL pin immutable accepted dependencies and publish only if session, swell identity, material generation and dependency identity still match. Queues, memory and processing SHALL remain bounded off the audio callback. Superseded, failed or deleted-target jobs SHALL retain durable history and SHALL NOT publish current Ready output.

#### Scenario: Rapid updates and active readers
- **WHEN** several source/IR edits arrive during rendering while audio uses an older accepted buffer
- **THEN** obsolete jobs coalesce/cancel, only the latest eligible result publishes, reader pins remain valid and no callback disk I/O, blocking or unbounded allocation occurs
