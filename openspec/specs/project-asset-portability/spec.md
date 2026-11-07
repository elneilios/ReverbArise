# project-asset-portability Specification

## Purpose

Allow accepted ReverbArise sessions and their exact audio dependencies to travel between Windows profiles without sacrificing legacy project recovery or independent instance ownership.

## Requirements

### Requirement: Asset folder association is explicit and host independent
The system SHALL support a user-selected asset folder without requiring the host project path. Ordinary association SHALL preserve existing identities and legacy references until a verified copy and explicit switch succeed. The explicitly approved incomplete recovery workspace defined below is a distinct operation, not verified migration. Relative references SHALL resolve against the selected store rather than an inferred host working directory.

#### Scenario: Host provides no project path
- **WHEN** the user associates a folder without available host project-location metadata
- **THEN** the folder can be used without changing capture or workflow semantics, and its actual path and backup responsibility are shown

#### Scenario: Association fails
- **WHEN** copying or verifying the destination fails or the user cancels
- **THEN** the previous folder and committed references remain authoritative and original files remain intact

### Requirement: Complete packs contain accepted dependency closure
A complete pack SHALL include accepted source coverage, session metadata, default/assigned/context IRs, completed wet renders, calibration, provenance and every saved Export history dependency. Missing dependencies or unknown dependency-bearing versions SHALL prevent a complete-pack claim. Pending work SHALL be excluded explicitly.

#### Scenario: Default IR without swells
- **WHEN** a session with an imported default IR and no swell definitions is packed
- **THEN** that managed IR and accepted source are included without requiring the original external IR

#### Scenario: Accepted history and in-flight work
- **WHEN** packing occurs during capture, import or rendering
- **THEN** one coherent accepted snapshot is pinned, pending work is identified as excluded and no provisional pass or partial render is claimed as saved

#### Scenario: Frozen and disabled swells
- **WHEN** the selected accepted state contains frozen, disabled or stale retained swell references
- **THEN** their exact referenced dependencies and saved statuses are preserved without automatic substitution or regeneration

#### Scenario: Historical context IR differs from current defaults
- **WHEN** an accepted schema-2 context pins an IR absent from the current Default IR and swell assignments, including an automatic-OFF or zero-swell session
- **THEN** the complete pack includes that IR and exact context ownership without replaying history as a generation command or inventing an IR for an empty reference

#### Scenario: Previous Export revision remains in saved history
- **WHEN** saved Export history contains non-current or legacy gain-law revisions
- **THEN** every referenced WAV and its checksum, origin, frame count and gain metadata are preserved and verified, and any missing required history file prevents a complete-pack claim

#### Scenario: Legacy contextless capture remains contextless
- **WHEN** a supported schema-1 manifest is packed
- **THEN** its exact source and unknown historical context are retained without silently migrating provenance or changing existing swell assignments

### Requirement: Version adapters preserve accepted metadata
Adapters SHALL distinguish processor, creative, capture and Export versions. They SHALL preserve context ownership/origins, generation fingerprints, auto-generation policy and Export checksums/timing/Dry/Wet/gain laws. Schema-1 historical context SHALL remain unknown.

#### Scenario: Mixed supported version domains
- **WHEN** an envelope-v8 session contains creative-v11 state, schema-2 capture and Export-v2 or legacy history
- **THEN** adapters preserve each domain independently, retain explicit empty references and reject unknown dependency-bearing versions without changing accepted state

### Requirement: Pack scope identifies all participating instances
The system SHALL identify the sessions and instance snapshots included in a pack. It SHALL NOT claim project-wide or historical FLP completeness from a single instance or from capture manifests alone. Packing SHALL NOT authorize source-store deletion or modification of the FLP. Receipts SHALL distinguish included managed Export history from unenumerated external Playlist files; packing SHALL NOT claim to rewrite or relink external DAW file references.

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
- **THEN** accepted sample/channel data, coverage, edits, timing, IR choices and wet results match without resampling, render regeneration, historical-context execution or auto-generation on restore/relink

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
Packing SHALL publish complete verified output before reporting success or offering pack-driven association. Incomplete recovery workspace creation SHALL NOT be presented as successful packing or a complete backup. Interrupted, cancelled, disk-full, locked, read-only or offline operations SHALL preserve originals and the previous successful snapshot. Incomplete destinations SHALL be identifiable and recoverable without being presented as complete.

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
New readers SHALL preserve supported legacy state, paths, checksums and immutable snapshots through nondestructive adapters. Exact-byte reuse SHALL preserve exact float32 samples and distinct dependency identity. Production storage SHALL use WAV. Unknown formats SHALL fail transactionally. Older-binary limitations and the need for pre-upgrade FLPs and stores SHALL be explicit.

