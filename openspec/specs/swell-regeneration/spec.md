# swell-regeneration Specification

## Purpose
Keep each edited swell tied to its captured source and IR while supporting selective background regeneration and explicit range-limited recapture without disrupting unrelated song sections.

## Requirements

### Requirement: Source-edge upgrades require explicit regeneration
Verified saved renders from before source-edge conditioning SHALL remain usable on restore with unchanged inputs. Regenerate Selected SHALL upgrade selected unfrozen renders; Regenerate All SHALL include older unfrozen ready renders while skipping ready renders already using current conditioning. Frozen renders SHALL remain unchanged. Upgrades SHALL preserve capture, swell identities and edits and SHALL not require recapture.

#### Scenario: Reopen an older saved project
- **WHEN** a project with verified unchanged older renders is restored
- **THEN** its saved wet assets remain current without automatic replacement solely due to the conditioning upgrade

#### Scenario: Upgrade existing overlapping swells
- **WHEN** Regenerate All is requested with older, current and frozen ready swells
- **THEN** older unfrozen renders are regenerated with new processing identities, current and frozen ready renders retain their assets, and capture samples and swell edits remain unchanged

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
The plugin SHALL provide Regenerate Selected and Regenerate All using committed capture without host playback. Selected SHALL process only selected confirmed swells; All SHALL process confirmed swells needing a current result, including disabled or muted definitions. Jobs SHALL freeze source/IR inputs, report progress, and publish only complete results matching the latest request for each swell.

#### Scenario: Generate selected targets
- **WHEN** the user regenerates one selected swell while another is already current
- **THEN** only the selected swell is rendered and the other's audio, edits, and identity remain unchanged

#### Scenario: Batch contains an invalid source
- **WHEN** Regenerate All includes one swell with a source gap and another with complete valid source
- **THEN** the first reports its own failure without publishing partial output or preventing the second from completing

#### Scenario: Edit while queued or rendering
- **WHEN** a swell is deleted or its source/IR changes before its job finishes
- **THEN** that completion cannot recreate the deleted swell or replace the newly requested result

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
Automatic refresh SHALL follow automatic-swell-processing, including saved controls, bounded background work and protection of manual edits. The legacy diagnostic recapture preference SHALL remain saved; its historical off default SHALL NOT disable the normal automatic workspace. Changed dependencies SHALL queue only affected work, coalescing obsolete requests. Failures SHALL retain labeled previous output and explicit recovery.

#### Scenario: Diagnostic recapture preference does not disable automatic workspace
- **WHEN** ordinary mapping changes a swell's source with the legacy recapture preference off
- **THEN** the normal automatic dependency policy still applies; the saved diagnostic preference does not suppress workspace refresh

#### Scenario: Rapid repeated recapture
- **WHEN** several commits update the same source while automatic refresh is enabled
- **THEN** obsolete pending work is coalesced and only results matching the latest accepted dependencies can become ready

### Requirement: Multiple swells do not create unbounded active render memory
The plugin SHALL keep analysis, asset I/O, preparation, convolution, and waveform construction outside the audio callback, enforce the displayed 256 MiB working budget across resident audio and active jobs, and report resource failures without truncating inputs. Stored swells SHALL not require every full render to be resident simultaneously. Active audition buffers SHALL remain pinned until callback acknowledgement.

#### Scenario: Many stored swells exceed resident capacity
- **WHEN** the session contains more full-render audio than the working budget permits in RAM
- **THEN** metadata remains editable, inactive render audio can be unloaded, and selecting or regenerating a swell either fits the budget or reports a specific error without damaging stored assets
