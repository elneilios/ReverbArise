# Spec Delta

## Purpose

Make managed audio storage inspectable and recoverable while protecting sole recordings, historical project dependencies and user exports from inferred or automatic deletion.

## ADDED Requirements

### Requirement: Storage inventory reports scope and uncertainty
The system SHALL provide read-only location, logical byte and file-count reporting by asset category and retained history for selected managed stores. Shared physical objects SHALL count once in physical totals. Unknown references, inaccessible files, scan time and unmeasured allocation SHALL remain explicit. Potential duplicates SHALL NOT be labelled reclaimable without eligibility proof.

#### Scenario: Mixed source and history
- **WHEN** a store contains captures, IRs, current and prior wet results, manifests, scratch and exports
- **THEN** reporting separates those categories, identifies exact inspected roots and distinguishes known dependencies, uncertainty and eligible scratch bytes

#### Scenario: Offline store
- **WHEN** part of the selected store or reference graph is offline or unreadable
- **THEN** reporting marks incomplete counts and blocks reclamation in that scope rather than reporting zero or assuming absence

### Requirement: Durable audio and old snapshots are protected by default
Captured source, imported IRs, immutable completed renders, snapshot dependencies and accepted provenance SHALL be retained indefinitely by default. Clear, missing current definitions, rebuildability, inactivity and RAM eviction SHALL NOT establish deletion eligibility. Unknown external FLP references SHALL be treated as protective uncertainty.

#### Scenario: Earlier FLP is offline
- **WHEN** a newer session no longer uses assets that an unenumerated older FLP could reference
- **THEN** those durable assets remain protected and ordinary cleanup cannot select them

#### Scenario: Rebuildable frozen render
- **WHEN** an old render appears reproducible from currently available source and IR
- **THEN** it remains protected because older snapshots, frozen audio and engine differences are not disproved

### Requirement: Reference accounting fails conservatively
Reference tracking SHALL cover observed accepted snapshots, prior registered snapshots, packed dependencies, accepted context, aliases, active jobs and readers, exports and recovery operations. Catalog absence or corruption SHALL NOT prove unreachability. Unsupported dependency metadata and missing roots SHALL block reclamation until resolved.

#### Scenario: Crash before index update
- **WHEN** immutable output was published but a catalog update was interrupted
- **THEN** rebuilding or uncertain classification preserves files rather than freeing them from a zero reference count

#### Scenario: New schema lacks dependency enumeration
- **WHEN** accepted metadata uses a dependency-bearing version the maintenance reader cannot enumerate
- **THEN** complete accounting and cleanup are refused for the affected scope without discarding metadata

### Requirement: Sole recordings and external exports cannot enter routine cleanup
Routine cleanup SHALL exclude captured recordings including abandoned capture staging, user-imported source and durable exports potentially referenced by the DAW. A missing current reference, file extension or managed-root location SHALL NOT transfer ownership of these files to a scratch collector.

#### Scenario: Abandoned capture pass
- **WHEN** an interrupted pass leaves source files not referenced by the latest committed map
- **THEN** recovery reports them and ordinary temporary cleanup excludes them as potentially irreplaceable recordings

#### Scenario: Playlist WAV has no plugin reference
- **WHEN** an exported WAV has no current plugin dependency and resides inside a selected asset folder
- **THEN** it is still treated as user-owned durable audio and excluded from cleanup

### Requirement: Scratch cleanup requires ownership proof and explicit review
Cleanup SHALL default to disabled and target only journaled plugin-owned non-recording transient files with terminal operation status, no committed or recovery dependency and no active writer, reader or job. Preview SHALL show exact files, counts, bytes, scopes and eligibility reasons. Unknown legacy temporaries SHALL be preserved. No file SHALL move without explicit review approval.

#### Scenario: Old temporary extension without a journal
- **WHEN** a legacy .tmp or .part file has no trustworthy operation ownership record
- **THEN** its age or extension cannot make it eligible and it is reported as unknown

#### Scenario: Pack-copy scratch is eligible
- **WHEN** a cancelled pack owns unreferenced staging copies with a verified terminal journal and no active leases
- **THEN** preview can offer those exact files without including original captures or completed durable assets

