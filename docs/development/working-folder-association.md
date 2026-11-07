# Working-folder association — issue #8

Phase 4 continues `portable-project-assets` on `codex/8-working-folder-association`,
from merged main `f98157e`. The owner authorized this phase on 2026-11-06. This
first increment prepares the storage transaction and a disposable native control
preview. **Production still reads processor envelopes 1–8, writes v8, and uses
the existing user-data root. The new panel is not in production Settings.**

## Ownership and transaction

`Storage/WorkingFolder.h` is a worker/control-only API. It extends the existing
pinned-handle, exact-byte backup backend; it neither calls Learn/rendering nor
changes a live processor. The processor binding will own the eventual atomic
association change, worker-root handoff and coalesced host-change notification.
No disk work belongs in the callback or host-state serializer.

1. Choose an explicit existing empty dedicated folder, or a supported store.
   No host path is inferred. Unsupported/unsaved hosts keep the user-data default.
2. Pin the accepted source/creative metadata and every declared dependency.
   Publish and verify a complete v1 backup before materializing working aliases.
3. Materialize `working/<project>/<instance>/<legacy-alias>` through the same
   no-replace, hash-verified copy mechanism. This deliberately duplicates the
   initial backup objects rather than introducing hardlinks or changing codecs.
4. Keep a prepared candidate; the live root remains unchanged. `Use This Folder`
   rechecks the exact root, epoch, XML, manifest, availability and destination
   hashes on a worker. A changed session, cancellation, corruption or conflicting
   destination fails without adopting the candidate or deleting originals.
5. The future production binding must recheck admission and atomically hand off
   capture/render/export roots before accepting. It requests a DAW save; it
   cannot claim the host saved. That binding is pending; native control approval is recorded below.

`WorkingFolderPanel` composes existing ActionButton, EllipsisLabel, FocusViewport,
text roles and spacing tokens. Content scrolls above pinned Choose/Use actions;
Stop replaces the actions during work. Picker completion uses a weak component
reference and accepted-state checks. The preview has a real disposable binding,
verified copying, rejection and explicit activation; it cannot alter a real FLP
or the installed plugin. No new style token or exception is introduced.

## Candidate metadata and writer ownership

The backend-only candidate codec reserves processor envelope **v9**, association
metadata **v1**. It embeds the canonical v8 accepted envelope and exact capture
manifest metadata (not PCM), plus store/snapshot/writer/registration identities.
The snapshot hash is domain-separated from the v1 pack snapshot/receipt hash.
Store identity plus complete saved closure must validate; a backup receipt for
an older revision is never substituted for current host edits. The production
processor does not yet read or write this candidate envelope.

This keeps a host save free of asynchronous filesystem publication: exact
accepted metadata can be serialized from memory even while a separate store
checkpoint is pending. The production implementation still needs coherent
source/creative publication tests, bounded v9 host decoding and the normal
missing-folder recovery binding. No claim of those behaviors is made here.

Absolute paths exist only in immutable local
`<user-data Projects>/locations/<registration-uuid>.json` hints. Hints grant no
authority. Resolve the explicitly registered candidate by store identity and
complete current closure. If it is unavailable, examine at most 128 explicit
registrations; multiple verified locations require explicit selection. Locate
verifies the selected folder and writes a fresh registration. It never chooses
the newest head, rewrites creative edits or infers a Save As operation.

Every admitted runtime gets a new UUID branch and an exclusively held native
lease under `instances/<instance>/branches/<writer>/lease`. A serialized writer
is provenance, never reusable ownership. Probe candidates are verified before
acquiring a lease. Closed lease files remain as evidence; no collector is added.
Existing immutable UUID/suffix filenames and no-replace publication preserve
historical aliases. Integration must keep a root/lease pin alive for every
admitted job; backend lease tests alone do not complete task 4.2.

Preparing from an associated working root into a different store performs a
physical copy with a distinct store identity. This is the backend for explicit
Fork. The production Fork command and end-to-end independent-write tests remain
pending; Save As never triggers it automatically.

## Compatibility, rollback and remaining gates

