# Spec Delta

## MODIFIED Requirements

### Requirement: Validated useful Stop commits automatically enter Live
An explicit DAW Stop SHALL close the pass once and settle asynchronously. Useful source acceptance SHALL settle in Learn. Only enabled Auto-generate on Stop with effective Automatic Swells SHALL request the shared Generate transaction after matching final analysis; Perform entry SHALL require a nonempty changed generation commit, without waiting for wet renders. Empty, mapped-only, aborted, incompatible or failed passes, and sessions left without useful material, SHALL remain Capture with truthful results and prior valid data intact.

#### Scenario: Stop while wet work remains pending
- **WHEN** source validation and an eligible shared generation transaction succeed while wet jobs are still queued or rendering
- **THEN** Perform is selected atomically with the successful generation commit and reports unfinished wet preparation honestly

#### Scenario: Empty or failed pass
- **WHEN** Stop closes an empty/mapped-only pass, publication fails, or no useful material remains after a silence-only replacement
- **THEN** Capture remains selected, the real reason is shown and earlier committed source remains available

#### Scenario: Identical explicit recapture
- **WHEN** an explicitly armed pass receives useful mapped audio identical to the accepted source and validation succeeds
- **THEN** Recapture disarms, source identity is reused and Learn remains selected unless the opted-in shared transaction commits a genuine changed automatic result

#### Scenario: Recapture deliberately replaces a note with silence
- **WHEN** an explicitly armed pass successfully replaces mapped samples with silence and other useful committed source remains
- **THEN** Recapture disarms, including for identical already-mapped silence; unchanged PCM retains identity, and Perform entry still requires opted-in successful changed generation

### Requirement: Creative mode names are separate from durable identity
User-facing mode names SHALL be Learn, Perform and Export for the existing capture, live and finalise behaviors respectively. Visible mode guidance, rejection reasons and accessible text SHALL use those names consistently. Saved mode identifiers and plugin identities SHALL remain compatible; versioned generation preferences and revised optional Stop transitions SHALL follow their explicit migration contracts. Audio routing SHALL retain its existing mode semantics. Functional concepts such as source capture, Recapture and live wet SHALL retain their accurate meaning.

#### Scenario: Reopen saved workflow modes
- **WHEN** supported projects saved with Capture, Live or Finalise identifiers reopen
- **THEN** their corresponding Learn, Perform or Export mode is displayed with the same source, swells, edits, readiness rules and output behavior, without renumbering modes or implicit recording/audition; the separately versioned Auto-generate preference follows its legacy migration
- **AND** projects lacking saved mode metadata still enter Learn with Recapture off under the existing legacy policy

#### Scenario: Save and restore a newly labelled mode
- **WHEN** a valid mode is selected through the new navigation and the host saves/reopens the project
- **THEN** the original durable mode representation round-trips, the chosen display label returns, and a real mode change retains existing host save notification behavior

#### Scenario: Automatic transition while renders prepare
- **WHEN** the opted-in Stop route commits one or more new or genuinely regenerated automatic swells from useful accepted source
- **THEN** Perform becomes selected after the generation commit without waiting for wet jobs, with honest background preparation status
- **AND** mapped-only, fresh all-silent, failed and erased-last-useful-source outcomes retain their existing Learn behavior

#### Scenario: Existing eligibility and routing
- **WHEN** a user requests Perform or Export with silent-only, incompatible, frozen-recovery or ready-render material
- **THEN** the existing eligibility rules decide the request without being weakened by presentation changes
- **AND** Learn captures only under its existing policy, Perform schedules wet without capture, and Export leaves DAW output dry with only explicitly initiated supported preview

## ADDED Requirements

### Requirement: Optional automatic generation follows useful Stop only
Auto-generate on Stop SHALL admit generation only after successfully accepted useful Stop with effective Automatic Swells enabled, waiting for final matching analysis. Toggling ON, restoring state, editing settings or entering Learn SHALL NOT trigger generation. OFF SHALL cancel pending automatic intent before commit. Changed settings/source/default/timing or workflow authority SHALL invalidate the request without retargeting it to later work.

#### Scenario: Preference changes without capture
- **WHEN** Auto-generate is enabled, the project restores or Learn settings change without a newly accepted useful Stop
- **THEN** no automatic generation starts and existing swells remain unchanged

#### Scenario: Stop request awaits final analysis
- **WHEN** a useful accepted Stop admits an automatic request and its pinned identities change or Auto-generate is disabled before commit
- **THEN** that request is cancelled without generating against a later snapshot or changing the accepted mode

### Requirement: Generation authority cannot outlive its workflow
Explicit Generate and eligible Stop generation SHALL share readiness, replacement and cancellation guards. Only an idle Learn request with matching final source/settings/timing/IR authority can publish. Capture restart, incompatible mode changes, restore, cancellation or changed dependencies SHALL invalidate pending publication and prevent partial definitions or stale Perform entry.

#### Scenario: Restart or switch while generation prepares
- **WHEN** a valid mode change, capture restart or restore supersedes a held generation request
- **THEN** no prepared definitions publish and no stale completion changes the accepted mode

#### Scenario: Manual navigation without generation
- **WHEN** an idle Learn user selects Perform or Export while candidate analysis is pending
- **THEN** existing readiness rules govern navigation, pending generation is invalidated and no automatic generation is implied

#### Scenario: Duplicate Stop or unsuccessful generation
- **WHEN** Stop repeats or generation is empty, protected-only, unchanged, failed or cancelled
- **THEN** no duplicate transaction commits, prior swells remain and the generation path stays Learn with its honest outcome
