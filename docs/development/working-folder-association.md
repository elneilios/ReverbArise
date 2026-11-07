# Working-folder association — issue #8

Phase 4 of `portable-project-assets` continues on PR #112 from merged PR #31.
The owner approved the native Choose folder → verified copy → Use This Folder
family and separately approved explicit incomplete recovery workspaces.
Production Settings now integrates that family. The processor reads envelopes
1–9, retains v8 for sessions using the legacy default, and writes v9 after an
explicit association. Capture schemas, creative state and Export versions are
independent and unchanged. Full phase-4 acceptance remains subject to the gates
below; this is not authorization to release, merge or clean up assets.

## User flow and scope

- Fresh/unsupported hosts keep the existing user-data working directory. No
  project path is inferred from CWD, project names, presets or Save As.
- In Storage, Choose folder copies and verifies the accepted dependency closure
  without changing the live folder. Use This Folder rechecks the prepared copy,
  then conditionally changes the source/engine location and requests a DAW save.
  It does not claim the host has saved anything. Originals stay untouched.
- Choosing a new empty folder for an already associated instance is an explicit
  physical Fork: verified independent bytes and a distinct store identity.
  Reopen/duplication uses a fresh writer lease without automatically moving assets
  or renaming durable project, instance, swell or legacy asset identities.
- Locate selects the associated store folder. Identity, exact current metadata
  and all required bytes are verified before adoption; a mutable newer head is
  never substituted. A moved folder must remain within the supported path limit.
- Restore selects an exact completed receipt. If the associated folder is lost,
  the user also chooses an empty destination; verified assets are copied into a
  new store before adoption. With an available working folder, the existing
  verified same-root restore behavior remains available.
- When assets cannot be recovered, Choose recovery folder prepares a deliberately
  incomplete workspace. It retains original locators, decodable arrangement and
  verified survivors; missing/unknown requirements stay explicit. Creation emits
  no complete snapshot or backup receipt. Use This Folder is still explicit.
  Then use normal Learn with Detect events off and Recapture audio on. Existing
  unfrozen swells rebuild only after complete required fresh source and verified
  IRs are available. Frozen consent and lost-export limitations are unchanged.

Working-folder actions and Backup/Restore/Locate occupy two pinned action rows.
Details above them scroll, using the approved shared ActionButton, EllipsisLabel,
FocusViewport, helper roles and comfortable spacing. No new style token, visual
variant or Storage Learn/Generate control is introduced. Scan retention remains
unchanged when returning to the same page; a changed session/root gets its own
inventory scope.

## Ownership and publication

`Storage/WorkingFolder.h` owns worker-only exact-byte preparation, verification,
selected-root pins, registry hints and exclusive runtime writer leases. Complete
packing includes source/context IRs, retained library/default/swell IRs, exact wet
results, metadata and every saved Export-history dependency. A recovery record
is also a protected dependency. Missing required files prevent complete packing.
There is no garbage collector or automatic durable-file removal.

`CaptureSessionSnapshot` retains an immutable `CaptureStorageLocation`, including
the selected projects root, managed-store boundary, lease and association metadata.
Old accepted snapshots/readers keep their original location alive. An unresolved
association retains known source metadata with a non-writable location; opening
it never redirects writes into user data. The UI reports the unavailable store
instead of exposing the internal non-writable placeholder.

The production binding runs off the message/audio threads. It pauses engine file
jobs, verifies the prepared closure and prepares any restored source frame. The
capture actor then acquires the processor/model publication guards in the defined
order, rechecks current metadata/admission and adopts the location without resetting
Learn state, advancing source generation or draining callback packets. Capture
that has already begun rejects the handoff. Packets admitted after a successful
swap use the same source generation and the new directory. Host save serialization
is fenced with the final publication and the coalesced dirty notification.

Material fingerprints exclude directory location. The common swell engine owns
sound maintenance; association adds no special recovery renderer. Engine/offline
jobs use the accepted source location. Retained Export preview/detail/delivery
readers use their pinned preparation location, and external Save is blocked inside
both the active and retained managed stores. No new disk work or locks enter the
real-time callback. Offline rendering retains its existing non-real-time contract.

## Host state, resolution and compatibility

V9 stores exact accepted v8-domain metadata plus bounded association and source
metadata. Saving is memory-only; it neither hashes files nor publishes filesystem
snapshots. A save during asynchronous host restore retains the loaded IDs,
reference/revision and association rather than serializing an empty loading frame.
Embedded accepted source metadata takes precedence over a locally changed manifest.
Dependency-bearing unknown versions fail transactionally.

Store/snapshot UUIDs and hashes identify metadata; the snapshot hash is not a
complete-pack receipt. Absolute paths exist only in local registry hints under
the original user-data Projects root. Saved writer IDs grant no ownership: every
associated runtime acquires a new UUID writer branch and exclusive lease. Registry fallback
examines at most 128 records; ambiguous candidates require explicit Locate.
An explicit operation can create the registry through pinned directory ancestry.

An existing associated store with missing files retains writable recovery access
after identity/metadata validation, without claiming complete closure. A wholly
unavailable store stays non-writable until explicit Locate, Restore or workspace
adoption. Recovery origins are bounded, retained inline in v9, and accompanied by
a pinned hash-verified recovery record. History is never silently truncated to fit.

Managed categories contain pinned `.directory-pin` files. Together with pinned
ancestry this prevents replacing an emptied category with a junction while allowing
the existing immutable UUID/no-replace publishers to work. Administrative markers
remain protected. The backend supports local Windows paths up to its measured
240 UTF-16-unit boundary; unsupported paths produce a specific error. No claim is
made for UNC, device paths or cloud-sync transactional guarantees.

Keep pre-upgrade FLPs, the original binary and the untouched legacy store. Loading
a legacy state after an associated session explicitly selects the legacy root.
An old binary may reject v9; rollback does not promise it can read an upgraded FLP.
Do not overwrite the only pre-upgrade project or delete original assets.

## Validation

Native component/processor coverage includes production Settings bindings and
pinned rows, stale/cancel/failure handling, unchanged Learn state, v9 saves during
restore, independent reopen leases, physical copies, legacy reopen, whole-store
loss, explicit workspace recovery and verified Locate/Restore. Two native processor
processes simultaneously publish actual captures, imported IRs, renders and exports,
then reopen their exact states while original hashes remain intact. This is native
MSVC coverage, not FL Studio or second-profile evidence.

Phase 4 passed the full native regression, six destination-only restore cases
in a separate Windows account, and the scoped FL Studio smoke. FL Studio reopened
both physical stores, preserved the association across Save As, and reopened the
retained v8 FLP after restoring the exact previously installed bundle. See the
dated evidence for fixture and host coverage. Issue #8 and the active change
remain open for separately gated phases 5/6.

See [backup contracts](portable-asset-backups.md),
[version/rollback matrix](asset-storage-contracts.md),
[earlier native evidence](../validation/2026-11-06-issue-8-working-folder.md) and
[production integration evidence](../validation/2026-11-07-issue-8-phase4-settings.md).
