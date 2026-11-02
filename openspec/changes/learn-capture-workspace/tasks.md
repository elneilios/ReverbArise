# Tasks

These are implementation tasks, not work completed by planning. Keep every box
unchecked until its behavior and stated verification pass. All edits stay on an
issue #11 branch; do not add this work to PR #18 or duplicate #16's rename.

## 1. Reconcile prerequisites and pin the baseline

- [ ] 1.1 Reconcile the reviewed/merged #10 baseline and #16's separate display-label mapping before implementation; verify clean issue-specific branch history, available Learn/Perform/Export mapping and unchanged persisted/internal identities, documenting any unmet dependency instead of silently implementing it here.
- [ ] 1.2 Record exact baseline source/binary/asset identities, state/source schema versions and supported sizes; run focused workflow/capture/default-IR/automatic/UI/state/transport tests and retain representative legacy/current/frozen/custom-IR fixtures with their actual results.

## 2. Accepted future defaults and pass-lock model

- [ ] 2.1 Extend the existing default-target importer/catalogue commands to accept real bundled IDs and managed WAV/WavPack defaults with requested-versus-accepted status; test latest-request-wins, cancel/invalid import, original-file removal, session supersession and unchanged existing assignments, and document the shared importer path.
- [ ] 2.2 Add an immutable off-callback generation-configuration snapshot and bounded pass-admission pin/lock; test concurrent Play/edit ordering, lock through validation, stale chooser/import/knob/typed completions and unlock after settlement without callback allocation/locks or half-updated settings.
- [ ] 2.3 Bind effective Automatic Swells and canonical sensitivity/default preferences to saved state and coalesced host notifications; test legacy master/marker combinations, hidden-value preservation, no-op/transient/rejected silence and accepted-setting save notifications, updating current settings/migration guidance.

## 3. Learn composition and first visual checkpoint

- [ ] 3.1 Add the dedicated mode-specific left-settings/right-timeline layout with shared header/theme and independent viewport state; native geometry tests must show no Swells list, selected inspector, audition strip or duplicate Overview in Learn and unchanged Perform regions after returning at 1100x850 and supported larger sizes.
- [ ] 3.2 Bind real Default IR/Open from File, separate Recapture/Automatic controls and horizontal sensitivity plus exact percentage; verify on/off hiding, locked-pass input rejection, accepted/pending/error labels, keyboard/numeric synchronization and no unimplemented Character controls in native UI tests.
- [ ] 3.3 Create native empty/on/off/Recapture-warning/default-picker snapshots at minimum and larger sizes and obtain user approval at visual checkpoint A before further visual implementation; record approval/adjustments separately from unfinished functional acceptance and update current layout guidance.

## 4. Coherent source and pass-context persistence

- [ ] 4.1 Define/version the immutable accepted source/context commit metadata and explicit bounds before wiring it; serialization tests must cover old source/state formats, finite enums/controls, safe references, absent/ambiguous context, capacity failures and deterministic legacy baselines without rewriting PCM or existing swell assignments.
- [ ] 4.2 Integrate pinned configuration into staged pass ownership and one atomic Stop acceptance/save snapshot; exact interval fixtures must cover additive audio/silence, first/last-received overlap, sparse seeks, context boundaries and identical/context-only updates without changing audio fingerprints or unnecessary renders.
- [ ] 4.3 Add transaction fault/race fixtures around source/context write, validation and acceptance; verify failed/cancelled/stale jobs leave the prior coherent record intact and cannot publish into a restored or copied instance, with no sole-asset deletion.
- [ ] 4.4 Persist selected defaults and accepted context references and restore deferred work; test pass A/IR A then default/pass B/IR B with reversed job completion and reopen before A generates, plus custom-file removal and independent FX copies, retaining exact old gain/IR/render identities.
- [ ] 4.5 Update capture/storage/session guidance with committed-versus-provisional data, new metadata versions/bounds, legacy unknown-context behavior and rollback copies; verify examples against the fixtures and keep root relocation/portable cleanup outside this change.

## 5. Deterministic generation and dry-only policy