#### Scenario: Older saved project after association
- **WHEN** a legacy FLP saved before folder association is reopened
- **THEN** its original committed dependencies remain readable without adopting a newer snapshot or requiring destructive migration

#### Scenario: Exact samples despite normalized content identities
- **WHEN** assets differ in float bit patterns such as signed zero or contain valid above-unity/subnormal samples
- **THEN** exact-byte reuse preserves the distinct data even if a normalized musical content identity agrees

#### Scenario: Binary rollback
- **WHEN** the user returns to an older binary that cannot read the new state version
- **THEN** documented recovery uses the retained pre-upgrade FLP and original store without claiming the upgraded FLP is readable

### Requirement: Asset administration preserves real-time behavior
Inventory, hashing, copying, compression and maintenance SHALL run outside the audio callback and preserve existing capture/workflow/DSP contracts. Resource failures SHALL remain explicit and SHALL NOT force capture, recapture or loss of unaffected output.

#### Scenario: Large pack during playback
- **WHEN** asset administration encounters memory or storage limits during playback
- **THEN** it fails or pauses explicitly without callback I/O, blocking or unbounded processing and without mutating accepted source

### Requirement: Host discovery has a safe user-data fallback
The system SHALL use project-location metadata only after its host-specific semantics are validated. Missing, ambiguous or stale metadata SHALL retain the user-data fallback. Storage SHALL expose the actual working folder and explicit selection. Discovery SHALL NOT silently relocate existing sessions or treat a preset path as a project path.

#### Scenario: Unsaved or unsupported host
- **WHEN** a fresh session has no verified project location
- **THEN** learning remains available with user-data storage and a visible folder choice

#### Scenario: Save As changes location
- **WHEN** the host reports a changed project path for an existing session
- **THEN** the plugin preserves the original association until an explicitly approved verified transfer succeeds

### Requirement: Storage provides verified backup and restore
Storage SHALL offer working-folder selection, Backup and Restore actions. Backup SHALL use complete accepted dependency closure and identify included instances. Restore or Locate SHALL verify immutable identities and bytes before explicit association. These actions SHALL preserve originals and disclose that the DAW project and external Playlist files require separate backup.

#### Scenario: User-data assets need portability
- **WHEN** the user backs up a session using the user-data folder
- **THEN** the verified self-contained backup can travel with the separately saved DAW project and Restore can resolve it without the original user profile

#### Scenario: Project folder moves
- **WHEN** the DAW project and complete associated asset folder are copied together
- **THEN** managed relative references remain usable after validated resolution or explicit Locate, without requiring the original absolute path

#### Scenario: Restore is incomplete or cancelled
- **WHEN** required dependencies fail verification or Restore is cancelled
- **THEN** existing metadata, surviving assets and the prior association remain intact and no complete recovery is claimed

### Requirement: Explicit recovery workspace preserves unresolved history
Recovery SHALL require explicit choice of a writable workspace when the original folder or required assets cannot be recovered. It SHALL preserve decodable swell metadata, original locators and verified surviving dependencies, and report missing assets or unknown source history. It SHALL NOT silently change the root or overwrite originals. Existing independent-writer and host-save notification rules SHALL apply.

#### Scenario: Whole associated folder is irrecoverable
- **WHEN** the user explicitly creates and adopts a new recovery workspace after the original folder cannot be located or restored
- **THEN** the original locator and swell metadata remain available, missing requirements remain explicit, and normal Learn can capture fresh source in the writable workspace without a complete-copy/backup claim

#### Scenario: Recovery workspace creation fails or is cancelled
- **WHEN** selection, metadata validation, survivor verification, publication or final adoption fails or is cancelled
- **THEN** the current association, decodable arrangement and all originals remain authoritative, no partial workspace is adopted automatically and no full recovery is claimed

### Requirement: Recovery workspace is not complete-pack certification
Creating an incomplete recovery workspace SHALL NOT publish a complete-pack receipt or claim a complete copy/backup. Missing required dependencies or unknown required source coverage SHALL retain explicit incomplete status and block complete-pack verification; recovery SHALL NOT invent assets or omit declared history to pass that gate.

#### Scenario: Incomplete recovery is not a complete backup
- **WHEN** a recovery workspace still has missing required dependencies or unknown required source coverage
- **THEN** its incomplete status remains explicit and complete-pack verification still fails rather than inventing assets or omitting declared history
