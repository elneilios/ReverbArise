# Design

## Context

See proposal.md for motivation and [audit](../../../docs/development/asset-storage-audit.md) for pinned evidence. Existing immutable chunks/snapshots and full-render metadata are useful mechanisms to extend. Current project/instance UUIDs do not identify host file paths. Unknown external FLP snapshots prevent complete reference enumeration. This is a proposed architecture; none of the following services/formats exist yet.

## Goals / Non-Goals

**Goals:** portable, inspectable source and exact saved wet results; independent future writes; conservative retention; failure recovery and legacy readability. Complete packing must work without host project-path support.

**Non-Goals:** audio/DSP changes, capture admission/transaction changes, #11 UI or state-machine changes, discovering every FLP on disk, silent reparenting, automatic durable-data deletion or mandatory compression. This planning submission does not authorize implementation.

## Decisions

### 1. Explicit association with a portable store

Keep legacy storage readable and initially writable. Offer a user-selected Project Asset Folder, suggested beside the FLP as `ReverbArise Assets`, through future settings/asset administration. Do not infer the FLP path from process CWD, a preset filename or directory names. Optional supported host metadata can suggest a location only after a targeted FL Studio/JUCE spike establishes semantics; explicit selection always works. Association is optional until a user asks to pack/migrate, so existing learning is not blocked.

A proposed folder contains:

```text
ReverbArise Assets/
  store.json                       # format/store identity and minimum reader
  objects/sha256/<prefix>/<id>.wav  # immutable exact audio bytes, initially WAV
  descriptors/<id>.json             # immutable format/dependency metadata
  snapshots/<snapshot-id>.json      # immutable complete accepted dependency closure
  instances/<instance-id>/branches/<branch-id>/head.json
  transactions/<operation-id>/     # owned staging + durable journal
  quarantine/<operation-id>/       # only proven transient candidates
```

Content paths are relative to the selected store, never relative to an unknown FLP path. Snapshot locators contain store ID, snapshot ID and legacy fallback; absolute local hints live in a non-authoritative registry. On a second PC, user selects the moved folder once with Relink; no automatic FLP-parent discovery is promised. A locally registered identical snapshot can resolve automatically only after identity/hash validation. Same store ID at diverged locations is not sufficient authority. Conflicting candidates require explicit selection, never silent latest-head substitution.

Alternatives: embedding all PCM bloats host state and changes save behavior; hardlinks/external shared global blobs undermine self-containment; automatically moving the cache endangers old FLPs. A folder bundle makes dependencies inspectable and needs no archive extraction engine initially. Pack is a ReverbArise asset operation, not an FLP writer or a full backup of unrelated instruments.

### 2. Identity: preserve legacy IDs, add writer branches

Keep serialized projectId/instanceId/swell IDs as durable provenance; do not reinterpret old IDs as filenames. Add independent store/snapshot IDs and a writer branch to future metadata. Immutable objects can be shared inside a store; mutable heads are hints scoped to a writer, never authoritative for old saves. Fresh runtime writers acquire unique branch ownership/leases, so loading the same state twice cannot share future publication authority. Each saved snapshot pins a branch-independent closure.

Save As and filesystem copies cannot reliably be distinguished from reopen. Default to sharing immutable history and independently branching future writes, even for a normal reopen; compact branch metadata later only under safe retention rules. Provide an explicit Fork/Pack to a different associated store when the owner wants physical isolation. No automatic projectId rewrite or silent migration on Save As. Multiple fresh instances associate with the same store only explicitly; preserve their different legacy IDs. Repeated open/clone/copy across processes needs tests independent of the current in-process epoch protections.

Alternative: detect a duplicate by IDs currently loaded in one process. This misses offline copies and other host processes; it cannot be a deletion safety oracle.

### 3. Separate byte objects, audio identity and render descriptors

Initially reuse exact existing float32 WAV/JSON bytes through adapters. A byte-object SHA-256 addresses exact persisted bytes and includes a domain/version tag in its namespace. Retain current normalized decoded content identities and legacy paths/checksums as aliases. Do not coalesce signed-zero-distinct source based only on normalized sample hash. Decoded equality can report potential duplication without authorizing replacement.

