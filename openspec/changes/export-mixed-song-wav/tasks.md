# Tasks

## 1. Composition identity, preferences and mode prerequisites

- [x] 1.1 Add immutable Export plan/request types and dependency identity covering source/timing, eligible accepted wet/render/shape/placement/gain versions, activity, percentage and warnings; verify deterministic identity and selection/isolation independence with pure fixtures, documenting the contract in `docs/development/song-export.md`.
- [x] 1.2 Add validated versioned Export preferences and completed-reference metadata to creative state; test v9 migration, effective Wet Only/65% first-use initialization, independent mode roundtrips, invalid/nonfinite/unsafe metadata rejection and preservation of source/context/import/render IDs. Document exact schema/version and rollback limits alongside the implementation.
- [x] 1.3 Separate compatible wet-based Export entry from dry and current-file readiness in model/processor guards; verify dry-only cannot enter, frozen-wet missing-dry recovery can, 0% stays in Export, lock/restore paths remain guarded and useful capture Stop still selects Perform.
- [x] 1.4 Implement independent instance/session/request ownership, cancellation and coalesced host-save notifications for real Export edits/references; test restore/copy during preparation, no-op/transient events, stale completion and editor-closed publication. Update the request/lifecycle guidance with the tested authority rules.

## 2. Bounded deterministic composition and gain

- [x] 2.1 Implement accepted-version membership and pinned asset/source readers using existing schedule/window mechanisms; verify all eligible noncontiguous/overlapping swells contribute once, accepted IR replacement stays fixed, required unavailable wet blocks readiness and disabled/muted/solo-excluded members are absent without changing host-bounce policy.
- [x] 2.2 Add the separate warned-silence dry reader policy for Export; verify exact captured silence, gaps, unavailable/corrupt dry, wholly absent dry and 100% dry-bypass behavior with range/sample oracles. Preserve strict existing Perform reader behavior and document visible substitution semantics.
- [x] 2.3 Implement one page-independent stereo mixing kernel with saved calibration/Level/envelopes, true-boundary wet ramps, linear dry/wet weights and fixed whole-revision protection measurements; compare independent 0/50/65/100% and correlated/antiphase overlap sample oracles, exposing measured attenuation without upward normalization.
- [x] 2.4 Implement verified origin/range mapping, negative clipping and exact audible-bound analysis independent of viewer position; test nonzero host origin, bar-9 first event, sparse gaps, quiet tails, all-zero mixes, complete dry/wet tails, incompatible timing and checked overflow without trimming song-origin WAV lead-in.
- [x] 2.5 Implement bounded/coalesced worker preparation, peak summaries and cancellation under the existing shared audio budget; measure live/preview/write coexistence, long/sparse spans and maximum overlap, asserting stable output across page sizes and no whole-song PCM retention. Record actual memory limits and kernel/gain semantics in development guidance.

- [x] 2.6 Add bounded ownership-checked viewport-layer summaries reusing the accepted measured mixer; account retained previous/current summaries and reader cache under the shared budget, coalesce rapid view requests, preserve preview prefetch and prove unchanged file/gain/render identities and stale-detail rejection.

## 3. Verified WAV publication and durable delivery

- [x] 3.1 Stream stereo float32 WAV at verified composition rate with checked RIFF bounds; decode produced files and compare full sample/frame/channel/rate/alignment oracles, including lead-in zeros, negative pre-roll, page seams, complete tails and finite/headroom behavior. Document supported format and explicit size failures.
- [x] 3.2 Publish unique mix-labelled managed files only after close/verification and atomic completion; inject short write, disk/close/verification/rename/cancel failures and verify prior files/references remain unchanged and no partial path becomes Ready. Test deduplication only for independently verified identical content.
- [x] 3.3 Persist accepted composition/desired percentage separately from last completed revision and preserve immutable file history; test save mid-job, reopen validation/rebuild-to-new-file, missing inputs, corrupted references and independent instance copies. Verify edits/Clear/mode changes cannot delete previously exported Playlist assets and update persistence guidance.
- [x] 3.4 Implement shared safely owned Save chooser invocation and transactional copy/explicit-overwrite handling; test cancellation, existing destination, stale chooser after restore/mode change, failed destination publication and preservation of the managed original.
- [x] 3.5 Implement native external-file drag invocation against only the verified current WAV, with file lifetime independent of drag completion; test pending/stale/missing paths, unsupported/cancelled drag, failed invocation and always-available valid Save fallback. Record that component invocation tests do not establish real FL Studio Playlist acceptance.

