# Design

## Context

`LearnWorkspace` and `PluginEditor` each own a `ConvolutionIrEditor` and `ConvolutionIrBinding`. They already share accepted identity, import invocation and guarded response dispatch. Perform separately owns its Level cell in the inspector value group. Learn has no Level control even though #190 provides `SwellEngine::setDefaultLevel`. The Learn import has an additional entry point in Diagnostics. The responsive Perform layout has a 276-unit inspector, a scrolling content viewport and a fixed Reset strip.

The #189 HTML preview provides composition and interaction evidence. Production geometry and semantics come from the [UI style guide](../../../docs/design/ReverbArise_UI_Layout_and_Style_Guide.md), [shared contracts](../../../docs/development/ui-control-contracts.md), [control inventory](../../../docs/design/ui-control-inventory.md), [Learn workspace](../../../docs/design/learn-workspace.md) and [workflow modes](../../../docs/development/workflow-modes.md).

## Goals / Non-Goals

**Goals**

- Share one control composition, presentation state machine and binding policy across the two hosts. Host adapters supply placement, visible context and guarded edit ownership.
- Make waveform and loading presentation independently testable without touching audio-thread work or the saved session.

**Non-Goals**

- Creative IR generation, tempo tracking, new host parameters, global control-family migration, cache file formats and history.

## Decisions

1. **Evolve the shared IR editor and binding.** Replace the flat list with preset rows keyed by definition identity and a separate file list keyed by managed identity. Filter managed choices from the existing engine catalogue and preserve the matching-PCM Custom distinction. Reuse the default and selected dispatch paths for response requests and the guarded default Level setter and Perform gain edit path for Level. Move the Perform Level presentation into the shared editor while retaining exact entry, dB calibration, legacy display, detent, edit guards and Reset behavior. Refresh reconciles accepted values silently. Leaving an unused independent Level cell or duplicating the editor in both hosts would split those contracts.
2. **Keep Custom IR presentation local until a file is accepted.** Selecting Custom IR from a bundled Sound shows “Custom IR (choose file)” and reveals the file row. The menu checkmark, preview waveform and audio still represent the accepted preset. The row remains available for retry after cancellation or error while the context is valid. Choosing a preset or changing scope dismisses the transient file choice. Route the existing Diagnostics default entry through the same contextual picker and capture guards. Preserve native chooser lifetime and focus restoration with `SurfaceInvocation`. Capture the target, catalogue identities and session/pass or swell input revision at invocation, and reject or cancel stale results.
3. **Reuse existing UI families with scoped native review.** Use C1 command buttons with scoped Previous/Next glyphs, C2 deliberate selector and popup policy, C3 field text/validation and aqua footer, C5 compact `KnobValue` with the existing paired geometry, and C6 switch for Boolean definitions. A UI definition supplies labels, order and typed control values. Continuous controls use existing exact entry. Use a stepped C5 value mapping and subdivision readout for Echo Rate, including accessible value text and existing gestures where applicable. Exercise stepped and Boolean presentation with preview-only fixtures. Production definitions expose Level for the six bundled presets and Custom IR. Review the new glyphs, stepped variant, selector decoration and busy treatment in a native preview before workspace adoption. Any approved exception is scoped and recorded in the standards and verified alongside implementation.
4. **Prepare bounded response peaks off the UI and audio threads.** Reuse bundled descriptors and verified managed readers with a bounded, low-priority presentation job and immutable mono/stereo summaries. Coalesce duplicate jobs and cap bins, pending work and cache memory. Key summaries by resolved response identity with the asset location or generation authority needed for safe invalidation. Share usable summaries across both panels and popup rows. Use the actual accepted reference for the closed control, each option's own reference for menu rows, and an already cached default response for generated options when a later preset supplies one. Paint decoration behind semantic text and state layers without changing geometry or accessible names. Missing assets, in-flight peaks and stale replies leave decoration absent where no valid preview exists. Menu inspection never requests synthesis.
5. **Show progress for a specific request and visible owner.** The current status projection exposes names and errors, and Learn has a separate 600 ms notice. Expose or reuse a transient request identity from the existing assignment lifecycle so repeated requests for the same name cannot share a timer. Base the two-second delay on a monotonic clock, bind it to the visible Sound context and dismiss it on settlement, supersession, tab exit, or context loss. Replace the old preparing notices in this path while preserving unrelated workspace notices and recovery feedback. Cover the Sound content viewport only. Close or suspend active panel input, keep outer actions reachable, announce the caption once, honor reduced motion with a static waveform and restore focus only to a valid target that still owns it.

## Risks / Trade-offs

- **Old and new assignments differ** → Bind the checkmark and closed waveform to accepted state. Use the accepted label during preparation and identify the Custom file-choice prompt explicitly. Track local import failure and pending requests separately. Verify #183 atomic audio handover, shorter-response fitting and offline snapshots.
- **Compact and scaled layouts can clip** → Use production dimensions, focus reveal and existing scroll policies. Inspect both workspaces at 900×600 and representative expanded sizes at 100% and 125%, including long names, wrapped errors, narrow geometry and an unavailable selection.
- **Picker or cache work can outlive its context** → Carry stable request and asset keys through callbacks, verify lifetime and owner before display or edit, and keep cache reads and file access outside painting and playback.

## Migration Plan

This change reads and writes the existing #190 state format. Open legacy and current fixtures with their accepted identity, Level, source, freeze and wet timing intent intact. Use a native component preview before adopting the shared editor in production, then verify the integrated editor and a brief FL Studio smoke.
