# Spec Delta

## MODIFIED Requirements

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

## ADDED Requirements

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
