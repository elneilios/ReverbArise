# Learn source/context protocol — implementation contract

Issue #11 implementation starts from merged source `dc003a7` / main `653f13a`.
The following protocol is pinned before code changes; implementation and tests
record which parts are delivered. Context-codec and atomic pass publication are
implemented. Issue #38 now treats accepted pass contexts as immutable capture
history: read-only Learn candidates and a shared explicit/opted-in Generate
transaction replace source-owned deferred creation commands.
Final integration/host acceptance and checkpoint B remain tracked separately.

Task 4.1 now implements the schema-2 codec, bounds and pure legacy-baseline
migration. Readers accept schema 1/2. Workflow capture now copies the pinned pass
configuration on the worker and publishes schema 2 with complete source ownership
at the same Stop acceptance fence. Contextless low-level compatibility adapters
retain schema-1 output. Protected mapped-only passes retain their prior record.
The `generationContexts`
table records `id`, `origin`, `automatic`, `sensitivity`, `separationSeconds`,
`ir` and `irName`; `contextSpans` records signed `start`/`end` and `context`.
New context/span records reject unknown fields, invalid identities and ambiguous
ownership. The neutral `Core/ManagedAsset.h` shares the existing Asset type and
codec contract without changing creative-state serialization. Schema 2 requires
context coverage to equal the mapped-source union exactly; it cannot own gaps.
Migration marks unknown historical settings legacy, coalesces equal neighbours
and leaves PCM, source-range metadata and existing swell assignments unchanged.

## Durable envelope and bounds

Issue #38 introduces creative metadata **v11** with a strictly typed
`autoGenerateOnStop` boolean. Fresh `Session`/plugin instances initialise OFF;
supported creative versions 1–10 and processor envelopes 1–2 without creative
metadata restore ON. Explicit v11 OFF/ON survives save/reopen independently of
Automatic Swells visibility. Missing or non-boolean v11 values reject the whole
restore, preserving accepted state. The processor XML envelope remains v8,
capture manifests remain schema 2 and numeric workflow/plugin identities remain
unchanged. Older binaries cannot read v11: retain pre-v11 saved project/cache
copies for rollback; no down-save or destructive asset migration is provided.
Creative v11 also saves each swell's optional `generationSource` PCM fingerprint.
This records the last committed automatic definition's source dependency even
before wet rendering finishes; it never claims a render is Ready. Older versions
have no such field. A matching retained render dependency can establish their
prior generation identity; otherwise an eligible explicit generation records it.
The new string is empty or a strict 64-character lowercase hash. Candidate
snapshots, pending Stop intents and prepared transactions are never serialized.

Provenance values remain `automatic=0`, `manualCreated=1`, `manualEdited=2`.
Version-1 creative state with no provenance decodes conservatively as
manualEdited; invalid newer origin values reject the complete state. Manual
creation/confirmed suggestions start manualCreated. Musical edits, name and
activity changes through `editChecked`, IR assignment/import, Reset All and
freeze/unfreeze retain or promote manual ownership. Changing a display-only IR
label does not infer ownership. Disabled state never infers automatic origin:
only an explicitly persisted automatic definition can remain eligible. Frozen
definitions are protected regardless of their original provenance. Existing
automatic-target tests cover protected identity/edits/IR/render retention; engine
tests cover actual edit, assignment, reset and freeze entry points.

- Capture manifest schema 2 extends the existing immutable manifest with a table
  of generation contexts and ordered, nonoverlapping absolute context spans.
  Audio ranges, PCM format, project/instance IDs and managed storage roots retain
  their existing meaning. Source and contexts publish through the same manifest
  reference/acceptance fence, not independent pointers.
- Each context contains a content-derived stable identity, effective automatic
  policy, finite sensitivity in [0,1], separation in [0.001,2] seconds, the accepted
  managed IR metadata/label and an explicit pass-versus-legacy origin. IR metadata
  uses the existing Asset validation/serialization contract, not a file-picker
  path or a catalogue index.
- Context spans contain signed absolute start/end samples and a context identity.
  A schema-2 accepted map has unambiguous context ownership for its mapped source.
  Adjacent equal spans coalesce. Additive capture preserves old spans; explicit
  Recapture replaces only admitted spans, with the same first/last-received rules
  as its paired source. A target resolves the context at its arrival.
- Bound one manifest to 4096 context definitions, 65536 context spans and at most
  8 MiB of encoded context metadata. Reject overflow explicitly before acceptance;
  do not drop ownership or alter committed source. Context/progress metadata is
  accounted separately from the existing 256 MiB PCM working allowance.
- Context-only changes do not enter audio-content fingerprints or render/gain
  dependencies. Identical source/configuration reuses identities. Selected future
  preferences remain in their existing session fields and state envelope v8;
  capture schema is versioned independently.

