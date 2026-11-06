# Spec Delta

## MODIFIED Requirements

### Requirement: Reopen validates cached renders and rebuilds only when possible
Restoration SHALL verify source/IR/render identities, bounds, engine version, sample rate, and accepted sample/timing-evidence compatibility before declaring a swell ready. A missing or invalid render with valid compatible source/IR SHALL be rebuilt in the background. Missing source/IR or incompatible timing SHALL mark only affected swells unavailable or stale and explain recovery; no replacement audio SHALL be silently accepted.

#### Scenario: Render cache is missing
- **WHEN** the saved render asset is absent but its source and managed IR are valid at compatible timing
- **THEN** the swell is visibly restoring/rendering and becomes ready only after complete verified reconstruction, without user recapture or automatic audition

#### Scenario: Managed IR or source is missing
- **WHEN** a referenced managed IR or source chunk cannot be verified on reopen
- **THEN** affected swells show the missing dependency, unaffected swells remain accessible, and unavailable audio is not presented as silence or a current preview

#### Scenario: Different host timing
- **WHEN** the host reopens at a different sample rate, unsupported meter or demonstrated timing-alignment conflict
- **THEN** saved sample positions and original capture rate are retained, affected swells are not auditionable as current, and no implicit stretching or reinterpretation occurs

#### Scenario: Reopen at a different BPM in the accepted map
- **WHEN** a saved tempo-map project reopens with the playhead at another BPM in the same accepted map and compatible rate/meter
- **THEN** verified source and render references retain their explicit sample alignment after fresh runtime admission, with musical conversion limited to verified coverage
- **AND** unchanged wet does not require recapture or regeneration merely because the current BPM differs

#### Scenario: Missing or malformed map evidence
- **WHEN** a supported fixed-tempo project lacks new evidence, or new-format timing evidence is missing/corrupt or unsupported
- **THEN** legacy state preserves its original fixed mapping without invented ramp history; new evidence failures are identified and restore cannot publish a partial or falsely verified state
- **AND** old asset bytes, swell identities and prior accepted state are preserved without autoplay

#### Scenario: Mixed authorities and persisted conflict
- **WHEN** a saved swell spans legacy/new timing authorities and an accepted revision has a conflict fence
- **THEN** restore preserves both authorities and join provenance; affected contribution admission remains fenced through later seeks until explicit complete recovery
- **AND** missing join evidence cannot be replaced by the host's current BPM or by relabelling old assets
