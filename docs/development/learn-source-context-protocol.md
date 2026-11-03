# Learn source/context protocol — implementation contract

Issue #11 implementation starts from merged source `dc003a7` / main `653f13a`.
The following protocol is pinned before code changes; implementation and tests
record which parts are delivered. Context-codec and atomic pass publication are
implemented, including source-owned deferred work and dry-only policy consumers.
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

## Deferred consumer and display

`SwellEngine` recovers bounded pending On spans from accepted source provenance,
with separate admission from explicit Perform reanalysis. Each worker cell clips
its reconciliation body to one owning span, but analysis history/extraction may
cross boundaries. Settings/IR ownership uses the arrival context. Old matched
assignments are preserved; absent accepted context/IR is an error, not permission
to use a later default. Publication checks source/session and context ownership.
Retired target request identities are pruned within the 4096-swell bound.

The Learn timeline reads accepted overview peaks and separate pass/epoch-fenced
progress. Mapped duration is the source union, provisional duration counts only
currently admitted input, and extent excludes viewport headroom. Silence is exact
coverage metadata, never an amplitude threshold. The pure `LearnTimelineModel`
owns dry bounds, stable stepped fit, region inspection and honest fallback labels;
it has no reference to wet bounds, host seeking or persistent state.
