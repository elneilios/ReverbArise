# Issue #8 — phase-4 working-folder preparation

Date: 2026-11-06. Baseline: merged main `f98157e`; branch
`codex/8-working-folder-association`. Windows native MSVC Release, JUCE 9.0.4,
warm `out/build/windows-msvc` in the retained issue worktree. No cache clearing,
downloads, plugin installation, release or FLP edits.

## Scope and acceptance

The [implementation boundary](../development/working-folder-association.md)
describes the backend and disposable preview. Production capture/render/export
roots and host serialization are unchanged. The owner authorized phase 4, while
the existing native control review remains required before integration.

| Task | Result at this checkpoint |
| --- | --- |
| 4.1 | Verified preparation and revised control composition with binding tests; live processor-root handoff, host-save notification integration remain pending; native control approval is recorded below. |
| 4.2 | New exclusive branch lease on each backend reopen; two real processes hold different leases simultaneously. Actual capture/render/export publication across processes remains pending. |
| 4.3 | Backend physical copy into another store preserves original dependency hashes. Production Fork command and independent future-write validation remain pending. |
| 4.4 | Candidate version/rollback guidance published. Original-binary/pre-upgrade-FLP rollback exercise remains pending. |

All are high-risk persistence/concurrency tasks; 4.1 also includes UI interaction.
No checkbox is completed by partial fixture evidence. Refreshed Apply reports
**20/36 complete, 16 remaining**; the active change and issue stay open.

## Native results

- `scripts/build.ps1 -Configuration Release` passed after adding the preview.
  Log: `out/build-logs/Release/20261106-202612-e4ef0682/`.
- Focused CTest selection `working_folder|portable|storage|shared_swell_engine`:
  **10/10 passed, 18.81 s**. It includes storage settings, backup/recovery,
  exact snapshots, both new working-folder cases and shared render maintenance.
- `preverb_asset_inventory_test`: **passed, 0.21 s** separately.
- Final focused `preverb_portable_assets_test --working-folder` passed after
  adding bounded/hostile candidate-envelope decoding checks. These are compiled
  native tests, not source-only fixtures or FL Studio tests.
- Initial complete wrapper run: **83/86 passed, 367.67 s**, exit 8.
  Log: `out/build-logs/Release/20261106-201902-ccb79e69/`.
  Three failures reproduced in a focused retry, without changing assertions:
  `preverb_perform_knob_reset_test` lacks OS foreground focus;
  `preverb_perform_readability_test` times out at its focus predicate;
  `preverb_swell_engine_test` times out at line 114 waiting for transient stale
  phase after a tempo change. These failures prevent a green integration baseline.
  The swell-engine executable was not rebuilt by phase 4 (18:08:59 timestamp,
  SHA-256 `9AEA8876B19DB4DA0DF0217F8A1A0624F1117403A642B3620230B71F867FEED7`).
  No production engine or existing test assertions changed.

- Final complete wrapper run, including both new tests and final codec guards: **84/87 passed, 366.24 s**. Log: `out/build-logs/Release/20261106-203004-a01623e7/`. The same three failures in existing tests remain unresolved; no required assertion was bypassed. Production integration and merge remain blocked.

The new backend fixture verifies exact dependency bytes, metadata/descriptor
closure, no switch during preparation, stale/unavailable inputs, cancellation,
read-only destinations, injected completion/revalidation faults, wrong-store
rejection, explicit location without local hints, complete physical copy and
two simultaneous child processes with distinct exclusive leases. Candidate
metadata tests cover exact round-trip, mismatched manifest/snapshot, future and
noncanonical versions, XML entities, excessive nesting and unknown payload keys.

Native panel tests verify cancelled/stale picker results, preparation retaining
the original root, later edits invalidating candidates, binding rejection,
explicit Use/save-request wording and pinned action bounds at 420×240/680×420.
The binding is disposable; it is not a production processor or host-save proof.

## Other checks and remaining gates

- UI standards: **passed, 66 files / 59 bounded occurrences**, revision 10;
  no new token, visual exception or regenerated debt inventory.
- `openspec validate portable-project-assets --strict`: passed.
- Semantic user-guide review: existing production instructions remain accurate;
  candidate controls are excluded. Explicit review record refreshed and currency
  check passed. Canonical specs were not synced or archived.
- Final whitespace and changed-document relative-link checks are recorded with
  the PR verification. No associated-store fresh-profile test or new host/DPI
  coverage is claimed. Prior recovery acceptance is preserved.

Native review entry point:
`preverb_portable_storage_preview --working-folder-review`. A copied executable
is launched directly for owner review, using disposable captured/source/IR/wet/
export fixtures and native folder selection. Only one phase-4 review window is
opened and it remains open for the owner. The existing design requires native
approval before production rollout; no approval is inferred from test success.

Future integration must retain independent writer/root pins for active jobs,
verify coherent host-state saves without callback disk I/O, preserve exact old
FLP aliases and test failure-safe root handoff, missing-store recovery, physical
Fork and rollback. No cleanup, eviction, quarantine, compression, dedup adoption,
release or merge approval is granted by this checkpoint.


## Owner native approval — 2026-11-06

The owner explicitly approved the working-folder layout and Choose folder →
verified copy → Use This Folder interaction: “Approve layout and interaction”.
This clears the scoped native control gate, not the remaining production root,
writer, state, recovery, rollback or validation gates. The owner closed the
disposable preview. Both unchanged native-focus tests passed on retry after that
review ended. No foreground-focus assertion was removed or weakened.


