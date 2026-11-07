# Design

## Context

See [proposal](proposal.md). The owner reviewed `418ac1c` and asked to retain the standard knob appearance and remove the extra labels and saved-value caption. A primary press on the dial brings an older saved value into range. The ordinary input shows the exact saved gain up to that point.

Level is per-swell `Shape::gainDb`, not IR asset gain. The durable finite [-60,+12] float domain and `shape.gain` serialization must remain readable. There is no Level host parameter. Learn shares IR selection with Perform but has no Level default.

## Goals / Non-Goals

**Goals:** One -12/+12 editing range in the standard knob, 0 dB at 12 o'clock, and preservation of older gains until the user operates Level.

**Non-Goals:** Extended authoring below -12 dB, new Learn gain defaults, bulk/history infrastructure, import normalization, gain stages, protection changes, mute controls, effects, a general control framework or plugin-wide migration.

## Decisions

### Separate compatibility validation from editing limits

Keep named durable limits [-60,+12] for state decode, save, DSP, frozen restoration and saved snapshots. Define the new-edit limits [-12,+12] once and use them for every production Level authoring path, including numeric/accessibility writes and any Settings/Diagnostics consumer. Narrowing the durable validator would reject or alter existing projects, so keep it unchanged. Preserve fractional saved values; valid in-range typed values need not be multiples of the 1/0.1 dB gesture steps.

All new values are finite and inside [-12,+12]. Relative movement clamps at those boundaries; out-of-range typed/absolute requests are rejected rather than silently accepted or clamped. No separate extended authoring path remains. Reset is exactly 0 dB. The owner-confirmed contract supersedes the earlier recommendation to retain extended entry; the prior measurements remain historical evidence about gain and source dynamics.

### Reuse the standard knob

Use the linear fraction `(dB + 12)/24` with the existing KnobValue caption, field, dial and drag response. Verify appearance and layout against another standard knob and confirm the actual paint puts 0 dB at 12 o'clock at supported sizes and user scales. Use the existing contextual help to describe the limits.

Drag, wheel and key input share the same bounded editing policy. Typed and absolute-accessibility requests use the same limits and validation. Preserve captured session/selection/input/render/pass guards and the established reset lifecycle. Keep absolute-versus-relative intent only where it serves these common limits; do not add another control framework or alter other KnobValue consumers. Real native pointer-event tests must cover both directions and endpoints, rather than using only synthetic Slider::setValue calls.

### Preserve an older out-of-range value until an accepted edit

For a saved value such as -30 dB, loading, displaying, focusing, hovering, saving, unrelated edits and rejected/cancelled text entry preserve the exact float and sound. The standard input shows the actual value while the dial rests at -12 dB. Accessibility exposes the bounded dial position, the exact applied value and the fixed edit limits.

Use the bounded displayed position as the anchor for relative input. A primary press commits -12 immediately when Level is below range, and a subsequent drag continues from that endpoint. An outward wheel/key step sets -12, and an inward coarse step sets -11. Valid typed or absolute-accessibility entry applies its requested value directly, and Reset sets 0. Unchanged old text and rejected text retain the applied gain.

Use the normal captured edit guards for the primary press and the subsequent drag. Passive refresh and selection cannot trigger a write. Test the legacy boundary with native pointer events, wheel, keys, numeric entry and accessibility.

### Preserve ownership, history and cache behavior

Only the captured selected swell changes. Learn's accepted future IR selection, its next generated swell's 0 dB initial shape, neighbours and other instances stay independent. Do not attach Level to `ConvolutionIrEditor` or `ConvolutionIrBinding`, which own IR assignment rather than shape edits. #9 must later consume this semantic domain and existing validated edit commands with stable targets/mixed values; do not implement bulk editing here.

No undo/redo stack or multi-program preset mechanism exists for this control in the inspected baseline. Preserve actual float values in saved snapshots; a future #9 undo record must retain historical before/after values even when below the new editing minimum. Do not add history infrastructure under #108.

An accepted gain edit increments shape generation and rebuilds shaped/aggregate results through current worker scheduling. Preserve raw render keys, calibration revision/amount, source/IR assets and frozen eligibility. Cache/member identities already include actual gain, not dial fraction. Ordinary editor refresh must not publish edits or invalidate caches. No convolution or callback work is added.

### Preserve all admitted audio paths

Production gain remains `10^((calibration.baselineDb + shape.gainDb)/20)` times the current envelope, before path-specific protection and monitoring/output gains. Keep audition, live Song, plugin WAV and host offline bounce routing/membership policies unchanged. Compare equal accepted content and unity wet controls away from disclosed boundary ramps; separately exercise genuine protection to avoid expecting uncapped ratios.

## Risks / Trade-offs

- First touch can substantially change an older gain -> keep passive actions read-only and verify the primary press and both relative directions explicitly.
- Prior tests overstated extended pointer coverage -> use real mouseDown/mouseDrag/mouseUp paths in both directions; distinguish earlier results from new verification.
- Existing listening evidence uses synthetic sources -> retain the scoped approval and source-relative quietness observation without claiming a universal audibility threshold or changing calibration. Request another host smoke only for concrete unresolved host behavior.
- Shared-control work may move under #107 -> reconcile the current branch before Apply; reuse the accepted family and avoid a competing abstraction.

## Migration Plan

No load-time state/asset migration is required. Keep session JSON v11, plugin wrapper, stored floats, calibration and engine revisions unchanged. Old -60/-30/fractional values remain supported in Legacy/Musical and frozen projects until accepted Level edits. Save/reopen before editing preserves sound; save/reopen after editing preserves the chosen bounded value. Rollback can read both. There is no Level host parameter or normalized automation mapping to remap; if one appears concurrently, resolve it before implementation. Do not silently adopt Musical policy in Legacy projects.

Sections 1-4 of tasks.md retain the completed earlier implementation record. Section 5 tracks the correction and validation. Keep the PR draft through validation and retain the dated evidence for earlier candidates.