Descriptors preserve rate, channel layout, frame length, format, original checksum, engine, complete render dependency/timing/bounds and calibration identity. Distinct render dependency descriptors can reference a single byte-identical audio object while remaining distinct semantic render revisions. Capture descriptors preserve absolute spans, revision, covered silence and gaps independently of byte storage; initially do not reslice/rechunk or rewrite manifests. Slices/recompression are later separately reviewed work.

Put-if-absent publishing uses unique staged files and atomic no-replace rename. On collision/existing hash, verify stored bytes/dimensions rather than overwrite. Pin source and dependencies while copying and while a job/export/pack runs. Dedup within the selected store/pack only; no mandatory machine-global object pool, cross-volume hardlinks or cross-project deletion authority. This trades some inter-project duplication for independent backups.

### 4. Pack pins one accepted snapshot and verifies its closure

A version-aware visitor enumerates the accepted capture manifest/chunks, metadata-only silence/gaps, session definitions/edits/selection/freeze, default IR even with zero swells, per-swell IR and completed renders, timing/calibration and accepted #11 provenance. Include the exact managed bundled IR used, not just an ID whose shipped version can later change. Include stale/frozen retained references as saved, with their honest statuses. No loss of disabled/muted dependencies. When supported #6 state introduces durable export/mix-revision references, the visitor must include those dependencies too, copying required export bytes without moving them or assuming collector ownership; no export path is asserted for the current baseline. Unknown dependency-bearing state versions stop a complete-pack claim.

Capture manifests alone cannot discover historical IR/renders in offline FLPs. Default Pack includes the selected accepted session only and says so explicitly. An optional archive mode can copy all retained files/snapshots in chosen stores plus user-provided session snapshots, but cannot claim arbitrary historical FLP closure without their state. Do not present Pack as permission to delete originals.

Transaction sequence:

1. On a control/worker boundary, pin one coherent accepted source/session/asset epoch. Wait for a consistent snapshot or retry; never merge source revision A with session revision B. No filesystem I/O in save callback or audio callback.
2. Preflight closure, versions, missing dependencies, destination permissions, size estimate and available space; allow headroom for staged + final copies. Pending capture/import/render remains excluded. Display that the snapshot includes committed work only. Missing required files fail complete packing; a separately labelled incomplete recovery copy cannot masquerade as complete.
3. Copy exact bytes to unique staging on the destination volume; journal IDs/expected hashes and path ownership. No references or folder association change yet. Copying works across volumes; atomicity is only claimed for final publication within one volume.
4. Verify each byte hash, descriptor agreement, relative containment and complete closure using only staged destination data. Include full completed wet renders so compatible reopen performs validation/loading, not convolution or resampling.
5. Publish the immutable snapshot then completion marker/index atomically within destination. Preserve previous successful pack and never overwrite a conflicting destination. Cancellation/error leaves the prior association intact and owned staging recoverable/reportable.
6. Pack alone does not redirect the live session. After verified publication, an explicit Use This Folder action can switch association transactionally, notify host via existing coalesced mechanism, and request a new FLP save. The plugin cannot assert the host has saved it. A receipt records snapshot ID, counts/bytes, hashes, build/schema versions and excluded pending work. Keep pre-switch FLPs and legacy assets.

Multi-instance scope: an instance can pack only its own available accepted state. A project-wide pack requires explicitly collecting accepted snapshots from every instance (register/import receipts); report participating instance IDs and do not claim discovery/completeness for absent instances. User transfers the FLP plus all relevant receipts/folders. No parsing or rewriting closed FLPs is required initially.

Alternative: regenerate renders on destination. Rejected as default because engine updates, frozen output, missing originals and CPU cost threaten exact immediate recovery. Complete compatible packs include them; incompatible timing/engine remains an honest unavailable condition under existing readiness rules, not a license to resample.

### 5. Reference tracking reports uncertainty rather than inventing reachability

Maintain a rebuildable local catalog of observed snapshots, aliases, store identity, dependency edges and transaction/job leases. Immutable snapshot manifests are authoritative; refcounts and head pointers are accelerators. Trace current/registered prior snapshots, packed snapshots, accepted context, pending jobs, pins and active readers. An interrupted catalog update cannot unprotect bytes: rebuild or classify unknown. Missing/offline roots, unknown metadata or index corruption block reclamation in the affected scope.