- [ ] 5.1 Resolve new automatic assignments by the context owning each arrival rather than the current mutable default; test confirmation/extraction across context boundaries, out-of-order passes, new manual targets using the latest accepted default and preservation of matched/manual/edited/frozen/suppressed targets.
- [ ] 5.2 Enforce dry-only learning across master/marker/legacy auto-refresh/forced-Recapture paths; use actual analysis/render counters and output oracles for all four Recapture/Automatic combinations, including pre-existing queued/stale swells, useful automatic mode entry and no disabled-pass work starting merely on Perform entry.
- [ ] 5.3 Preserve explicit permitted Perform creation/reanalysis/edit behavior while distinguishing it from deferred pass work; test rapid reanalysis/off/on supersession, stable IR ownership and unchanged dependencies/calibration rather than blanket stopping manual convolution or implicitly recapturing dry.
- [ ] 5.4 Update automatic-processing guidance to explain per-pass defaults, future-only Learn settings, existing Perform actions and off-policy scheduling; verify every documented distinction against the automatic/model/processor fixtures.

## 6. Progressive source display, fit and second visual checkpoint

- [ ] 6.1 Add bounded worker progress summaries for real admitted pending/staged source, keyed by pass/session ownership and separate from committed readers; test feedback before storage batching/Stop, exact additive/Recapture admission, cancel/failure/restore discard and editor-closed operation with callback allocation guards and measured metadata bounds.
- [ ] 6.2 Draw actual dry peaks and distinct labelled audio/silence/gap/unavailable/provisional states with range inspection, accessible alternatives and separate mapped/provisional/extent totals; native pixel/coordinate/model tests must reject fabricated quiet-tail classification, gap filling and stale overlays.
- [ ] 6.3 Add independent origin-anchored threshold/coalesced full-dry auto-fit and truthful ruler/playhead fallback; test bars 86-90 then 1-4, repeated same-extent Recapture, backwards loops, unavailable timing, retained negative source and large sparse spans without jitter, wet-bound influence or host seeking.
- [ ] 6.4 Verify UI lifecycle/notification behavior through mode changes, progress, imports, accepted commits and rejected edits; tests must show independent Perform navigation/selection, no transient dirty notifications, no callback GUI/storage work and preserved saved live/audition preferences, updating coverage/fit guidance.
- [ ] 6.5 Deliver native recording/settling/cancelled/error and mixed-coverage snapshots plus unchanged Perform-return snapshots, then obtain user approval at visual checkpoint B; record actual readable coverage distinctions, supported-size results and any unperformed host/DPI checks.

## 7. Consolidated integration and acceptance

- [ ] 7.1 Run the full configured Debug build/CTest and relevant Release capture/context/migration/default-IR/automatic/UI/gain/streaming/transport/export regressions; record commands, actual counts, source/binary/schema/asset identities, metadata/PCM memory limits and realtime guards, preserving independent sample oracles.
- [ ] 7.2 Present one consolidated FL Studio checklist for the approved Learn layout, on/off dry learning and Recapture, default/custom IR choice with deferred work and reopen, truthful sparse/silent/provisional coverage, automatic Perform entry and return to the existing editor; record exact build, available host configuration, actual passes/failures and supported DPI checks without inheriting #10's host waivers.
- [ ] 7.3 Review every issue #11 criterion against spec/test/visual/host evidence, confirm #16/#5/#6/#8/#9/#1 boundaries, run strict all-spec validation and diff checks, and leave unresolved acceptance unchecked; deliver a concise evidence crosswalk and any explicit owner-approved validation exceptions without inventing host passes.

## Workflow follow-up

- Planning only is authorized here. Stop after presenting these artifacts; implementation starts with a new apply request.
- Read actual mockups from issue #11 comment 6578923411 and the Capture IR reference in issue #9 comment 6579018037; #16's terminology supersedes their old labels, not their approved layout.
- Keep visual checkpoint approvals separate from implementation checkboxes and compilation. Use grouped manual review, not a long serial list of micro-checks.
- Update task checkboxes from refreshed CLI locations as work passes; do not mark baseline or implementation work done merely because planning inspected code.
- Reconcile the planning branch with merged prerequisites before committing implementation, then use one issue #11 PR for owner review. No automatic commit, push, merge or GitHub issue mutation is authorized by this planning request.
- Sync/archive only after implementation and required evidence are complete; do not modify the archived #10 change or canonical specs during planning.
