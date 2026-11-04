# Design

## Context

See proposal.md for motivation and issue scope. Baseline main is `9b8e70f`, with #11 merged. `AutomaticCoordinator.cpp::automaticStep` currently runs `suggestOnsets` and immediately calls `reconcileTargets`, marks persistent changes and queues pass rendering. `WorkflowState::complete` independently changes useful accepted captures to Live. These are the two authorities that must change together.

`AutomaticTargets` already copies the session, uses deterministic one-to-one matching and protects manual/frozen targets, but permits empty retirement and adjusts automatic windows across the collection. Reuse its mechanisms with explicit transaction scope and outcome guards. `Session` already persists automatic/manualCreated/manualEdited provenance (v2 onward), frozen (v6 onward), detector anchors and deletion suppression. Version-1 decoding conservatively maps origin to manualEdited. Creative metadata is v10; processor envelope is v8. No new inference from names or positions is needed.

Learn uses shared Switch, SliderControl/NumericInputControl, IrSelector and IconActionButton, an existing source timeline model and ephemeral inspection. ActionButton already supplies a primary visual role. Focused tests include automatic targets/coordinator, generation configuration, capture transactions/context, workflow state/processor and Learn timeline/editor cases.

## Goals / Non-Goals

**Goals:** Separate detection from mutation, define one safe Generate admission/publication authority, and preserve recoverable creative/source history. Make refinement usable for dense and noncontiguous songs.

**Non-Goals:** Do not replace onset DSP, the capture admission model, render engine, timeline navigation or whole UI library. Do not implement adjacent open issues or modify generated OpenSpec workflows.

## Decisions

### Candidate snapshots are read-only worker products

Extract the existing onset scan/cache path from automatic reconciliation. Publish an immutable bounded candidate snapshot keyed by instance/session epoch, accepted source/coverage revision, detector policy, canonical sensitivity/separation and coherent fixed timing. Record actual eligible mapped runs, completed-pass boundaries, candidate positions and final/provisional status. Progressive analysis uses safely published capture data; never access mutable callback buffers or duplicate whole-song PCM. Keep the previous displayed snapshot with Updating/stale metadata while latest work runs, including cancellation. No candidate refresh writes assets, definitions, persistent change notifications or convolution requests.

Provisional snapshots carry pass/display authority and can never satisfy Generate. Stop acceptance schedules a final full-arrangement snapshot; failure discards provisional authority and retains prior accepted data. Current Learn settings apply across all accepted mapped coverage, including earlier dry-only passes. Accepted pass contexts remain immutable recording history, not an authority to silently generate. This explicitly replaces the old deferred pass-generation journal behavior. Existing supported Perform analysis/edit commands remain separate; returning to Learn itself cannot reconcile definitions. Reuse detector implementations and bounded caches rather than introducing a second detector.

Default IR is not a detector dependency. Changing it cancels pending generation and updates generation readiness but reuses settled marker positions. Host timing changes invalidate analysis/generation; unknown or incompatible timing is unavailable, never guessed. Coalesce rapid sensitivity edits and check identity again at publication.

### One generation request and replacement planner

Both entry paths call one request API with a trigger origin. Freeze the currently displayed final snapshot plus accepted source/coverage, session/model revision, current accepted Default IR, automatic policy, timing and workflow token. Verify IR/source outside realtime processing. No pending import can substitute for the accepted selected IR. The explicit path requires idle Learn; source validation, stale/provisional analysis, capture, another request or unavailable dependencies return an accessible reason. The Stop path pins these same references when the useful source commit is accepted, waits only for its matching final analysis and cancels on any later incompatible edit.

Prepare a complete replacement session off the audio thread. Scope is the union of committed mapped runs; unmapped gaps are never a bounding-box deletion domain. Candidates require real covered extraction windows under existing completed-loop/tail rules. Incomplete candidates do not authorize retirement of corresponding pending/out-of-scope work. Preserve outside-eligible-scope automatic definitions, manualCreated/manualEdited, frozen and conservatively classified legacy work byte-for-byte. Preserve deletion suppression.

Reuse `targetTolerance(rate, separation)` (minimum of analysis-window samples and half the selected separation, at least one sample). Deduplicate candidates by absolute sample. Protected targets reserve collision neighborhoods first, using both actual arrival and retained detector anchor where relevant; no unprotected match consumes their priority. Then deterministically match eligible automatic definitions one-to-one by distance, sample and ordinal ID. Retain matched IDs, names, selection and enabled/mute/solo state; use the newly frozen default IR and recomputed automatic source dependencies. Already-disabled automatic definitions remain eligible, retaining disabled state. New automatic definitions use shared creation defaults. Manual edits continue to set manualEdited provenance, including changes through every editor/diagnostic path.

Compare durable definitions/dependency identities, excluding progress, transient generation counters and render phase. If no viable definition is new or genuinely regenerated, return empty/protected-only/no-change without any mutation. Retirement alone cannot qualify as success. If a changed eligible result exists, remove superseded eligible unfrozen automatic definitions only in that same commit. Bound capacity, matching edges, source scans and preparation; overflow fails the whole request without truncation. Preserve last-good render references until replacement audio is verified; no asset purge occurs.

Blindly calling current `reconcileTargets` once per run is rejected: it could publish a partial arrangement, retire on an empty result or alter outside-scope windows. Refactor the existing planner to prepare all runs in one candidate copy with protected-first matching and an explicit outcome.

### Commit and mode entry share a guarded linearization point