Host serialization is evidence a snapshot was observed, not proof it was saved or that all old host copies are known. Catalog absence cannot prove durable-asset unreachability. Legacy trees retain a blanket unknown-external-reference pin. New durable snapshots are protected indefinitely by default too. Last access, age, project name and missing current definitions never remove that protection.

Future Storage/Manage Assets can report exact inspected paths, scan timestamp, logical bytes/counts by category/history, known shared bytes (count physical object once), missing/unknown dependencies, duplicate byte hashes and optional decoded-equivalence groups. Separate eligible scratch bytes from potentially redundant durable bytes and protected exports; do not label all duplicates reclaimable. Unknown/inaccessible sizes and allocation overhead stay explicit. Scan only selected managed roots read-only; never follow reparse points into unrelated paths.

### 6. Conservative retention and two-stage scratch cleanup

| Class | Default policy | Eligibility for proposed cleanup |
| --- | --- | --- |
| Capture and imported IR | Indefinite protection, including unreferenced/unknown history | Never in ordinary cleanup |
| Completed wet renders and descriptors | Indefinite protection for old/frozen/unknown sessions | Never infer disposable from reconstructibility |
| Manifests, receipts and accepted provenance | Indefinite immutable protection | Keep closure and recovery history |
| Export/Playlist WAVs | User-owned durable files, outside plugin collector | Never cleanup targets, even if inside selected root |
| Unknown/legacy temporary files | Preserve and report | Extension/age alone insufficient |
| Proven transaction scratch | Retain until ownership/recovery/lease checks pass | Opt-in quarantine preview only |

Initial collector is disabled by default and limited to scratch produced by journaled new-format operations. A candidate must have verifiable ownership, a terminal abandoned/cancelled operation, no committed snapshot/alias/recovery edge, no active writer/job/reader/export/pack lease, and be under an explicitly managed staging root. Obtain exclusive worker-side maintenance coordination across processes and revalidate identity, file handle, hash and graph immediately before action. Sharing failures, uncertainty, path replacement or new leases skip the file. Never lock the audio callback or reclaim its in-use buffers.

Preview lists exact files/counts/bytes, operation and affected scopes, reason, unknown-reference exclusions and recovery deadline. On explicit approval move proven scratch into same-volume quarantine and write a durable reversible journal; nothing outside reviewed candidates moves. Default proposed grace is at least 30 days, configurable longer, with no automatic final erase. After grace, a second explicit permanent-delete review rechecks all eligibility/leases. Expired grace alone grants no permission. Restore reverses journaled moves and verifies hash, refusing overwrite conflicts. Crash during move is reconciled using source/destination identity; journal failure keeps/reports files rather than destroying them.

Captured source accidentally staged but never committed can still be the only recording. Therefore even new capture staging is excluded from ordinary scratch cleanup unless the owner explicitly classifies it as discarded recording after recovery inspection; phase 3 initially targets pack-copy scratch only. Recovery shows abandoned capture namespaces and available snapshots without adopting them automatically. Legacy durable purge is outside this plan; a later expert operation would need explicit scope, a verified independently readable backup and an honest permanent-loss warning, not an inferred safe GC label.

### 7. Failure and path policy

Validate schema/size/count limits before allocation, canonical relative paths before resolution, and containment on opened Windows handles. Reject traversal, absolute/UNC/device/drive-relative entries, alternate data streams, reserved names, case-fold collisions, escaping symlinks/junctions/reparse components and mismatched store/hash identity. Revalidate at publication to address path races; lexical checks alone are insufficient. Support Unicode paths; long paths need a measured Windows/JUCE capability test and a specific error when unsupported. Reject ambiguous case-only duplicates on Windows. No pack requires executing content or trusting an embedded absolute path.

Disk-full, cancellation, read-only/locked/offline roots and interrupted transfers never replace accepted state or delete originals. Keep a bounded diagnostic/recovery record, previous successful pack and any safely committed objects; partial packs lack completion markers. Recovery can resume only after revalidating staged bytes and current operation identity. Flushing and rename semantics must be failure-tested; do not claim directory durability or cloud-sync transactional safety. Unaffected swells and dry behavior follow existing workflow readiness; no forced capture/mode transition is added.