### Approval follow-up validation

The three original failures all passed unchanged once after the owner closed
the review window (focused CTest: 3/3, 16.23 s). The engine assertion was still
race-sensitive: it waited for an intermediate stale phase and required manual
Generate even though the implemented common worker immediately maintains changed
material. The fixture now publishes coherent tempo to both provider and callback,
requires both new verified 241-BPM renders without Generate, retains identities,
shape/IR metadata and original byte hashes, and checks a superseded request yields
the current source-bound render while preserving its predecessor. The original
single-target selection before later removal is retained.

The two focus fixtures now use a titled native DocumentWindow owner that the
desktop runner can discover. The knob fixture allows up to 30 seconds for the
foreground precondition, matching the existing precision fixture; actual OS/JUCE
focus, popup dispatch, exact-value and notification assertions remain mandatory.
The titled knob fixture passed in 11.49 s, precision fixture in 8.17 s; no
successful external activation was needed in those runs. Desktop activation was
attempted only after observing one returned native window; that test had already
completed by the subsequent lookup, so no injected focus success is claimed.

Supported complete wrapper: **86/87 passed, 310.60 s**, native Release build
passed; `out/build-logs/Release/20261106-212351-92d99eed/`. The sole failure was
the knob fixture's OS foreground precondition. Its subsequent focused CTest run
passed **1/1, 2.32 s** (`out/phase4-approved-knob-final.log`). All 87 cases thus
have passing native results across these runs; **no uninterrupted 87/87 wrapper
pass is claimed**. A stable complete native run remains required before cutover.

UI standards (66 files/59 bounded occurrences), guide currency, strict active
OpenSpec validation, eight changed-document relative links and whitespace pass.
No production DSP, capture, editor, root binding or serializer changed in this
follow-up. No new FL Studio/profile, release, merge or cleanup operation occurred.

The recovery-workspace requirements question in the development guide remains
open. Layout approval is recorded; no exception to complete-copy association is
implemented or inferred. Tasks stay 20/36 complete and PR #112 remains a draft.


## Approved explicit recovery workspace and location ownership

The owner approved an explicit recovery workspace after the whole original store
is lost/offline. This clears the requirements question above; it does not approve
an incomplete pack, silent fallback, destructive cleanup or production cutover.
The active proposal/design/specs/tasks record the exception. All phase-4 tasks
remain unchecked (20/36 overall).

Native coverage now verifies original locators/metadata, hash-verified surviving
assets, known missing capture metadata, unknown source coverage, ordinary complete
Learn capture, stale/cancel/read-only/disk-full/fault guards and no complete-pack
receipt. The candidate v9 codec round-trips bounded recovery locators and rejects
unsupported origin versions without replacing accepted output. Production still
reads 1–8/writes v8; no Settings or host association binding is claimed.

Capture source snapshots retain immutable working-root/lease ownership; native
tests cover failed handoff rollback, superseded resolution, retained old readers
and blocked writes when unavailable. Engine/offline/export jobs use the source
location, defer while Loading, reject unavailable storage and retain export
history. The location test checks unchanged material causes no new convolution,
new imports/exports use the new root, managed export-save boundaries remain
enforced, originals are unchanged and the callback allocates/frees nothing.
Actual independent processor writer publication remains a later gate.

The native NTFS probe rejected junction conversion for nonempty owned categories.
Pinned administrative `.directory-pin` files retain that invariant while ordinary
immutable capture publication succeeds. A trial that denied directory write
sharing prevented normal MoveFileEx publication (error 32); it was discarded,
and existing immutable publishers remain unchanged. No original/user folders
were deleted or broadly write-locked.

Focused native CTest: 5/5 passed in 30.46 s (capture, engine, export, working
folder and shared engine). After adding recovery-locator serialization the native
working-folder/recovery cases also passed. The first broader wrapper built
successfully but passed 86/87 in 242.57 s: recovery admission failed because the
engine initially held a different location object from the source worker. The
engine now initializes from the source frame before starting its worker; strict
coherent-location checks remain intact. Log: `out/build-logs/Release/20261106-235105-e60ba834/`.

The initialization fix passed ordinary missing-asset recovery. Its full wrapper passed 86/87 in 248.17 s; the only failure was a transient rename of the newly written disposable offline-store fixture (out/build-logs/Release/20261106-235621-abf3575a/). That fixture now retries for at most five seconds and still requires the original directory to be genuinely offline. Final supported wrapper: native Release build passed; 85/87 tests passed in 306.08 s (out/build-logs/Release/20261107-000136-74c3a060/), including working-folder and ordinary recovery. The two failures were Perform preview setup waits (knob reset and readability); both passed the immediate focused retry, 2/2 in 3.65 s (out/phase4-recovery-preview-retry.log). All cases have passing native results across the current runs; no uninterrupted 87/87 pass is claimed. Stable complete-suite validation remains a rollout gate.

UI standards passed (66 files/59 bounded occurrences), strict active OpenSpec
validation passed, changed Markdown relative links and whitespace passed. The
maintain-user-guide review confirms no prose change is appropriate while the
controls/host binding remain unintegrated; currency is explicitly refreshed and
checked. No FL Studio/profile installation, original-binary rollback, release,
merge, archive, cleanup or issue closure occurred. PR #112 remains draft.