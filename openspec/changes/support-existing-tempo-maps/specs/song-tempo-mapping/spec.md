# Spec Delta

## Purpose

Preserve accepted relationships between absolute source samples and observed
musical time, with explicit coverage and precision for existing tempo automation.

## ADDED Requirements

### Requirement: Capture retains sparse timing evidence with source
Capture SHALL admit original audio at valid absolute samples across pre-existing tempo steps and ramps at a constant supported rate and meter. Accepted source SHALL retain available sample, PPQ, BPM, meter, block extent and pass provenance. Seeks, loops and missing musical fields SHALL produce explicit evidence boundaries or gaps, never a fabricated global map or shifted PCM.

#### Scenario: Step and ramp inside a pass
- **WHEN** valid sample-positioned capture crosses an existing tempo step or continuous ramp
- **THEN** ordinary instantaneous BPM changes do not abort the pass and admitted PCM retains its absolute samples with truthful timing observations
- **AND** the musical evidence does not claim knowledge of unreported changes inside a processing block

#### Scenario: Sparse loops and missing musical information
- **WHEN** capture seeks between separated regions, loops over accepted source or temporarily lacks PPQ/BPM while absolute sample timing remains usable
- **THEN** audio coverage remains sparse and protected, timing provenance records the discontinuity or unavailable fields and no line is inferred across an unobserved gap
- **AND** invalid sample coordinates, unsupported rate/meter changes and source admission failures retain existing transactional safeguards

### Requirement: Musical conversion declares coverage and precision
Musical conversion SHALL return a verified coordinate with its sample-error bound or an explicit unavailable/conflicting result. Exact observations SHALL NOT imply exact ramp interiors. Affine conversion SHALL apply only within its established constant-tempo authority; unsupported interpolation, extrapolation and ambiguous inverse mappings SHALL NOT authorize musical edits.

#### Scenario: Unobserved ramp interior
- **WHEN** two observations bracket a tempo ramp but its interior timing is not established to integer-sample precision
- **THEN** Snap and exact beat/bar conversion in that interior are unavailable rather than inferred by integrating a single BPM or assuming a linear ramp
- **AND** exact sample/time edits remain available without changing the accepted evidence

#### Scenario: Verified local conversion and inverse lookup
- **WHEN** a requested beat boundary is covered by sufficient validated local evidence
- **THEN** conversion uses that evidence with checked arithmetic and declared precision, including inverse ambiguity and deterministic integer-sample rounding
- **AND** a result whose error cannot meet the musical operation's integer-sample contract is rejected with its actual reason

### Requirement: Evidence publication is atomic with source ownership
Accepted timing evidence SHALL be immutable, versioned and bound to the same source revision and interval ownership as committed audio/silence. Additive capture SHALL preserve protected evidence; explicit Recapture SHALL replace only encountered dependencies. Failed, cancelled, superseded or incomplete transactions SHALL NOT publish mismatched source and timing or modify historical snapshots.

#### Scenario: Recapture and old snapshots
- **WHEN** explicit Recapture accepts new source and timing for one interval
- **THEN** the new interval and its evidence publish together, unrelated intervals stay unchanged and prior saved projects retain readable old references
- **AND** no metadata-only change implicitly moves samples, substitutes performances or rewrites render assets

#### Scenario: Timing metadata cannot commit
- **WHEN** evidence storage, validation or publication fails during settlement
- **THEN** no partially accepted source/evidence pair becomes current, prior accepted state remains usable and failure is reported

### Requirement: Absolute audio validity is separate from grid validity
Verified source and wet assets SHALL retain sample-based validity independently of missing musical coverage. Ordinary BPM evolution within accepted evidence SHALL NOT globally stale renders or silence safely aligned audio. Rate, meter, source identity, asset corruption and demonstrated alignment conflicts SHALL remain separate, explicit admission failures scoped to affected material.

#### Scenario: Audio spans uncertain musical conversion
- **WHEN** a verified swell spans a tempo change whose exact musical interior is unavailable while absolute host samples remain compatible
- **THEN** its accepted wet plays at stored samples with its existing gain/shape and the grid limitation is reported separately
- **AND** missing musical information alone does not trigger convolution or source replacement

### Requirement: Later map conflicts preserve original material
Later timing observations SHALL be compared only where evidence supports a meaningful comparison under validated run provenance. A demonstrated alignment conflict SHALL mark affected scheduling or musical operations unavailable with a reason while preserving original source, edits and renders. Unobserved regions SHALL remain unverified rather than being certified unchanged or declared edited.

#### Scenario: Known boundary changes after capture
- **WHEN** later host evidence demonstrably contradicts an accepted musical boundary after applicable run alignment has been validated
- **THEN** affected state reports the conflict and requires explicit owner recovery; it is not silently retimed, recaptured or admitted by resetting a baseline
- **AND** unaffected verified material and immutable historical assets remain available

#### Scenario: Seek into an unobserved map region
- **WHEN** the host visits a region without sufficient musical comparison evidence
- **THEN** diagnostics identifies unverified mapping without claiming a complete map revision match
- **AND** safely sample-aligned audio is not silenced solely because grid verification is unavailable

### Requirement: Versioned evidence restores without changing identities
Saved state SHALL retain validated evidence references, coverage, precision, provenance and accepted revision without embedding PCM. Supported fixed-tempo states SHALL retain their original affine authority without inventing ramp history. Unknown versions, unsafe references and malformed evidence SHALL fail transactionally. Restore SHALL retire transient runs and preserve plugin, swell, source and asset identities.

#### Scenario: Fixed-tempo legacy project
- **WHEN** a supported older project without tempo-map metadata reopens
- **THEN** its existing samples, assets, fixed grid, edits and preferences retain their prior meaning without forced recapture or automatic playback

#### Scenario: Map project or malformed state
- **WHEN** a valid tempo-map project reopens, or evidence with unsupported version, nonfinite values, invalid ranges or unsafe references is supplied
- **THEN** valid state restores coherently and awaits fresh runtime admission; invalid state cannot replace valid accepted state or delete assets

### Requirement: Timing collection and admission remain bounded
Realtime timing observation and playback admission SHALL use bounded work and storage without allocation, deallocation, locks, waits or file access. Worker-side indexing and persistence SHALL expose finite resource limits. Evidence loss or capacity exhaustion SHALL explicitly reduce precision or reject the transaction; neither SHALL invent coverage or silently change source ownership.

#### Scenario: Dense automation and queue pressure
- **WHEN** long sparse capture with rapidly changing tempo exceeds evidence or publication capacity
- **THEN** callback work remains bounded, dry routing continues and the resulting unavailable evidence or failed pass is disclosed with accepted state intact
- **AND** editor visibility does not determine whether timing observations are collected
