# Spec Delta

## Purpose

Allow accepted ReverbArise sessions and their exact audio dependencies to travel between Windows profiles without sacrificing legacy project recovery or independent instance ownership.

## ADDED Requirements

### Requirement: Asset folder association is explicit and host independent
The system SHALL support a user-selected asset folder without requiring the host project path. Association SHALL preserve existing identities and legacy references until a verified copy and explicit switch succeed. Relative references SHALL resolve against the selected store rather than an inferred host working directory.

#### Scenario: Host provides no project path
- **WHEN** the user associates a folder without available host project-location metadata
- **THEN** the folder can be used without changing capture or workflow semantics, and its actual path and backup responsibility are shown

#### Scenario: Association fails
- **WHEN** copying or verifying the destination fails or the user cancels
- **THEN** the previous folder and committed references remain authoritative and original files remain intact

### Requirement: Complete packs contain accepted dependency closure
A complete pack SHALL include the selected accepted source coverage, session metadata, default and assigned IRs, exact completed wet render dependencies, calibration and supported accepted provenance. Pending work SHALL be excluded explicitly. A missing dependency or unknown dependency-bearing version SHALL prevent a complete-pack claim.

#### Scenario: Default IR without swells
- **WHEN** a session with an imported default IR and no swell definitions is packed
- **THEN** that managed IR and accepted source are included without requiring the original external IR

#### Scenario: Accepted history and in-flight work
- **WHEN** packing occurs during capture, import or rendering
- **THEN** one coherent accepted snapshot is pinned, pending work is identified as excluded and no provisional pass or partial render is claimed as saved

#### Scenario: Frozen and disabled swells
- **WHEN** the selected accepted state contains frozen, disabled or stale retained swell references
- **THEN** their exact referenced dependencies and saved statuses are preserved without automatic substitution or regeneration

### Requirement: Pack scope identifies all participating instances
The system SHALL identify the sessions and instance snapshots included in a pack. It SHALL NOT claim project-wide or historical FLP completeness from a single instance or from capture manifests alone. Packing SHALL NOT authorize source-store deletion or modification of the FLP.

#### Scenario: Another instance is unavailable
- **WHEN** one instance is packed while another project's instance state has not been collected
- **THEN** the receipt identifies the included instance and excludes any unsupported claim that the whole FLP is portable

#### Scenario: Older FLP backups are not supplied
- **WHEN** the user packs the current accepted session without older host snapshots
- **THEN** the result states its snapshot scope and does not claim to preserve unknown historical IR/render references

### Requirement: Verified packs reopen independently of the source profile
A complete compatible pack SHALL restore exact source positions, sparse silence/gaps, definitions, managed IRs and completed wet results using destination assets alone after explicit relink when necessary. It SHALL require neither recapture, resampling nor convolution to recover included valid results. Existing timing and engine readiness constraints SHALL remain enforced.

#### Scenario: Clean Windows profile
- **WHEN** the FLP and complete compatible pack are transferred to a second profile with original caches and external IRs unavailable, then the folder is relinked
- **THEN** accepted sample/channel data, coverage, edits, timing, IR choices and wet results match without resampling or render regeneration

#### Scenario: Incompatible timing or engine
- **WHEN** the receiving environment is incompatible with saved timing or render requirements
- **THEN** the incompatibility is reported, exact assets remain intact and no changed audio is silently substituted

### Requirement: Relink verifies immutable snapshot identity
Relink SHALL validate store, snapshot, descriptor and content identity before accepting a folder. Moving a folder SHALL preserve relative references. Multiple conflicting candidates or missing assets SHALL remain explicit rather than selecting a newer head or accepting a matching filename alone.

#### Scenario: Moved folder
- **WHEN** a user selects the moved complete folder with matching immutable snapshot and bytes
- **THEN** resolution succeeds without rewriting audio or depending on the old absolute path

#### Scenario: Conflicting store copies
- **WHEN** two candidate folders share a store ID but differ in required snapshot content
- **THEN** neither is silently substituted and a validated explicit selection is required

### Requirement: Copies retain history and isolate future publication
Restored copies and duplicated instances SHALL share readable immutable history while independently owning future writes. Save As SHALL NOT silently reassign durable IDs or move assets. Explicit physical forks SHALL copy and verify dependencies before switching association.

#### Scenario: Simultaneous cloned writers
- **WHEN** two instances in separate host processes load the same snapshot and independently capture, clear or change IR
- **THEN** neither can publish into the other's accepted session, overwrite prior assets or invalidate the other's saved snapshot

#### Scenario: Save As with no reliable host notification
- **WHEN** a host saves or copies a project without a distinguishable plugin notification
- **THEN** existing immutable references remain readable and future publication remains independently owned without requiring filename-based inference

### Requirement: Pack publication is cancellable and recoverable
Packing SHALL publish complete verified output before reporting success or offering association. Interrupted, cancelled, disk-full, locked, read-only or offline operations SHALL preserve originals and the previous successful snapshot. Incomplete destinations SHALL be identifiable and recoverable without being presented as complete.

#### Scenario: Interruption during publication
- **WHEN** a process stops between audio, descriptor and snapshot publication
- **THEN** no partial pack is reported complete, prior success remains readable and restart can report or verify resumable owned staging

#### Scenario: Host save after successful pack
- **WHEN** a user explicitly switches to a verified folder
- **THEN** the system requests saving the FLP and does not claim that a host save has already happened

### Requirement: Portable references enforce path and resource bounds
The system SHALL reject escaping or ambiguous paths and malformed or oversized pack metadata before accepting dependencies. Validation SHALL prevent path replacement from escaping the managed root. Supported Unicode and long-path behavior SHALL be documented with specific errors where unsupported.

#### Scenario: Malicious or ambiguous entries
- **WHEN** a pack contains traversal, absolute/device paths, alternate streams, case collisions, escaping reparse points or replacement races
- **THEN** acceptance fails without overwriting or deleting files outside the owned operation

### Requirement: Legacy and exact-sample compatibility remain recoverable
New readers SHALL preserve supported legacy state, paths, checksums and immutable snapshots through nondestructive adapters. Storage deduplication or compression SHALL preserve exact float32 samples and dependency identity. Unknown formats SHALL fail transactionally. Older-binary limitations and the need for pre-upgrade FLPs and stores SHALL be explicit.

#### Scenario: Older saved project after association
- **WHEN** a legacy FLP saved before folder association is reopened
- **THEN** its original committed dependencies remain readable without adopting a newer snapshot or requiring destructive migration

#### Scenario: Exact samples despite normalized content identities
- **WHEN** assets differ in float bit patterns such as signed zero or contain valid above-unity/subnormal samples
- **THEN** deduplication and optional encoding cannot silently change those samples even if a normalized musical content identity agrees

#### Scenario: Binary rollback
- **WHEN** the user returns to an older binary that cannot read the new state version
- **THEN** documented recovery uses the retained pre-upgrade FLP and original store without claiming the upgraded FLP is readable

### Requirement: Asset administration preserves real-time behavior
Inventory, hashing, copying, compression and maintenance SHALL run outside the audio callback and preserve existing capture/workflow/DSP contracts. Resource failures SHALL remain explicit and SHALL NOT force capture, recapture or loss of unaffected output.

#### Scenario: Large pack during playback
- **WHEN** asset administration encounters memory or storage limits during playback
- **THEN** it fails or pauses explicitly without callback I/O, blocking or unbounded processing and without mutating accepted source
