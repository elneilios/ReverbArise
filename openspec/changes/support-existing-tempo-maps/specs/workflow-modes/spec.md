# Spec Delta

## MODIFIED Requirements

### Requirement: Mode entry prerequisites reflect verified material
Live entry SHALL require useful verified compatible source or an eligible verified frozen wet render at compatible timing. Finalise entry SHALL require an enabled, unmuted, solo-eligible verified exportable wet render compatible with the accepted sample rate, constant meter and sample-alignment evidence, independently of dry availability or mixdown readiness. Mapped silence alone SHALL NOT satisfy usefulness. Pending wet renders SHALL NOT prevent source-ready Live entry. Frozen recovery SHALL NOT fabricate dry or permit replacement.

#### Scenario: Manual workflow after dry capture
- **WHEN** useful source has committed with Automatic Swells off and no prepared swell
- **THEN** Live is available for manual editing and Finalise remains unavailable

#### Scenario: Silence or failed verification
- **WHEN** coverage is only silence, unavailable, incompatible or still validating
- **THEN** neither mode becomes available by treating that coverage as useful compatible source; an independently verified eligible wet render retains its explicit recovery eligibility
- **AND** valid stored material is retained without reinterpretation

#### Scenario: Finalise readiness
- **WHEN** useful compatible source and an enabled ready swell exist
- **THEN** Finalise can be selected without requiring an immediate export

#### Scenario: Ready render has incompatible timing
- **WHEN** useful source matches the host but a ready render has an incompatible sample rate, unsupported meter or demonstrated alignment conflict
- **THEN** that render cannot qualify for Finalise or frozen-wet recovery; eligibility does not substitute for the later export workspace's full asset/output preparation

#### Scenario: Frozen wet recovery without source
- **WHEN** captured dry is missing/cleared but an enabled unmuted solo-eligible frozen render verifies at compatible timing
- **THEN** Live entry and wet editing/playback are available without recreating source
- **AND** unfrozen source-dependent swells remain unavailable, captured-dry audition still requires real dry, and no Recapture permission is granted

#### Scenario: Export with independently recoverable wet
- **WHEN** dry source is missing but an eligible frozen wet render verifies at compatible timing
- **THEN** Finalise entry is available and wet-only preparation requires no invented dry; dry-inclusive mixes use the explicitly warned-silence policy
- **AND** source-dependent unavailable renders do not become exportable merely because the mode changed

#### Scenario: Entry and current file readiness differ
- **WHEN** a verified wet render permits Finalise entry and the user selects 0% Wet or a new mix is still preparing
- **THEN** Finalise remains selected, its current Save/Drag/preview actions reflect actual mixdown readiness and dry-only source alone still cannot unlock initial entry

#### Scenario: Accepted automation with an unavailable local grid
- **WHEN** useful verified source or eligible frozen wet exists at compatible absolute samples and the current BPM differs within accepted tempo automation
- **THEN** Perform/Export prerequisites do not fail solely on that BPM difference or incomplete musical conversion
- **AND** musical actions, prepared audio readiness, dry availability and current file readiness retain independent truthful guards

### Requirement: Optional automatic generation follows useful Stop only
Auto-generate on Stop SHALL admit generation only after successfully accepted useful Stop with effective Automatic Swells enabled, waiting for final matching analysis. Toggling ON, restoring state, editing settings or entering Learn SHALL NOT trigger generation. OFF SHALL cancel pending automatic intent before commit. Changed settings/source/default/accepted timing-evidence authority or workflow authority SHALL invalidate the request without retargeting it to later work.

#### Scenario: Preference changes without capture
- **WHEN** Auto-generate is enabled, the project restores or Learn settings change without a newly accepted useful Stop
- **THEN** no automatic generation starts and existing swells remain unchanged

#### Scenario: Stop request awaits final analysis
- **WHEN** a useful accepted Stop admits an automatic request and its pinned identities change or Auto-generate is disabled before commit
- **THEN** that request is cancelled without generating against a later snapshot or changing the accepted mode

### Requirement: Generation authority cannot outlive its workflow
Explicit Generate and eligible Stop generation SHALL share readiness, replacement and cancellation guards. Only an idle Learn request with matching final source/settings/accepted timing-evidence/IR authority can publish. Capture restart, incompatible mode changes, restore, cancellation or changed dependencies SHALL invalidate pending publication and prevent partial definitions or stale Perform entry.

#### Scenario: Restart or switch while generation prepares
- **WHEN** a valid mode change, capture restart or restore supersedes a held generation request
- **THEN** no prepared definitions publish and no stale completion changes the accepted mode

#### Scenario: Manual navigation without generation
- **WHEN** an idle Learn user selects Perform or Export while candidate analysis is pending
- **THEN** existing readiness rules govern navigation, pending generation is invalidated and no automatic generation is implied

#### Scenario: Duplicate Stop or unsuccessful generation
- **WHEN** Stop repeats or generation is empty, protected-only, unchanged, failed or cancelled
- **THEN** no duplicate transaction commits, prior swells remain and the generation path stays Learn with its honest outcome
