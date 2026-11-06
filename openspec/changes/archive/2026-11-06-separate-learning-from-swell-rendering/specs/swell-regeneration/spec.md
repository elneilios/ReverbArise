# Spec Delta

## MODIFIED Requirements

### Requirement: Automatic regeneration is explicit and bounded
The swell engine SHALL asynchronously maintain verified renders for accepted unfrozen definitions whenever material dependencies change, independently of selected mode, detection settings, marker Generate on Stop and legacy refresh preferences. Only affected work SHALL queue, obsolete requests SHALL coalesce, and failures SHALL retain prior assets without claiming current readiness.

#### Scenario: Diagnostic recapture preference does not disable automatic workspace
- **WHEN** ordinary mapping changes a swell's source with the legacy recapture preference off
- **THEN** the normal automatic dependency policy still applies; the saved diagnostic preference does not suppress workspace refresh

#### Scenario: Rapid repeated recapture
- **WHEN** several commits update the same source regardless of detection settings
- **THEN** obsolete pending work is coalesced and only results matching the latest accepted dependencies can become ready

## ADDED Requirements

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
