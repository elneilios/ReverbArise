# Proposal

## Why

ReverbArise now captures, prepares, edits, saves and plays swells, but its native editor still exposes a feasibility-spike composition, duplicated monitoring controls and boxed engineering panels. Align the working plugin with the approved flat charcoal/aqua design so musicians can understand and edit actual results without sacrificing the validated engine or saved sessions.

## What Changes

- **Visual/layout:** Create eight stable regions in six horizontal bands: branded header, full-width global marker and regeneration controls, three-column Swells/editor/Selected Swell workspace, full-width Audition, Overview and host information. Use centralized baseline tokens and thin separators; only the Overview waveform viewport gets a major rounded outline.
- **Visual/layout:** Replace legacy editor copy with the integrated waveform-A ReverbArise wordmark and italic tagline; reuse the supplied outlined SVG in resources/logo without redrawing it. Use supplied licensed Inter typography with a documented system fallback, simple stroke icons, neutral dry waveforms, aqua wet/envelope data and restrained yellow timing markers.
- **Interaction/binding:** Reuse existing model operations for individual IRs, stable selection, activity, source/bounds, placement and fades. Expose Pre-roll, signed End Offset, Fade In, Fade Out, Level, Curve In and Curve Out as synchronized knobs/exact entries. The user explicitly permits the additional curve knob to preserve independent tensions; every movable waveform edit, including target, source start/end and whole-wet placement, also has an inspector knob and exact entry. Curve presets remain in Settings. Source selection has matching vertical boundaries and a subtle shaded range. Freeze and Solo stay visible in Selected Swell, with a real bundled-IR dropdown and adjacent file loader.
- **Interaction/binding:** Replace separate audition mode buttons with one continuous Dry-to-Wet slider and percentage. Preserve legacy audible mode on restore, protection, trim, isolation, tail, preview playhead and the independent live-routing settings. Map 100% to genuine Wet Only without requiring captured dry context.
- **Interaction/binding:** Keep automatic capture/marker/render semantics, truthful coverage/readiness and secondary recovery. Keep both sensitivity knobs in the global controls band; put other advanced playback/processing and audition preferences in the header Settings menu; retain a dedicated settings-accessed diagnostics window in normal builds without shrinking the workspace.
- **Interaction/binding:** Add a visible Reset split button: direct wet-shape defaults (including 20 ms Smooth release) and explicit Reset all swell settings. The latter rebuilds creation defaults at the current target using current capture/default IR, preserving identity/name/arrival and unrelated state. New swell names use the arrival Playlist bar, not collection count; saved/custom names remain intact.
- **Focused cleanup:** Audit every current diagnostic control/readout; consolidate duplicate sensitivity/status, move musical controls to normal menus and retire only proven-unused bindings/helpers. Preserve the tested scheduled tone, explicit recovery and compatibility fields. Do not merely hide obsolete polling or side effects.
- **Validation:** Add layout/colour/coordinate/binding assertions and matched native screenshots at supported sizes/scales, retain audio/state/transport/export regressions, and require a separate FL Studio visual/interaction/DPI acceptance record.

## Capabilities

### New Capabilities

None. Extend existing UI and monitoring capability boundaries.

### Modified Capabilities

- `swell-workspace`: Stable approved composition and visual system, musical inspector, truthful overview, purposeful dedicated diagnostics, exact interactions, compatibility and visual acceptance.
- `audition-monitoring`: Single continuous audition mix presentation with faithful existing-state restoration and exact endpoint geometry, replacing the separate mode-button contract while retaining audio laws and safety.
- `swell-editing`: Explicit wet-only/all-swell reset scopes and default bar-based names for newly created swells.
- `transport-swell-playback`: Restyle envelope handles as circles while preserving accepted centre-line endpoints, true-time stacking and gesture semantics; retain independent live control access.

## Impact

Primary implementation targets are `Source/Plugin/PluginEditor.{h,cpp}` and `Source/UI/SwellTimeline.{h,cpp}`, plus focused native theme/layout/control/diagnostics components under `Source/UI`. Reuse `EnvelopeHandles.h`, `SwellViewport.h`, `SongBarRuler.h`, `MusicalPosition.h`, `SwellEngine` snapshots/commands and version-7 session properties. Small UI binding adapters are necessary for relative timing and the unified mix, plus an explicit-ID background assignment path for bundled/file IR selection; no new host parameters or audio architecture are proposed.

Extend `tests/swell_ui_test.cpp` and targeted model/processor tests; update current usage/validation docs during apply. No framework rewrite, web UI, new dependency, general undo/preset system, speculative DSP, Pack/Relink, standalone file export or plugin-to-host transport control. Preserve plugin IDs/name as presented to the host, `PreverbState`, asset namespaces, checksums and existing saved values. No code is changed by this proposal.

References, code evidence, the requirement gap matrix, diagnostics dispositions, conflicts and task/evidence traceability are in `design.md`. There are no other active OpenSpec changes at proposal creation; all preceding milestones are archived. This change intentionally corrects stale main-spec UI wording that still calls transport deferred.
