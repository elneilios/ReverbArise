# Design

## Context

See proposal.md for motivation and issue traceability. SongExportPlan, ExportState and ExportFile currently carry a single `wet` fraction; song-export-v1 identifies the crossfade. SongMix gates dry on wet<1, scales dry by 1-wet, applies measured wetGain and sumGain ceilings, and shares its measurements with detail readers. ExportWorkspace already uses SliderControl, ActionButton, FileDragTile and shared timeline navigation. Tests/SongMixCases.h and Tests/ExportControlCases.h provide focused extension points.

## Goals / Non-Goals

**Goals:** one accepted pair and gain-law revision through state, preparation, measurement, preview and publication; bounded page processing; truthful amplitude/peak presentation and immutable assets.

**Non-Goals:** external DAW equivalence without matched assets/routing, new output trim, Perform gain changes, a generic UI redesign or migration of unrelated controls.

## Decisions

1. **Approved transparent sum.** Owner approved on 2026-11-05: `dryLevel*capturedDry + wetLevel*acceptedWet`, independently bounded to [0,1]. Preserve per-swell render calibration, Level, shape and existing per-swell edge treatment consistently in both preview and WAV. Remove only Export aggregate wet and sum ceilings; retain rejection of non-finite samples. Measure resulting peak without altering PCM. Peak above unity produces an accessible warning, not normalization or a hidden limiter. Keep boundary start/stop/seek ramps in ExportVoice monitoring only and exclude those boundary samples from parity oracles. Existing upstream render calibration is part of the comparable accepted wet layer; do not claim equality to uncalibrated source convolution. An optional output trim would add scope and is excluded.

2. **Pair-based immutable identity.** Carry independent levels through ExportState, requests, SongExportPlan, prepared metrics, detail and ExportFile. Change songExportRevision/gain-law identity so even an identical numeric-looking legacy request cannot reuse an old crossfade WAV. Serialize canonical finite bounded values with sufficient precision; reject invalid new state. One setter updates the pair atomically for presets, coalescing one intent rather than intermediate pair revisions. Retain instance/session/pass/generation/dependency guards and previous presentation distinction. Level changes reuse accepted convolution assets, never regenerate them.

3. **Silence and dependency gating.** Gate dry reads/warnings by dryLevel>0 and wet reads by wetLevel>0, independently. Retain song origin and existing complete range policy. At 0/0 show Silent, disable preview/seek/Save/drag, and do not publish a misleading ready revision or reuse prior ready actions. Nonzero levels with all-zero PCM retain honest no-audible-start behavior. Audible Fit is calculated from the weighted accepted samples, including cancellation; waveform contributions remain separately white/aqua on one disclosed display scale. Display scaling never changes PCM. A common scale can accommodate above-unity layers without falsely clipping their drawn peaks.

4. **Preset is a pair shortcut.** Initial candidates are Wet Layer 0/100, Subtle 100/25, Balanced 100/50, Cinematic 100/100, Wet Forward 65/100. These values and names are not sonic approval. Show Custom for unmatched pairs; derive matched names from canonical pair values without rounding arbitrary saved values into a preset. Retain the current wet-only default as 0/100. Preset selection modifies only levels, not source, render, output gain or processing. Direct fader adjustment preserves the other level. Reuse SliderControl/NumericInputControl at the existing 0.1% edit step without reducing saved precision.

5. **Scoped control composition.** Add a compact reusable accessible preset selector using existing Controls conventions, invocation/no-wheel policy and theme; IrSelector is domain-specific and must not be copied with its IR behavior. Compose it above two shared slider/value rows in the right column. Preserve left waveform/ruler, below-waveform Zoom/Fit and percentage legend, preview transport, WAV/Save/Return and header notices. Use production Inter roles and UiGeometry spacing 4/6/12/16/24; no speculative tokens. Approve the new/revised selector family and two-fader interaction in a directly launched native preview before production integration. Any necessary variant requires scoped owner approval, standards revision and tests; never regenerate UI debt inventory to hide failures.

## Risks / Trade-offs

- Above-unity addition can overload downstream playback/conversion → show actual peak and warning in accepted status; preserve Float32 values and document downstream responsibility. No automatic attenuation.
- Displayed intent can differ from retained waveform during work → label previous accepted pair and pending/error separately; disable current-file actions until matching completion.
- Legacy identity reuse or reset could detach history → preserve old descriptors and files with legacy semantics, invalidate only desired/current readiness and generate a fresh identity/path.
- Preset double updates or stale gestures can publish wrong pairs → atomic pair setter and session/pass-pinned bindings, automated out-of-order completion and restore tests.
- Cancelling layers complicates audible bounds → sample-level antiphase and sparse-gap tests, separate contribution display from summed audible Fit.

## Migration Plan

After a fresh Apply request, continue on an issue-specific branch with #6 baseline merged and review findings checked; preserve unrelated #71 planning work. Introduce an explicit Export preference version and new composition revision. Read old single-crossfade state/history without deleting assets; reset the old preference to the existing wet-only default 0/100, clear obsolete desired/current identity, preserve all historical file references and disclose the developer-only reset. Current pair state round-trips exactly within existing float precision. Keep VST3/session identity and unrelated creative state unchanged. Rollback uses the previous binary and a saved pre-change test project; no rollback process rewrites managed assets. Update living guidance and dated evidence with the migration behavior.

## Review and Validation

Extend focused DSP, state, engine and component tests before broad Debug/Release wrapper validation. Compare composite PCM and decoded WAV against separately prepared comparable dry/wet layers at matching gains, including unity overlap, antiphase, above-unity peaks, sparse source, leading silence, pre-arrival wet and late tails. Verify preview interior parity separately from boundary ramps. Exercise rapid edits, stale restore, disk failure, Save/drag readiness and history byte/path preservation. Run check-ui-standards.ps1 and launch native preview at 1100x850 and larger supported sizes; record actual coverage. Obtain scoped control approval before integration, integrated acceptance after it, then consolidate FL Studio checks into one short final pass. Candidate preset listening approval remains a required checkpoint, not a claim in this plan.