### Requirement: Cleanup revalidates concurrent ownership
Before every reviewed cleanup action the system SHALL revalidate file identity, path containment, reference and recovery edges, and cross-process leases under exclusive maintenance coordination. Conflicts, locked files, path races or uncertainty SHALL skip the candidate. Cleanup SHALL never block the audio callback or reclaim its in-use buffers.

#### Scenario: New reader after preview
- **WHEN** a candidate gains a job, reader or recovery dependency after preview
- **THEN** cleanup skips it and reports the changed eligibility rather than acting on stale approval

#### Scenario: Root is replaced by a link
- **WHEN** a candidate path changes to an escaping reparse target between preview and action
- **THEN** the action fails safely without moving or deleting the target

### Requirement: Removal uses reversible quarantine and separate final review
Approved scratch removal SHALL first use durable journaled quarantine with hash-verified undo and a minimum 30-day grace period. Grace expiry SHALL NOT authorize deletion. Permanent removal SHALL require a second explicit review and fresh eligibility checks. Interrupted moves SHALL preserve recoverable source or quarantine data and report reconciliation needs.

#### Scenario: Undo after restart
- **WHEN** the user requests undo during grace after a process restart
- **THEN** journaled candidates restore only after identity verification, without overwriting conflicting files

#### Scenario: Grace expires
- **WHEN** quarantine reaches its grace deadline without further user action
- **THEN** files remain retained until separate permanent-removal review succeeds

#### Scenario: Interruption during quarantine
- **WHEN** the process stops during a journaled quarantine move
- **THEN** restart reconciles source and destination identities and preserves ambiguous data instead of deleting it

### Requirement: Recovery never silently adopts abandoned history
Recovery SHALL identify recoverable snapshots, missing dependencies and abandoned source scopes without replacing accepted state automatically. Explicitly selected recovery SHALL validate dependencies and retain the previous accepted snapshot. Failure SHALL preserve both the current session and recoverable originals.

#### Scenario: Unsaved committed capture survives shutdown
- **WHEN** assets or snapshots newer than the FLP are discovered
- **THEN** they are offered as separate recovery candidates without redirecting the FLP's exact saved references

### Requirement: Missing assets expose explicit recovery choices
The system SHALL identify unavailable dependencies and offer Locate assets, Restore backup and explicit Re-learn song. Decodable swell identities, positions and edits SHALL remain preserved, together with unaffected output and surviving assets. Metadata decode failure SHALL be reported separately rather than claiming the arrangement is intact.

#### Scenario: Source folder is unavailable
- **WHEN** valid saved swell metadata refers to missing or offline audio
- **THEN** the message confirms the preserved arrangement and offers verified folder location or backup recovery before re-learning

### Requirement: Re-learning is recovery rather than exact restoration
Re-learn recovery SHALL require explicit user choice and DAW playback, preserve swell identities and edits, and regenerate only with verified required IRs and complete fresh source. It SHALL preserve surviving assets and saved auto-generation policy. Frozen results SHALL require explicit consent to replacement; changed-input/DSP and missing-export limitations SHALL be disclosed.

#### Scenario: Re-learning succeeds
- **WHEN** fresh source coverage commits and required IRs are available after explicit recovery authorization
- **THEN** affected swells can regenerate with preserved arrangement and edits without a promise of bit-identical old output

#### Scenario: Re-learning is partial or cancelled
- **WHEN** capture is incomplete, cancelled or fails, or a custom IR remains unavailable
- **THEN** existing arrangement and surviving dependencies remain preserved, missing requirements stay explicit and no successful full recovery is claimed

### Requirement: Future cache limits cannot reclassify durable history
A future rebuildable-cache policy SHALL require separate eligibility and reconstruction review before eviction is enabled. Current source, IRs, immutable saved renders, exports and unknown old-project dependencies SHALL remain protected. Swell deletion, regeneration or lack of a current reference SHALL NOT establish garbage-collection eligibility.

#### Scenario: A deleted swell has an older saved project
- **WHEN** current metadata drops its render reference but older FLPs cannot be enumerated
- **THEN** the render remains protected rather than being classified as disposable cache
