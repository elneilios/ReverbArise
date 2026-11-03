# Spec Delta

## MODIFIED Requirements

### Requirement: Capture commits are recoverable
Capture passes SHALL stage immutable source and generation-context metadata, then publish one coherent validated source/context record atomically on explicit DAW Stop. Until acceptance, readers and saves SHALL retain the previous accepted record. Abort, validation/publication failure or incomplete capture SHALL NOT partially replace it. Prior committed revisions SHALL remain recoverable.

#### Scenario: Interrupted capture commit
- **GIVEN** a valid accepted map/context record exists and a multi-chunk pass is being staged or committed
- **WHEN** processing stops unexpectedly before durable publication completes
- **THEN** reopening the earlier state retains its prior source/context and no incomplete range or pass default is presented as committed

#### Scenario: Failure after multiple staged chunks
- **WHEN** disk-full, corrupt/nonfinite data, queue loss or publication failure occurs after earlier pass chunks were written
- **THEN** the pass is rejected, all prior accepted references remain unchanged and a useful error is shown
- **AND** provisional assets/context are not mistaken for accepted data or deleted along with sole committed copies

#### Scenario: Successful multi-chunk replacement
- **WHEN** Stop closes an admitted Recapture pass spanning chunks and seeks
- **THEN** all validated encountered source/context changes appear together as one accepted record instead of partial replacements

### Requirement: Song overview communicates capture coverage
The system SHALL provide a mode-appropriate full-song capture overview: compact in Perform and the sole large dry timeline in Learn. It SHALL distinguish captured audio, captured silence, uncaptured gaps, unavailable data and provisional recording with explicit inspectable states, keeping real absolute positions and accepted/provisional totals distinct.

#### Scenario: Inspect progressive coverage
- **GIVEN** the map contains non-contiguous captured ranges and at least one captured-silence range
- **WHEN** the user inspects the overview
- **THEN** audio, silence and gaps are visually distinguishable and inspectable at their absolute positions without relying solely on waveform height

#### Scenario: Inspect an unfinished pass
- **WHEN** admitted incoming source is displayed before Stop acceptance
- **THEN** it is labelled provisional/Recording, prior accepted coverage remains distinguishable and capture totals do not claim it has already committed

### Requirement: Capture progress distinguishes provisional and accepted source
Capture progress SHALL expose real admitted provisional spans/peaks separately from accepted source. Loops/seeks SHALL retain absolute coordinates. Ordinary Capture SHALL protect accepted coverage and earlier staged positions; armed Recapture SHALL resolve repeated positions deterministically. Provisional display SHALL NOT grant render/save/playback access or publish partial replacements.

#### Scenario: Loop within an uncommitted pass
- **WHEN** ordinary Capture revisits positions first recorded earlier in the same pass
- **THEN** those staged samples stay fixed, later uncovered positions can be added and no loop wrap is treated as a completed Stop commit
- **AND** incoming differences at protected positions do not falsely appear as accepted replacements in the display

#### Scenario: Repeated explicit replacement
- **WHEN** an armed pass records different performances at the same positions before Stop
- **THEN** its last completely received samples determine the validated replacement, provisional feedback reflects admitted data and the old accepted map remains readable until acceptance

#### Scenario: Continuous recording feedback
- **WHEN** valid audio arrives during a long pass before a storage batch is written
- **THEN** the timeline provides bounded progressive feedback from real admitted input rather than waiting for Stop or drawing fabricated peaks

#### Scenario: Cancel or supersede the displayed pass
- **WHEN** capture is cancelled, fails or is superseded by restore/new-session ownership
- **THEN** obsolete provisional display is discarded and cannot overwrite or relabel the current instance's accepted source

## ADDED Requirements

### Requirement: Accepted source retains deterministic generation provenance
Accepted source SHALL resolve the immutable generation context assigned to each admitted span. Ordinary additive capture SHALL preserve protected spans' provenance; Recapture SHALL replace only encountered provenance with its source. Saves/consumers SHALL observe one coherent accepted source/context record, including across loops, sparse seeks and instance copies.

#### Scenario: Additive capture crosses old and new coverage
- **WHEN** a pass with a different Default IR crosses old mapped audio/silence and an uncaptured interval
- **THEN** old source/context remains unchanged and only the newly admitted interval receives the new pass context

#### Scenario: Different contexts meet at an attack boundary
- **WHEN** onset confirmation or extraction spans two accepted contexts
- **THEN** a new target consistently resolves the context owning its arrival rather than using the most recently edited default

#### Scenario: Audio identity is unchanged
- **WHEN** identical PCM is encountered with unchanged context, or an explicit context-only update is accepted
- **THEN** unchanged audio-content identities and existing render/calibration dependencies are reused without unnecessary convolution

#### Scenario: Context metadata is unavailable
- **WHEN** accepted source is readable but required generation provenance or its IR cannot be verified
- **THEN** dry source stays preserved and generation reports the missing dependency rather than silently substituting current defaults

### Requirement: Progressive display cannot compromise realtime capture safety
Progress collection/publication SHALL use bounded metadata and coalesced updates without full-song PCM duplication, callback allocation, locks, file access or waiting. Display pressure SHALL be handled explicitly without corrupting source admission or inventing coverage. Playback and mode transitions SHALL retain the established audio safety contract.

#### Scenario: Large sparse recording with the editor open or closed
- **WHEN** a long pass visits many separated positions and the editor opens/closes or summaries reach their budget
- **THEN** display work stays bounded, current coverage remains truthful, source transactions retain exact admission semantics and audio callbacks remain nonblocking

#### Scenario: Saving while progress is visible
- **WHEN** a project is saved during a provisional display update
- **THEN** it references only the coherent accepted source/context and cannot restore the provisional overlay as durable audio