## 4. Explicit Export preview with shared composition

- [x] 4.1 Add bounded worker-prefetched playback of the same prepared composition, starting at exact audible bounds with absolute cursor reporting; compare captured callback contributions with decoded WAV interior samples at all reference mixes and isolate only documented monitor-boundary ramps.
- [x] 4.2 Implement explicit Play/Stop intent, safe same-position mix-revision handoff or truthful stop/rebuffer, starvation handling and non-realtime buffer retirement; verify Stop supersedes delayed completion, mode exit/restore never autoplays and page boundaries do not retrigger ramps or skip audio.
- [x] 4.3 Integrate Export preview under dry-passthrough/no-capture/no-scheduled-wet authority and independent monitoring preferences; use processor oracles for running/stopped/offline host transport, two instances, wet-only live preference, exit/return to Perform and excluded generated-source feedback.
- [x] 4.4 Document preview/file equivalence, protection/monitor differences, audible-start versus file-origin semantics and future #5 reuse in `docs/development/song-export.md`; verify examples against sample oracles without changing Perform's existing selected-window audition.

## 5. Dedicated Export workspace and shared controls

- [x] 5.1 Use the two owner-supplied issue #6 mockups reviewed in chat on 2026-11-04 and inventory existing shared controls before integration; map their dedicated layout and 0/50/100% comparison onto current Inter/spacing tokens, obeying wet-only endpoint visibility and preserving the approved header/other workspaces. Reference retrieval is resolved; control inventory and implementation mapping remain to verify. Resolve any material visual conflict with the owner before implementation proceeds through that checkpoint.
- [x] 5.2 Reuse approved SliderControl/NumericInputControl and extract missing command/drag-tile interaction families with domain-independent callbacks; test wheel/typed/stepper parity, keyboard/focus/disabled behavior, invocation ownership, font faces and clipping at minimum/larger sizes. Update shared-control guidance and directly launch a native family preview for owner checkpoint A before wider integration.
- [x] 5.3 Refine the approved first-pass layout with a proportionate roughly 4:3 WAV tile, cohesive icon/text, compact colour-keyed layer legend and existing spacing; retain and verify the integrated the approximately 66/33 waveform-to-controls Export view with Play/Stop, exact mix, Drag WAV, Save WAV and Return to Perform; verify region geometry, keyboard order, no modal/summary/duplicate Overview and unchanged Learn/Perform/header snapshots at 1100x850 and supported larger sizes.
- [x] 5.4 Refine complete audible fitting with unclamped visual margins, explicit audible Preview versus song-aligned WAV duration labels and full-tail fixtures; retain and verify drawing of cached actual dry/wet layers using one disclosed amplitude scale and composition gain plan, with ruler, coverage and distinct preview cursor; verify asymmetric rising pre-arrival wet, 0/50/100% visibility/scaling, real post-arrival tails, actual audible fit, all-zero state and original song-position labels using model/pixel/native fixtures.
- [x] 5.5 Retain and label a dimmed last-good waveform through pending/error with immediate requested percentage and atomic matching replacement; bind stable pending/ready/error/file states and persistent gap/bypass/layer/replacement guidance through existing shared notices; test that rapid mix changes cannot flash unreadable helpers, shift layout, enable stale actions or hide actionable failures, and that no fake WAV readiness is exposed.
- [x] 5.6 Reopen checkpoint B for revised tile/legend/navigation/last-good/duration presentation; deliver native integrated reference-mix/pending/error/missing-dry snapshots and directly launch the preview for owner checkpoint B; obtain explicit composition/control-feel approval, retain unchanged component/header approvals, record supported-size/raster results separately from untested OS-DPI/host coverage and update `docs/design/export-workspace.md` plus documentation links.

