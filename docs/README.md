# ReverbArise documentation

This directory covers product identity, the existing implementation, development guidance and historical records.

| Location | Purpose | Maintenance |
| --- | --- | --- |
| [Product Vision](product-vision.md) | Concise target product identity and creative workflow | Replace for major direction changes; archive the previous edition |
| [Development](development/) | Build, DSP, capture, persistence and transport technical guides | Update in place; Git records revisions |
| [Design](design/) | UI system, current implemented layouts and diagnostics guidance | Update in place as work is implemented |
| [Validation](validation/) | Test evidence, calibration files, benchmark metrics and JSON reports | Preserve test scope, original measurements and actual results |
| [Archive](archive/) | Superseded visions, handoffs and early visual concepts | Historical context only |

## Recent engineering references

- [Shared command buttons — issue #107](validation/2026-11-06-issue-107-buttons.md) — owner-approved native family revision 4, explicit command roles and independent aqua corner focus; production migration, automated checks and integrated acceptance are recorded separately. Knobs and independently owned #108/#8 implementation remain outside this migration.
- [Shared selectors — issue #107](validation/2026-11-07-issue-107-selectors.md) — approved quiet inspection/explicit commit, independent aqua focus and small local help/red error feedback; named consumer adoption and native verification. Action menus remain a separate child.
- [Shared field feedback — issue #107](validation/2026-11-07-issue-107-field-feedback.md) — native approval and the Settings Swell source-range adoption.

- [UI control inventory and roadmap](design/ui-control-inventory.md) and [shared contracts](development/ui-control-contracts.md) — owner-approved #107 audit, reconciled with merged #80; [evidence](validation/2026-11-06-issue-107-control-audit.md). Production migrations remain separate family changes.

- [Swell-engine architecture](development/swell-engine-architecture.md) — implemented ownership/call paths, data-flow/lifecycle diagrams and material regeneration boundaries; [native/profile verification](validation/2026-11-06-shared-swell-engine.md).

- [Issue #8 fresh-profile validation](validation/2026-11-06-issue-8-fresh-profile.md) — passing destination-only restore in a separate Windows account, exact metadata/audio and zero generation/recapture work.
- [Issue #8 Storage polish and recovery candidate](validation/2026-11-06-issue-8-storage-polish-recovery.md) — retained scans, pinned actions, native recovery tests and explicit remaining profile/rollout gates.
- [Issue #8 FL Studio Storage smoke](validation/2026-11-06-issue-8-fl-studio-storage-smoke.md) — successful installation retry, Backup/Save As/project-copy/reopen/Locate/Restore results and explicit same-profile/default-IR limits.

- [Issue #8 approved Storage actions](validation/2026-11-06-issue-8-storage-actions.md) — production Backup/Restore/Locate bindings, explicit receipt selection, native tests and remaining profile/host/recovery gates.

- [Portable backup backend](development/portable-asset-backups.md) — exact-byte phase-2 backend and approved production Backup/Restore/Locate controls; [dated verification and remaining gates](validation/2026-11-06-issue-8-portable-backend.md).

- [Working-folder association](development/working-folder-association.md) — delivered phase 4 folder selection, independent writers, recovery and rollback.

- [Scratch cleanup scope](development/scratch-cleanup-scope.md) — approved phase 5 scope, with the eligibility matrix, read-only preview and remaining gates.

- [Issue #8 FL Studio folder-discovery spike](validation/2026-11-06-issue-8-project-folder-spike.md) — native state-stream observations, tested host/JUCE versions, explicit-folder fallback and restored test installation.

- [Read-only asset inventory](development/asset-inventory.md) — phase-1 dependency readers, protected storage accounting and automatic Settings scans; [Settings integration evidence](validation/2026-11-06-issue-8-settings-integration.md) and [backend verification](validation/2026-11-06-issue-8-inventory.md).

- [Issue #8 measured storage audit](validation/2026-11-06-issue-8-storage-audit.md) and [contract/rollback matrix](development/asset-storage-contracts.md) — deterministic capture/IR/render growth, synchronized clone writers, version fixtures and preserved phase/owner-review gates.
- [Shared Learn/Perform IR editor](validation/2026-11-06-issue-80-shared-ir.md) — shared field/binding ownership, owner-reviewed feedback and scope refinements, native isolation/persistence/audio evidence and completed native acceptance and remaining host acceptance.
- [Host processing reactivation](development/perform-live-diagnostics.md#host-processing-reactivation) — explicit VST3 reset invalidates the live clock without changing saved source or swells; [issue #103 evidence](validation/2026-11-06-issue-103-host-reactivation.md) distinguishes native lifecycle coverage from pending FL Studio confirmation.

- [Issue #73 PR and archive record](validation/2026-11-06-issue-73-pr.md) — current-main integration, canonical spec sync, completed archives and preserved owner acceptance.

- [Stable playback Snap repair](validation/2026-11-05-issue-73-snap-fix.md) — captured-grid coordinates with independently validated playback offsets, native evidence and local test installation.

- [Combined #73 diagnostic test installation](validation/2026-11-05-issue-73-combined-test-install.md) — local integration of the existing offline repair and current grid/live evidence, with verification, installation inventory and the exact host capture procedure.

- [Issue #73 clock investigation](validation/2026-11-05-issue-73-clock-investigation.md) — separate grid/live evidence, retained local offline fix and explicitly unresolved host attribution; no timing-policy repair or main integration is implied.
- [Swells-list selection reveal](development/perform-navigation.md#fit-lifecycle) — explicit list navigation applies contextual Fit automatically, following the owner's native-review decision; [issue #93 evidence](validation/2026-11-05-issue-93-selection-reveal.md) records native verification and separate owner/host coverage.
- [Stable Perform transport readout](development/transport-readout.md) — fixed field/separator geometry, truthful formatting, bounded overflow and scoped native approval for issue #68.
- [Compact UI review — issue #58](design/compact-ui-900x600/README.md) — approved interim 900x600 layout, spacing tokens and future-capacity mockups; [native evidence and integration sequence](validation/2026-11-05-issue-58-compact-ui.md) distinguish interim approval from the pending combined implementation and final review.
  [Combined integration evidence](validation/2026-11-06-issue-58-integration.md)
  tracks the accepted #51/#68/#93 baseline and independent scale work.

- [FL Song/Pattern and source-mute spike outcome](validation/2026-11-05-issue-1-spike-outcome.md) — no reliable direct discriminator established; script onboarding/offline limitations and input-silence ambiguity recorded. Investigation complete; product workflow remains a separate decision.

- [Perform playback caution](validation/2026-11-05-perform-playback-warning.md) — persistent low-priority warning, native notice precedence/fit checks and scoped owner approval; playback/capture remain unchanged.

- [Swell source-edge conditioning](development/swell-source-edges.md) — 1 ms wet-source tapers, explicit saved-render upgrades and capture preservation.

- [User guide maintenance](development/user-guide-maintenance.md) — explicit code/spec review and read-only release currency gate; [build information](development/product-version.md#running-build-information) explains Settings About and candidate identity.

- [Private alpha preparation](validation/2026-11-04-issue-59-alpha.md) — delivered in merged PR #63; [closeout](validation/2026-11-05-issue-59-closeout.md#executed-merged-main-verification--2026-11-05) records completed post-merge verification, scoped owner sign-offs and the explicit additional-host-smoke waiver. [Redistribution review](validation/2026-11-04-issue-59-redistribution-review.md#subsequent-owner-sign-off--2026-11-05) records the one-friend private evaluation scope; public distribution remains undecided.

- [Fixed-tempo DAW export clock](development/offline-export-clock.md) — per-bounce runtime validation, unchanged sample scheduling and copyable failure diagnostics.

- [Perform waveform display](development/waveform-display.md) — GUI-only summaries, layer ownership, invalidation and close-zoom fidelity; [issue #67 evidence](validation/2026-11-05-issue-67-waveform-paint.md) separates component timing, full editor refresh and owner/host coverage.

- [OpenSpec housekeeping audit](validation/2026-11-05-openspec-housekeeping-audit.md) — four merged changes synced/archived, remaining active work and strict-validation length warnings.

- [Manual Perform preparation](development/workflow-modes.md#explicit-manual-preparation-in-perform) — explicit render requests with automatic policies off and honest unavailable timing; [issue #55 evidence](validation/2026-11-04-issue-55-manual-preparation.md) records native results and owner FL Studio acceptance.

- [Perform workspace](design/perform-workspace.md) and [navigation/state contracts](development/perform-navigation.md) — approved compact composition, contextual Fit, guarded fixed-grid gestures and optional UI preferences; [integration evidence](validation/2026-11-04-issue-46-integration.md) records validation and owner acceptance.

- [Perform live diagnostics](development/perform-live-diagnostics.md) — read-only callback admission, historical rejection and worker preparation evidence for transport silence.

- [Shared timeline controls](development/shared-timeline-controls.md) — composable Zoom/Fit intents, musical ruler and the #46 → #6 Export integration boundary.

- [Issue #4 chronological Swells list](validation/2026-11-04-issue-4-swell-list.md) — list control, scoped native acceptance and automated capture/restore evidence; host/DPI results are recorded separately.

- [Production UI design system](design/ReverbArise_UI_Layout_and_Style_Guide.md) — validated Learn/header roles, comfortable density and review rules; [issue #37 evidence](validation/2026-11-04-issue-37-ui-standards.md) separates automated/native results from host/DPI coverage.

- [Issue #25 performance audit](validation/2026-11-03-issue-25-performance-audit.md) — delivered source/thread audit and original measurements; waveform optimisation subsequently shipped in #67 / PR #69. Host/resource follow-up remains tracked by #25.

- [Local Windows releases](development/releases.md) — validated ZIPs, immutable outputs, manual publication and redistribution gate.

- [Product version and release identity](development/product-version.md) — CMake authority, exact tag checks, candidate labels and schema/host compatibility.

- [Development setup and local validation](development/development.md) — Windows prerequisites, Debug/Release builds, matching CTest and build-directory locking.
- [UI development standards](development/ui-development-standards.md) — shared controls, incremental consolidation, interaction tests and visual checkpoints for UI work.
- [Workflow modes](development/workflow-modes.md) — Learn / Perform / Export header, durable state identities, guarded transitions and capture ownership.
- [Bundled IR library](development/ir-library.md) and [musical gain staging](development/musical-gain-staging.md) — living technical references.
- [IR Level range investigation — issue #108](validation/2026-11-06-issue-108-ir-level.md) — verified per-swell semantics and the focused-dial proposal with sound-preserving extended entry. [Approved production integration](validation/2026-11-06-issue-108-integration.md) records the owner-accepted -12/+12 dial, full -60/+12 exact domain, isolation/audio-path checks and final Debug/Release results. The [earlier native checkpoint](validation/2026-11-06-issue-108-control-checkpoint.md) preserves candidate results and subsequent approval.
- [Learn source/context protocol](development/learn-source-context-protocol.md) — captured-source persistence, read-only candidates and guarded Generate transactions; implemented by merged #56 and synced/archived through #66.
- [Learn capture failure diagnostics](development/capture-failure-diagnostics.md) — bounded first-cause evidence, provisional/committed source feedback and native fault harness; [issue #54 evidence](validation/2026-11-04-issue-54-capture-failure.md) distinguishes injected failures from the unknown original FL Studio incident.
- [Loop host timing and admission](development/loop-host-admission.md) — Rack input authority, independent readiness and recovery rules for Loop integration. [Issue #123 evidence](validation/2026-11-09-issue-123-rack-timing.md) records the measured configurations and owner decision.
- [Learn workspace](design/learn-workspace.md) — approved Learn foundation plus #38 waveform/event refinement, Generate controls, diagnostic inspection and stable dry fitting.
- [Issue #38 validation](validation/2026-11-04-issue-38-generation-workflow.md) — native generation, initial Perform navigation, dense fixtures and retained owner/host acceptance; [latest-main reconciliation](validation/2026-11-04-issue-38-main-reconciliation.md) records integrated gates.
- [Song export composition](development/song-export.md) and [Export workspace reference](design/export-workspace.md) — delivered issue #6 contracts and approved control mapping; independent additive Dry/Wet levels and continuous gain preview are implemented by #51.
- [IR library evidence](validation/ir-library-evidence.md) and [gain-staging evidence](validation/gain-staging-evidence.md) — recorded validation and measurement evidence. Related JSON reports are kept alongside the IR evidence.

- [Asset storage audit](development/asset-storage-audit.md) and [dated investigation evidence](validation/2026-11-03-asset-storage-investigation.md) — source-backed Issue #8 findings; proposed portability and retention remain planning only.

## Documentation policy

Keep only this index and `product-vision.md` directly under `docs/`. Use the directories above for engineering documents and reports; check for existing copies before moving files and update links and build/test paths when relocating them.

- **GitHub Issues and approved issue mockups** describe the future roadmap and requirements.
- **Canonical OpenSpec specifications** describe the system as currently implemented; OpenSpec changes evolve them as features are built.
- **Living technical and agent guidance** is edited in place and kept current. Git history is the safety net.
- **Major superseded product visions and design proposals** are archived as historical snapshots, not rewritten to match a later vision.
- **Validation reports** preserve what was actually tested at the time, even if product behaviour subsequently changes. Historical paths in archived records describe their original context; use the current index for navigation.
- A difference between approved future design and current implementation documentation is intentional, not necessarily a contradiction.

Detailed feature acceptance criteria belong in their GitHub issues, not this documentation index or the vision.
