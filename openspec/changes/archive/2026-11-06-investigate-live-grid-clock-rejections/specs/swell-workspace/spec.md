# Spec Delta

## ADDED Requirements

### Requirement: Fixed-grid diagnostics explain the actual availability decision
Diagnostics SHALL identify the actual fixed-grid availability result, rejected timing dimension, accepted mapping, observed mapping, capture rate and sample-position availability. Valid origins and their delta SHALL be shown with enough precision to assess the existing one-sample allowance. Missing inputs SHALL remain unavailable. Grid evidence SHALL be labelled independently from live callback admission and SHALL NOT change Snap, editing or playback policy.

#### Scenario: Playback offset disables the accepted source grid
- **GIVEN** accepted timing at sample 3399344, PPQ 144, 48000 Hz, 122 BPM and 4/4
- **WHEN** grid evaluation uses host sample 3399344 and PPQ 143.999322222222219 with the same rate, tempo and meter
- **THEN** diagnostics identifies the approximately 16-sample accepted-to-host origin difference as a grid-origin rejection under the existing policy
- **AND** it does not infer live rejection from that grid result or claim the existing offline fix repairs Snap

#### Scenario: Stop restores grid compatibility
- **WHEN** stopped host timing again matches the accepted source mapping after a grid rejection
- **THEN** current grid evidence reflects that compatibility and labels its evaluation context
- **AND** restored grid availability is not presented as proof of preceding wet audibility or successful live preparation

#### Scenario: Other grid prerequisites fail
- **WHEN** sample position is unavailable, timing is invalid or nonfinite, capture rate is incompatible, tempo or meter differs, or grid conversion cannot be represented
- **THEN** diagnostics identifies the failing prerequisite without inventing an origin or reducing every failure to an origin mismatch
- **AND** the existing Snap On rejection, Ctrl bypass, Snap Off, exact-entry and stale-gesture protections remain unchanged

#### Scenario: UI and callback observations have different provenance
- **WHEN** a grid decision and a live callback or worker observation come from different snapshots
- **THEN** diagnostics labels those contexts separately and exposes the exact inputs used for the grid evaluation without claiming callback-level simultaneity
- **AND** unavailable evidence cannot be substituted with a later stopped observation

#### Scenario: Read diagnostic detail without editing
- **WHEN** grid evidence is refreshed, inspected or the editor is reopened
- **THEN** it uses the existing Diagnostics presentation and accessible full text without changing saved navigation, Snap preference, source/render identities, job counts or host transport
- **AND** the main workspace retains its existing concise unavailable message and supported editing alternatives