Source settlement ends in idle Learn and disarms successfully consumed Recapture as before. Keep numeric mode values and capture phase encoding. Analysis/generation state is worker-owned; do not add persisted workflow substates for UI copy. Add a bounded generation reservation/token to the workflow authority: Play, mode changes, restore/cancel and setting edits must either win before admission or invalidate preparation before publication. At the final short control-thread commit, recheck all pinned identities, reserve/validate the exact workflow token and atomically publish the complete session with accepted Perform entry. A failed CAS must not leave a published replacement or falsely entered Perform. Do not hold a blocking lock in processBlock or publish definitions then attempt an unchecked mode change.

Wet work queues after successful definition publication, using existing stale-result fencing and immutable asset reuse. Perform may show Queued/Rendering; success never implies all wet is ready. Manual navigation remains governed by existing source/frozen-wet/export readiness, including while preview analysis is pending. It invalidates pending generation rather than acting as a wizard lock.

Auto-generate is an independent per-project preference. ON only admits a request after an accepted useful Stop with effective Automatic Swells enabled. Toggling ON, restoring, sensitivity/default changes and returning to Learn never trigger generation. OFF cancels a not-yet-committed Stop request even while it awaits analysis; allow this cancellation path through any UI lock. Duplicate Stop cannot enqueue duplicate generation. Empty/failed/cancelled/no-change outcomes stay Learn and preserve accepted swells; generation failure cannot roll back an already accepted source commit.

### Persistence and compatibility

Add a strict boolean autoGenerateOnStop in creative metadata v11 (reassess next free version if main advances). Fresh plugin initialization explicitly sets OFF; decoding supported older state with no field sets ON. A v11 missing/malformed field rejects restore transactionally. Preserve explicit OFF/ON, hidden automatic settings, all numeric provenance values, source manifest v2 and processor envelope v8. Version-1 swells stay manualEdited; unsupported malformed origins remain a strict decode error rather than becoming automatic. A valid persisted automatic origin is trusted; no heuristic reclassification.

Persist only accepted definitions/source and preference, never candidate authority, Stop requests or prepared transactions. Restore rebuilds preview from accepted source and current settings but does not resume generation. Remove old recovery paths that would treat accepted source contexts as commands to create new definitions on reopen. Existing render recovery stays permitted under its current contracts.

### UI reuse and approval

Keep Learn's 304-pixel settings column and dominant dry timeline. Place Auto-generate on Stop before Marker Sensitivity so Generate Swells sits immediately below sensitivity (and any genuinely implemented conditional options). Compose existing ActionButton primary role, Switch, SliderControl/NumericInputControl, IR controls, helper typography and reserved NoticeBar. The helper directly under Generate reads: “Want to place swells yourself? Add manual swells in Perform.” It wraps with the existing helper line role and remains legible while Generate is disabled. Automatic Swells OFF hides generation controls, preserves values and leaves dry capture/inspection usable.

Readiness comes from one model snapshot with accessible idle/analysing/capturing/committing/generating/unavailable reasons. Marker count/status belongs to the existing Learn status area; actionable unsuccessful outcomes use NoticeBar. Button and all secondary command routes enforce the same model checks; refresh issues no edit. Candidate overlay uses named timing yellow with triangular heads/dashed stems, white inspection pin, solid yellow DAW cursor and distinct provisional/stale styling plus text. Coordinates remain sample-derived across origin, negative pre-roll, gaps, resize and scale; visible overlay work is bounded by display resolution.

No new control family or token is planned. Candidate-marker visual treatment requires scoped owner native review before production rollout; if implementation needs a new token/variant, present rationale, consumers and regression evidence and obtain approval first, then update production standards revision, code and tests together. Launch one prepared native preview directly, preserve its window through review and include focus, keyboard, disabled reasons, marker distinctions and minimum-size helper wrapping. Open #46/#48 and waveform optimization may change shared seams; integrate accepted APIs without duplicating controls or adopting unapproved density.

## Risks / Trade-offs

- [Cross-thread generation and capture race] → test deterministic held workers and CAS losses at each boundary; successful publication and mode entry must agree, with no callback locks/allocation/I/O.
- [Legacy automatic work resumes on restore] → explicitly remove context-as-command recovery and test save/reopen during all jobs while retaining accepted definitions/render recovery.
- [61-plus candidates make preview unusable] → measure dense realistic full-song fixtures and sparse multi-run capture, inspect usefulness at several sensitivities and record scan/overlay/refresh costs; avoid claiming an algorithm defect or inventing performance gains.
- [Identity-preserving regeneration hides real changes or fabricates success] → compare canonical definitions and dependency fingerprints; test changed IR, changed source, same settings, protected collisions and removal-only outcomes.
- [Source recapture invalidates older wet] → retain immutable last-good assets/references while reporting stale/incompatible dependencies honestly; never falsely call an old render current.
- [Old binaries cannot read v11] → preserve earlier project/cache copies; no destructive asset migration or down-save claim.

## Migration Plan

After proposal review and a new Apply request, implement codec/default tests, workflow reservation and candidate isolation before wiring mutation/UI. Validate each layer with focused tests, then native Debug and Release suites, UI standards lint and strict OpenSpec validation. Update living workflow, Learn and source-context guidance and dated validation evidence as implemented; canonical spec sync/archive remains a separate lifecycle operation.

Obtain scoped native marker/control acceptance, then perform one brief FL Studio smoke covering default-OFF capture/refinement, repeated Generate/protected work, opt-in Stop, mode return and save/reopen. Record exact host/build and untested Windows DPI/screen-reader coverage. Open one owner-review PR, leave unmerged. Rollback uses the previous binary and its separately retained pre-v11 project/cache snapshots; never delete shared immutable revisions.