## Admission and lifetime

Prepare the full immutable configuration off the callback. Publish/pin its bounded
slot/revision through capture's atomic admission authority, so a racing edit is
wholly before a pass or rejected for that pass. Keep the admitted configuration
alive until worker settlement has copied it into the coherent source record.
The callback must not copy strings, serialize, allocate, take an engine lock or
destroy an owned configuration. Restore/new-session epoch and request identities
fence stale imports, jobs and display snapshots. Missing accepted IR does not
prevent dry capture; it remains an explicit generation error.

The implemented admission mechanism uses two `GenerationConfiguration` slots,
written and copied only under the off-callback engine lock. An index in the same
lock-free workflow word as the pass/phase pins the accepted slot. Publishing an
idle edit uses a compare/exchange against the observed pass; if Play wins,
publication fails before model preferences or host notifications change. Mode
changes preserve the slot and invalidate gesture/chooser tokens via the pass
generation. Slot reuse is blocked through recording, draining and validation.
The capture worker copies this pinned value once per pass outside its command
fence; the staged transaction owns that independent copy until settlement.
Source and context remain private through preparation, then publish as one
immutable manifest. Metadata-only replacement advances the accepted record while
preserving PCM assets/range revisions, source fingerprints and audio-dirty
callbacks. Identical source/configuration reuses the accepted reference. Clearing
source clears its ownership spans without deleting managed assets or old records.
Gain-compatibility edits publish the future job epoch through the same admission
CAS before invalidating jobs; capture cannot pin a stale configuration epoch.

Begin retries at most eight compare/exchanges when idle edits race admission.
Exhaustion reports an explicit retry notice and accepts no truncated pass; it
never waits on a UI lock. Numeric-editor opening, drag, wheel and keyboard
invocation pin ownership through asynchronous delivery in shared controls.
Async imports/choosers retain session/pass/request identities, and rejected
settings do not dirty saved state. Factory loading follows the same boundary:
an absent accepted IR remains absent for that pass, then can load after settlement.

## Display and compatibility

Provisional display is worker-owned metadata, not durable or renderable source:
at most 10 publications/second and at most 4 MiB per published progress snapshot,
with bounded/coarsened real peak bins and explicit incomplete telemetry status.
Display pressure never invents coverage or invalidates valid source admission.

Schema 1 source remains readable. Where historical settings are unknown, migration
marks a deterministic baseline derived from the saved accepted preferences as
legacy, without claiming original pass settings or rewriting existing swell IRs.
No old PCM or assigned render assets are rewritten. A schema-1-only older binary
cannot read schema-2 source; retain pre-upgrade project states and their immutable
manifest references for rollback. Invalid/unsupported context metadata is rejected
transactionally, preserving the previously accepted record and all managed assets.

## Instance imported-IR collection

Creative JSON v9 adds `importedIRs`, storing managed Asset metadata and labels.
The envelope remains v8 and the creative-state limit remains 8 MiB. The table is
bounded to 4097 unique content identities (the legacy maximum default plus swell
assignments). Reject capacity overflow explicitly; never silently evict an import.
Successful, current imports enter the collection together with assignment acceptance.
Unused imports persist; failed/cancelled/superseded requests do not enter it.
Legacy JSON v1-v8 seeds only accepted named references. No directory scan can
reconstruct formerly unused imports. Older binaries reject v9 creative metadata;
keep pre-upgrade project copies for rollback. This is metadata-only: PCM assets,
plugin identifiers, gain and DSP semantics remain unchanged.

## Recovery, evidence and rollback

The saved envelope carries one accepted immutable manifest reference. That manifest
contains both PCM coverage and generation ownership. Never recover one table from
an unrelated newer reference. `CaptureSession::preparePass` writes/validates the
candidate first; `acceptPass` publishes only under the still-current pass/epoch.
Restore, cancellation and write/validation failures keep the old record intact.
Worker UI progress does not enter this reference, creative JSON or source readers.

Executable examples are in `tests/capture_context_test.cpp` and
`tests/capture_transaction_test.cpp`:

- Adjacent captured audio `[0,4)` and exact-zero silence `[4,6)` share one context
  span `[0,6)`; a gap `[6,10)` has no owner. A later `[10,14)` span remains sparse.
- A different accepted context replacing `[2,6)` leaves `[0,2)` under the prior
  context. Additive passes cannot replace either committed audio or silence.
- Identical PCM under a new context advances the accepted manifest reference but
  retains PCM paths, range revisions, audio fingerprint and source dependency.
  It sends no audio-content dirty callback. A repeated identical context reuses
  the accepted reference.
