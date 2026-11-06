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
Realtime observation/admission SHALL use bounded storage and work without allocation, deallocation, locks, waits or file access. Required evidence loss, queue overflow, capacity exhaustion or evidence-write failure SHALL abort the pending source/evidence transaction atomically. Host-reported missing musical fields SHALL remain valid unavailable-field records, not capacity loss. Limits SHALL disclose representative recording durations and preserve prior accepted state.

#### Scenario: Dense automation and queue pressure
- **WHEN** long sparse capture with rapidly changing tempo exceeds evidence or publication capacity
- **THEN** callback work remains bounded, dry routing continues and the whole pending transaction fails explicitly with prior accepted state intact; no capacity-induced precision downgrade accepts new source
- **AND** editor visibility does not determine whether timing observations are collected

### Requirement: Fresh fixed-tempo capture retains affine editing authority
Fresh constant-tempo captures SHALL retain existing affine Snap, ruler and musical-entry behavior under the unchanged fixed-tempo consistency policy, including stable runtime offsets. Evidence storage SHALL NOT alone restrict them to recorded anchors. Demonstrated variation SHALL remove only unsupported authority; arbitrary ramp interiors SHALL NOT inherit an affine guarantee.

#### Scenario: Fresh capture between callback anchors
- **WHEN** a new fixed-tempo capture completes and a user edits at a valid musical point between recorded callback starts
- **THEN** Snap, bar/range entry and relative edits retain their existing integer-sample conversions before and during an admitted offset run
- **AND** any proposed reduction of that behavior requires explicit owner review

### Requirement: Runtime comparison tolerates partition differences without inferred curves
Each runtime SHALL distinguish comparable, unverified and conflicting mapping. Different block boundaries SHALL compare through valid local authority, never invented ramp interpolation. A stable observation offset SHALL be calibrated independently per map run using two compatible affine observations and the existing one-sample residual; it SHALL affect comparison coordinates only, never PCM placement. An uncalibrated ramp offset SHALL remain unknown.

#### Scenario: Different bounce partitions after affine calibration
- **WHEN** capture, playback and bounce start callbacks at different samples and a run has independently calibrated a stable sample-domain offset
- **THEN** comparison queries accepted authority at the offset-adjusted observation sample, audio still uses its unchanged absolute sample position and only supported comparable queries can prove conflict
- **AND** unsupported ramp interiors remain unverified, not fitted to force admission or rejection

#### Scenario: Run begins inside a sparsely observed ramp
- **WHEN** no affine calibration or exact comparable anchor exists at the runtime observation
- **THEN** the mapping is unverified, compatible sample-positioned audio without a known conflict fence may play, and unsupported musical edits stay unavailable
- **AND** a mismatch alone cannot be relabelled a proven edit or used to create a corrective offset

#### Scenario: Matched samples lack a justified uncertainty bound
- **WHEN** different partitions observe the same ramp sample with different PPQ and no justified host-coordinate error bound
- **THEN** that comparison remains unverified rather than using bit equality or a newly invented tolerance to declare conflict
- **AND** feedback does not claim detection of every later map edit in those regions

### Requirement: Conflict fences survive transport and require explicit recovery
Demonstrated conflicts SHALL fence affected source/authority dependencies and complete positioned wet contributions, including lead-in and tails. Fences SHALL persist by instance/accepted revision across Stop, seeks, bounce and save/reopen. Unknown conflict bounds SHALL extend conservatively until verified convergence. Clearing SHALL require explicit fully validated recovery, preserving originals; failed recovery or persistence SHALL NOT silently admit affected audio.

#### Scenario: Upstream tempo edit and seek into wet lead-in
- **WHEN** an upstream edit demonstrably shifts accepted later timing without a verified convergence boundary, and the host seeks into an affected swell's earlier wet lead-in
- **THEN** the dependent contribution remains wholly suppressed under its persistent fence, the original audio/edits are retained and independently unaffected neighbors remain usable
- **AND** a new run, sparse gap, acknowledgment or new bounce does not clear the fence

#### Scenario: Save and recover a conflicted project
- **WHEN** a conflicted project is saved/reopened or explicit Recapture/regeneration or restoration of the old DAW map is attempted
- **THEN** fences restore without autoplay and clear only after validating the whole affected dependency; partial or cancelled recovery preserves them
- **AND** conflict-storage failure reports an admission latch and prevents falsely clean saved state

### Requirement: Mixed authorities preserve their join contract
A swell spanning legacy affine and new observation-based source SHALL validate both authorities and compatible join provenance without implicitly upgrading either. Compatible rate, constant meter, verified join and sample dependencies SHALL permit sample-defined audio across the join. Missing or contradictory join evidence SHALL block the spanning contribution and affected musical operations while preserving independently valid neighbors and original references.

#### Scenario: Mixed-authority swell spans a tempo ramp
- **WHEN** explicit source bounds begin in legacy affine source and end in newly captured ramp source with a verified compatible join
- **THEN** sample-based generation/playback uses both accepted spans, musical editing requires supported endpoints/join and no exact ramp interior is fabricated
- **AND** save/reopen retains both authority kinds and the original asset identities

#### Scenario: Mixed authority join is unavailable or conflicting
- **WHEN** the join cannot be verified or a dependency is fenced, including after restore or a seek into that swell
- **THEN** the complete spanning contribution is unavailable with the join/conflict reason, no implicit retiming occurs and wholly valid neighbors remain eligible