### 8. Compression is a later independent experiment

WAV remains the baseline and initial portable format. Evaluate standalone lossless WavPack per chunk and byte-preserving generic compression on synthetic finite floats plus representative permitted fixture audio. FLAC integer conversion is excluded for arbitrary float32. Require exact sample bits, independent file checksums, channel/rate/length/bounds equality and unchanged waveform/render output. Current normalized content hash is insufficient to prove signed-zero bit fidelity.

Measure compression ratio by category, encode/decode CPU/wall, peak scratch memory, first-open/seek latency, file count and cold/warm reads at 44.1/48/96 kHz. Reject hybrid lossy fallback or missing correction dependencies. Codec versions/frame boundaries become explicit metadata. Keep decode off callback, within current working limits; do not alter DSP. Adopt only through a later reviewed gate after measured benefit, bounded random access and reader/rollback tests; preserve legacy bytes and offer uncompressed packs. No promised savings or codec migration is part of this proposal.

## Risks / Trade-offs

- [Unseen old FLPs] -> indefinite durable retention; never equate catalog reachability with full knowledge.
- [Pack omits another instance/default/context] -> explicit scope receipts, schema-aware closure, zero-swell/default and #11 fixtures.
- [Copied folders share identity] -> immutable snapshots and exclusive writer branches; explicit physical Fork when desired.
- [Legacy byte references break dedup] -> preserve aliases and original files; initially only copy, do not rewrite saved refs.
- [Old engine sound differs] -> include exact full wet bytes; no automatic rebuild to certify pack completeness.
- [Free-space/atomicity overstated] -> destination-local staging, conservative estimates, failure injection and incomplete markers.
- [Unbounded retained disk] -> honest accounting and backup guidance; storage growth is preferable to silently losing source.
- [UI scope creep] -> later settings-only control review, no #11 Learn controls or workflow modifications.

## Migration Plan

Each phase requires its own reviewed implementation scope and appropriate focused checks. This document authorizes none.

0. **Contract and evidence:** reconcile #11 accepted-context/schema contract and #6 export protection; run disposable growth/clone fixtures and optional host path spike. Gate: source-backed measured report and reader/version matrix.
1. **Inventory and adapters:** read-only inventory and version-aware closure/legacy adapters. No write relocation/deletion; keep legacy default. Gate: exact totals/uncertainty, old-state fixtures, no user mutations.
2. **Pack/Relink and association:** first exact-byte standalone folder pack, then explicit association and writer branches. Gate: destination-only second-profile test with originals unavailable, no recapture/convolution/resampling for compatible complete snapshots, all failure/path/concurrency cases.
3. **Safe scratch administration:** only journaled pack-copy scratch first, preview/quarantine/undo then explicit final erase after grace. Gate: fault-injection and zero durable-source/export eligibility, separate owner review. No legacy orphan purge.
4. **Efficiency experiments:** store-local exact-byte dedup then optional compression, separately approved after measured benefit and compatibility proof. Source reslicing/global pools remain outside scope.

Rollback at every phase: disable new association/services, reopen pre-upgrade FLP with original binary and untouched legacy store. Never overwrite an old FLP in place as the only rollback copy. New binary reads supported legacy state and new versioned locators transactionally; old binaries can reject newer state, so rollback does not promise they can read upgraded FLPs. Readers must not silently downconvert unknown metadata. Do not cut over until destination verification and explicit association; errors leave original refs authoritative. A new-only session has no guaranteed old-binary equivalent: export/retain documented compatible audio and pre-upgrade snapshots rather than claim lossless reverse migration. Changing default storage or durable purge needs later explicit approval beyond the tasks here.

## Open Questions

Deferrable tuning only: which supported Windows path lengths/volumes qualify initially; benchmark thresholds and codec effort; inventory progress/cancel cadence and a longer grace default. These do not weaken completeness, explicit association or indefinite durable retention. Host project-path support is an optional optimization; #11 schema reconciliation is a phase-0 implementation gate, not an assumption of support.
