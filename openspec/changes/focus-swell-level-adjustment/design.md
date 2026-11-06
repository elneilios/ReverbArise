# Design

## Context

See [proposal](proposal.md). At `541b8ba`, the control has a -12/+12 display sweep but retains extended editing and [-60,+12] numeric entry. Review found that downward native dragging at an extended value is blocked by the normalized Slider endpoint, although wheel/key edits can decrease it. The owner then explicitly confirmed a simpler fixed editing range and preservation of older values until a deliberate change/reset. This revision implements that clarification; earlier approval/evidence does not verify the revised interaction.

Level is per-swell `Shape::gainDb`, not IR asset gain. The durable finite [-60,+12] float domain and `shape.gain` serialization must remain readable. There is no Level host parameter. Learn shares IR selection with Perform but has no Level default.

## Goals / Non-Goals

**Goals:** One clear -12/+12 editing range, visibly labeled limits, 0 dB at 12 o'clock, consistent input methods, and sound-preserving old-project retention until accepted edits.

**Non-Goals:** Extended authoring below -12 dB, new Learn gain defaults, bulk/history infrastructure, import normalization, gain stages, protection changes, mute controls, effects, a general control framework or plugin-wide migration.

## Decisions

### Separate compatibility validation from editing limits

Keep named durable limits [-60,+12] for state decode, save, DSP, frozen restoration and saved snapshots. Define the new-edit limits [-12,+12] once and use them for every production Level authoring path, including numeric/accessibility writes and any Settings/Diagnostics consumer. Narrowing the durable validator would reject or alter existing projects, so keep it unchanged. Preserve fractional saved values; valid in-range typed values need not be multiples of the 1/0.1 dB gesture steps.

All new values are finite and inside [-12,+12]. Relative movement clamps at those boundaries; out-of-range typed/absolute requests are rejected rather than silently accepted or clamped. No separate extended authoring path remains. Reset is exactly 0 dB. The owner-confirmed contract supersedes the earlier recommendation to retain extended entry; the prior measurements remain historical evidence about gain and source dynamics.

### Make the dial's bounds and center explicit

Use the linear fraction `(dB + 12)/24` inside the editing range. Show persistent -12 dB and +12 dB endpoint labels and a clear 0 dB center reference. Verify the actual rotary angles/paint geometry place 0 dB physically at 12 o'clock; a numeric fraction of 0.5 alone is insufficient evidence. Reuse the existing text roles, geometry/spacing tokens and KnobValue family. Keep the scoped labels/layout usable at the supported compact size and user scales, and obtain native family approval before rollout. No new visual token or one-off variant is presumed approved.

Drag, wheel and key input share the same bounded editing policy. Typed and absolute-accessibility requests use the same limits and validation. Preserve captured session/selection/input/render/pass guards and the established reset lifecycle. Keep absolute-versus-relative intent only where it serves these common limits; do not add another control framework or alter other KnobValue consumers. Real native pointer-event tests must cover both directions and endpoints, rather than using only synthetic Slider::setValue calls.

### Preserve an older out-of-range value until an accepted edit

For a supported saved value such as -30 dB, loading, displaying, focusing, hovering, mouse-down without movement, saving, unrelated edits and rejected/cancelled text entry preserve the exact float and sound. Show its actual value with a short saved-value indication; help/accessibility must distinguish the preserved applied value from the fixed editing limits. The dial rests at the -12 endpoint without writing that endpoint to the model. This indication is compatibility feedback, not an extended editing mode or another editable range.

Relative edits begin from the bounded displayed position, not from the old numerical gain. A relative move that cannot change that bounded position is a no-op: at saved -30, downward input at the lower endpoint preserves -30, and an upward coarse step selects -11. The same rule applies to pointer, wheel and keys; there is no wheel-only route to -31. An accepted explicit value of -12 sets -12, and Reset sets 0. Once a valid value is committed, ordinary [-12,+12] editing applies. Unchanged old text is not a fresh invalid edit; changed out-of-range text is rejected without changing the old sound.

Such an explicit edit can change sound substantially; never perform it merely because the UI refreshes or an edit starts. Bind the saved-value indication and actual readback to accepted state, and retain stale-intent rejection. Test the legacy boundary with actual pointer events as well as wheel/key/numeric/accessibility commands so no input method secretly retains extended authoring.

### Preserve ownership, history and cache behavior

Only the captured selected swell changes. Learn's accepted future IR selection, its next generated swell's 0 dB initial shape, neighbours and other instances stay independent. Do not attach Level to `ConvolutionIrEditor` or `ConvolutionIrBinding`, which own IR assignment rather than shape edits. #9 must later consume this semantic domain and existing validated edit commands with stable targets/mixed values; do not implement bulk editing here.

No undo/redo stack or multi-program preset mechanism exists for this control in the inspected baseline. Preserve actual float values in saved snapshots; a future #9 undo record must retain historical before/after values even when below the new editing minimum. Do not add history infrastructure under #108.

An accepted gain edit increments shape generation and rebuilds shaped/aggregate results through current worker scheduling. Preserve raw render keys, calibration revision/amount, source/IR assets and frozen eligibility. Cache/member identities already include actual gain, not dial fraction. Ordinary editor refresh must not publish edits or invalidate caches. No convolution or callback work is added.

### Preserve all admitted audio paths

Production gain remains `10^((calibration.baselineDb + shape.gainDb)/20)` times the current envelope, before path-specific protection and monitoring/output gains. Keep audition, live Song, plugin WAV and host offline bounce routing/membership policies unchanged. Compare equal accepted content and unity wet controls away from disclosed boundary ramps; separately exercise genuine protection to avoid expecting uncapped ratios.

## Risks / Trade-offs

- A preserved old value sits outside the editing range -> show its actual saved value truthfully, avoid automatic writes, and verify first accepted edits and no-op interactions across all input methods.
- Bounds or center labels can be unclear at compact size -> use existing roles/tokens, native geometry/render checks, and one focused owner review of the revised presentation.
- Prior tests overstated extended pointer coverage -> use real mouseDown/mouseDrag/mouseUp paths in both directions; distinguish earlier results from new verification.
- Existing listening evidence uses synthetic sources -> retain the scoped approval and source-relative quietness observation without claiming a universal audibility threshold or changing calibration. Request another host smoke only for concrete unresolved host behavior.
- Shared-control work may move under #107 -> reconcile the current branch before Apply; reuse the accepted family and avoid a competing abstraction.

## Migration Plan

No load-time state/asset migration is required. Keep session JSON v11, plugin wrapper, stored floats, calibration and engine revisions unchanged. Old -60/-30/fractional values remain supported in Legacy/Musical and frozen projects until accepted Level edits. Save/reopen before editing preserves sound; save/reopen after editing preserves the chosen bounded value. Rollback can read both. There is no Level host parameter or normalized automation mapping to remap; if one appears concurrently, resolve it before implementation. Do not silently adopt Musical policy in Legacy projects.

This invocation updates planning only. Sections 1-4 of tasks.md retain the completed earlier implementation record; section 5 contains unchecked correction work and fresh validation. Use Apply separately to implement the revised contract and keep the PR draft until it matches the plan and passes the required review/checks. Keep canonical specs and dated historical evidence unchanged; sync/archive and merge remain separately authorized lifecycle actions.