- [x] 5.7 Extract and test shared read-only Fit/pointer-zoom/pan/keyboard navigation over existing SwellViewport; verify numerical bounds, ruler/cursor mapping, manual viewport preservation, once-per-accepted-mix fit and ephemeral save-neutral behaviour, then obtain family look/feel approval in revised checkpoint B before wider reuse.

## 6. Consolidated integration and delivery evidence

- [x] 6.1 Run final affected Debug/Release validation through `scripts/build.ps1 -Configuration Debug|Release -Test` after implementation settles; record actual native test results, callback allocation/lock/I/O guards, memory bounds, source/binary/schema/asset identities and remaining limitations in dated `docs/validation/` evidence. Repeat unchanged full suites only for new failures/inputs/behavior.
- [x] 6.2 Obtain one grouped final FL Studio acceptance: preview versus saved 0/50/100% composition, real ready-WAV drag into Playlist at song origin with cancel/unsupported Save fallback, Export bypass and replacement-source muting guidance, and save/reopen of percentage/revision. Record actual host version/rate/buffer/DPI and untested parts without repeating automated edge cases absent concrete host issues.
- [ ] 6.3 Cross-check every #6 criterion against delta/model/sample/file/component/visual/host evidence; confirm #5/#8/#38 and concurrent header/performance plans remain separate, Learn Stop→Perform and captured provenance remain unchanged, run strict all-spec validation and diff checks, and leave any unresolved acceptance unchecked.

## 7. Internal preview positioning

- [x] 7.1 Implement ephemeral ready-revision click/drag seek authority over the existing paged preview; test stopped parking, seeked PCM/ramp/cursor, playing/latest-intent handoff, Stop/mix/mode/restore cancellation and unchanged composition/file/state identities.
- [ ] 7.2 Bind left-click/drag preview positioning with Shift-drag pan, unchanged wheel/Fit and immediate cursor feedback; test viewport mapping, right-click/pending rejection and native interaction feel, preserving prior visual/host passes and deferred #46 control integration.

## Workflow follow-up

### Owner-approved deferred integration (2026-11-04)

Current Export visual acceptance passes as presented. In coordination with Agent
2 / issue #46, integrate the fuller absolute musical ruler and shared Zoom − /
slider / Zoom + / Fit controls later; no Snap/grid or editing handles. Preserve
Export's complete-audible Fit, existing gestures, mix/readiness and WAV alignment.
Do not independently refactor waveform infrastructure. Interface requirements
are recorded in `docs/design/export-workspace.md`. These refinements do not reopen
the current visual checkpoint. Merge and archive remain held until integration
is completed or explicitly separated by the owner into a follow-up issue.

The owner subsequently passed all three FL Studio groups and authorized preview
click/drag positioning now under section 7, with Shift-drag preserving pan.
Shared #46 controls/ruler remain deferred for coordination. Preview seeking is
separate from DAW seeking and audio edits; validate it without repeating prior
host acceptance of unchanged export/delivery behavior.

- This is the authorized planning phase only; stop after presenting the proposal. A subsequent Apply request is required before implementation.
- Continue on the Issue #6 branch, maintain exact task-line/post-checkbox verification through vanilla OpenSpec Apply, and use one review PR for this coherent issue after implementation authorization.
- Keep #11's already synced archive intact. Sync/archive this change only after implementation/acceptance and explicit authorization; do not auto-merge or close issues.