- A legacy baseline is explicitly `origin=legacy`. It is a deterministic
  migration of the saved accepted preferences, not evidence of historical pass
  intent. It never changes existing per-swell IRs, gains or render identities.
- Failed segment/manifest writes, restore during prepared acceptance, and a held
  stale context-provider completion leave the previous source/context pair
  authoritative. An independently restored copy cannot publish into its origin.

Retain the pre-upgrade `.flp` **and** its referenced managed assets/manifests when
rolling back. Do not overwrite the only older project with a schema-2/v9 save.
The plugin does not delete the old immutable records or perform root relocation,
portable packaging or orphan cleanup. Preserve those files when investigating a
failure; removing the cache is not a recovery procedure.

## Candidate preview and generation

Learn reuses `suggestOnsets` through `scanLearnCandidates` over the actual mapped
run union, with source/session/coverage/settings/policy/timing/completed-boundary
identities. Current Learn sensitivity applies to the full accepted arrangement,
including earlier dry-only passes. It does not rewrite pass history. Default IR
is not a detector dependency. Settled snapshots are reused while identities match;
edits coalesce and retain the previous markers marked updating during refresh.
Missing mapped assets reject a settled result rather than publishing a smaller
set that could authorize retirement.

For progressive preview the capture worker publishes only immutable staged chunk
manifests, fenced by pass/epoch. No pending PCM or provisional manifest enters the
accepted source/save path. Candidate detection reads those chunks off callback,
withholds the unsafe trailing analysis window and labels the result provisional.
Stop needs a matching final accepted snapshot. Abort/failure discards provisional
authority and restores the previous settled display when one exists.

Explicit Generate and eligible Auto-generate on Stop call one request API. The
Stop intent is pinned while validation still owns the pass, and is cancelled by
settings/default/source/workflow changes or OFF. ON, restore and mode entry never
generate. Admission freezes the currently displayed final candidates, current
accepted Default IR, source, settings, model and workflow identities. The worker
verifies IR content and every mapped source run, then prepares one replacement
without mutating committed definitions. Protected collisions take priority using
the existing detector tolerance; matched automatic IDs/activity remain stable.
Manual/edited/frozen, unknown-origin and outside-scope definitions remain intact.
An empty/protected-only/unchanged/failed/cancelled plan never retires old work.

A nonempty viable replacement with new/regenerated definitions or eligible retirements can publish the full replacement and
guarded Perform transition. A low/high/low sensitivity loop can therefore remove extra unedited automatic swells while retaining unchanged survivors and their renders. Wet jobs queue afterward only for changed definitions. Last-good immutable render
references remain; accepted source-dependent work is maintained by the shared engine independently of preview/marker policy. Protected manual calibration
also retains its saved reference through automatic collection replacement;
explicit later source/IR/arrangement edits retain their normal editing policy.
Restore rebuilds preview and permitted committed-render recovery, never pending
source-context generation. Retired job identities are pruned within the existing
4096-swell bound, without deleting old immutable files or saved project assets.

New Generate admission supersedes earlier unfrozen automatic wet work through a
cooperative worker cancellation epoch. The serial worker services Generate before
remaining renders. Interrupted accepted jobs are kept eligible for recovery if
the replacement does not commit. Completed protected/last-good render metadata is
rebased into the request before planning when musical revision, session and
selection are unchanged. Render output still notifies persistence, but does not
invalidate musical authority; real edits and workflow/dependency changes retain
their original publication fences.

After complete accepted Recapture, changed existing unfrozen swells rebuild through ordinary material reconciliation, including protected manually edited definitions and detection/marker Generate on Stop off. Saved marker parameters and assignments persist. Provisional/cancelled/rejected source cannot become render authority. Frozen work is held; exact compatible saved wet audio is reused. Explicit Perform Regenerate is still available to force a revision, not required for successful normal recovery. See [shared engine ownership](swell-engine-architecture.md).

Learn accepts completed-pass boundaries through successful source settlement,
including on a fresh instance whose previous empty manifest is still schema 1.
A provisional RT Stop cannot queue legacy reconciliation. Live loop boundaries
and explicitly requested Perform updates retain their existing window policy;
Learn return/restore does not turn capture history into a generation command.

The Learn timeline reads accepted overview peaks and separate pass/epoch-fenced
progress. Mapped duration is the source union, provisional duration counts only
currently admitted input, and extent excludes viewport headroom. Silence is exact
coverage metadata, never an amplitude threshold. The pure `LearnTimelineModel`
owns dry bounds, stable stepped fit, region inspection and honest fallback labels.
Its main waveform uses left-click or drag to pin, pointer-wheel zoom and
middle-drag pan. Manual navigation and the absolute pin survive ordinary
capture updates. Coverage reads use the full captured extent and stay independent
of wet bounds, host seeking and persistent state.