Current production formats and user-data paths are unchanged. Old FLPs continue
to name their exact original dependencies. Keep the original plugin binary,
pre-upgrade FLP and untouched legacy store. After future association activation,
save a new FLP copy: an upgraded envelope need not open in an older binary.
Rolling back means using the retained original binary and pre-upgrade FLP with
the original store, never rewriting the only upgraded or rollback project.
The original-binary rollback exercise is still required before task 4.4 closes.

Native approval was received for this revised control composition, as recorded below. Production integration in
Settings must still satisfy the active design's “Explicit association” decision.
Production root handoff, actual host-state compatibility, missing-store recovery,
two-process capture/render/export publication, fresh-profile association proof
and consolidated host validation remain required. All phase-4 tasks stay unchecked
until their complete acceptance criteria pass. Durable cleanup, scratch removal,
eviction, dedup experiments, compression, release and merge retain their separate
gates. The issue and active change remain open.

See [backup contracts](portable-asset-backups.md),
[version/rollback matrix](asset-storage-contracts.md) and
[dated evidence](../validation/2026-11-06-issue-8-working-folder.md).


## Owner native approval — 2026-11-06

The owner explicitly approved the working-folder layout and Choose folder →
verified copy → Use This Folder interaction: “Approve layout and interaction”.
This clears the scoped native control gate, not the remaining production root,
writer, state, recovery, rollback or validation gates. The owner closed the
disposable preview. Both unchanged native-focus tests passed on retry after that
review ended. No foreground-focus assertion was removed or weakened.


## Approved integration decision: an irrecoverable whole working folder

The portability delta requires a verified copy before association, while the
retention/recovery delta promises normal Learn when original assets cannot be
recovered. Missing individual files in an available working folder can follow
the existing recovery path. If the entire associated folder is lost or offline,
its writer root is unavailable and a complete verified transfer is impossible.
The present plan does not authorize an incomplete transfer as a successful pack.

The owner approved this explicit exception on 2026-11-06: create a new recovery workspace
only after a deliberate user choice. Keep the original store/snapshot locator,
all decodable swell identities/settings and unresolved references for later
Locate/Restore. Clearly label the workspace incomplete; never publish a complete
copy/backup receipt. The user then uses normal Learn with detection off; existing
complete-source and IR checks, automatic unfrozen regeneration, frozen consent
and missing-export disclosures remain unchanged. No Learn/Generate controls are
added to Storage, no original data is deleted, and no fallback migration occurs
silently. Host association changes still request an explicit DAW save.

The owner chose the explicit recovery workspace. Native layout approval and this recovery exception are recorded separately; all remaining validation and rollout gates still apply. Worker/backend primitives are implemented and covered by native tests; production Settings, coherent host saves and atomic live-root handoff remain pending.


## Recovery and location primitives — 2026-11-06

`prepareRecoveryWorkspace` requires a deliberate, empty, separate destination.
It preserves the accepted processor/source metadata, original locator and
verified surviving dependencies, records missing dependencies and unknown source
history, and never publishes a complete snapshot or receipt. A known missing
capture manifest can be reconstructed from its exact accepted metadata; missing
audio remains unavailable. Unknown capture coverage is never invented. Native
tests then use the ordinary complete Learn capture transaction in this workspace.

The candidate v9 codec carries bounded original recovery locators alongside the
new working association. Unsupported locator versions/identities are rejected
transactionally. The recovery record lives under the instance namespace and is
hash-verified and pinned. Production continues to write v8 until host binding and
rollback checks are complete.

Capture snapshots now own an immutable location/lease. Host-restore commands can
resolve a location off the callback, retain accepted source on failed handoff,
and retain known metadata while blocking writes when a folder is unavailable.
Old snapshots keep their original lease. The engine, offline rendering and
export use the accepted source location; a Loading source defers file work and
an unavailable location blocks it. A mere directory change keeps material
fingerprints unchanged, and the focused engine fixture verifies no unnecessary
convolution. These primitives are not the final user-association transaction:
production must quiesce engine file work and conditionally adopt the prepared
source without resetting workflow or dropping queued capture audio.

Managed category directories contain pinned `.directory-pin` files. Together
with pinned directory ancestry this prevents replacing an emptied category with
a junction. Native probes confirm nonempty-category junction conversion is
rejected and ordinary immutable capture publication still succeeds. No broader
drive/profile directories are write-locked; marker files are administrative
protected files, not assets eligible for cleanup.
