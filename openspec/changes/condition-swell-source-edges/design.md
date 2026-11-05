# Design

## Context

See proposal.md and the dated source-boundary analysis. SwellEngine reads an owned stereo SourceInterval on its worker, then calls a generic mathematical convolver. Raw assets carry a source-content dependency and remain independently verified. Dry calibration reads a separate original interval. Regenerate All currently skips every ready swell.

## Goals / Non-Goals

Keep conditioning at the wet-generation boundary, with no callback work or changes to generic convolution, capture, dry calibration inputs, existing shape/gain policies, file codec or scheduling. Preserve restored and frozen audio. Output protection and tempo-map issue #71 remain separate.

## Decisions

- Add an allocation-free in-place helper for the worker-owned interleaved stereo source vector. Nominal ramp frames are round(rate * 0.001), capped at floor(N/2). For at least two ramp frames, use 0.5 - 0.5*cos(pi*i/(rampFrames-1)); mirror at the end. Fewer than four input frames remain unchanged to avoid erasing tiny selections. Existing readers reject invalid samples. Check cancellation before and during bounded ramp work.
- Keep the exact generic convolver unchanged. Condition only SwellEngine's local wet source before calling it. Dry references and capture fingerprints use original samples.
- Prefix new render dependencies with `source-edges-1ms-v1:` followed by the original source fingerprint. Asset metadata already supports arbitrary dependency strings and content hashing includes that field. No schema/codec migration is required. Matching accepts either that exact current dependency or the exact legacy source fingerprint; unknown revisions do not match. Revalidation still compares original source fingerprints.
- Keep legacy renders on restore/check. Explicit selected generation already bypasses cached renders. Regenerate All includes ready assets without the current prefix, while retaining its skip for ready current assets and all frozen assets. A new successful render recalibrates against original dry samples and its new render identity.
- Automatic generation's legacy fallback reads the original fingerprint from known prefixed render dependencies, so a processing revision is not mistaken for recaptured source. Keep this small metadata helper in Core to avoid an Analysis/DSP dependency cycle.
- Keep worker cancellation and publication guards; failed/superseded work never replaces committed metadata. No captured-source writes occur.

## Risks / Trade-offs

- The taper changes very short attacks at source cuts → only 1 ms, unchanged middle, linked stereo, independent numerical and overlap regression evidence.
- Older saved renders still contain artifacts until regeneration → explain Regenerate All once after installing; no recapture needed.
- Some existing engine fixtures use impulses exactly at cut edges → update only expectations for wet generation, retaining generic convolution oracles and original dry assertions.
- Musical improvement still needs one FL Studio listening check → automated comparison against the supplied four-source case plus final owner smoke test, not a claim of host acceptance.

## Migration Plan

Local branch based on the accepted export-clock fix. Run focused tests then full native Debug/Release wrappers. Install the complete Release VST3 using the supported helper with prior bundle retained. Preserve original projects/assets; rollback may use the prior bundle and original saved project. No publication or schema bump.
