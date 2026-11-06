# Design

## Context

See [proposal](proposal.md) and [investigation](../../../docs/validation/2026-11-06-issue-108-ir-level.md). The current visible control is per-swell Level, not IR asset gain. `Shape::gainDb` is a finite float in [-60,12], serialized as `shape.gain`; there is no host Level parameter. Learn shares IR selection with Perform after #80, but has no Level default. Compatibility and the difference between dial sweep and stored domain warrant this design.

## Goals / Non-Goals

**Goals:** Increase dial travel per dB around neutral, preserve all supported numerical values and audio, and make extended edits deliberate and understandable.

**Non-Goals:** New Learn gain defaults, bulk selection/history, import normalization, gain stages, protection changes, mute controls, processor effects, a general control framework or plugin-wide migration.

## Decisions

### Keep the persisted/DSP domain; focus the main sweep

Use named constants for persisted minimum -60, main minimum -12, maximum +12, default 0, coarse 1 and fine 0.1 dB. Put the semantic constants near the existing shape definition and the UI mapping beside the binding; do not embed UI geometry in DSP. The core validator must continue accepting [-60,12]. No quantization of existing floats or exact entry is allowed. Gesture steps retain existing precision; typed values need not be multiples of 0.1.

The displayed dial fraction is `clamp((dB + 12)/24, 0, 1)`, replacing `(dB + 60)/72`. Keep default/reset at 0 dB. The same angular travel then produces one third the dB change, while 0 dB moves from fraction 5/6 to 1/2. This is a resolution calculation, not measured usability approval. Reject a hard stored minimum of -12: it would alter valid projects and remove useful attenuation. Retaining the full 72 dB dial would preserve compatibility but miss the owner's interaction goal; a new mute is unnecessary.

### Separate relative gestures from explicit absolute entry

Normal in-range drag/wheel/key edits use the 24 dB span and [-12,12] bounds. If the invocation starts below -12, anchor relative edits to its exact value, use the same sensitivity and permit [-60,12], without snapping at invocation. Example: -30 plus one coarse step becomes -29; one fine step becomes -29.9. Once a new invocation starts inside the main range it uses the main bounds again. A saturated lower dial cannot by itself display distance below -12; exact numeric text and existing tooltip/accessibility help must disclose the full value and extended range.

Typed entry and absolute accessibility writes use the entire semantic domain and the existing finite-value validation transaction, not the narrower relative clamp. In `KnobValue`, numeric accessibility currently converts absolute writes to `onDelta`; add only the small explicit absolute-intent callback needed by this binding, preserving existing behavior for other consumers. Pin session/selection/input/render/pass as current edit guards do. Avoid formatting an exact value through the compact one-decimal readout before commit. Do not add a visual token, control or style variant. Review this revised family behavior natively before production integration; coordinate with #107 and update the applicable standards revision/tests together.

### Preserve ownership, history and cache behavior

Only the captured selected swell changes. Learn's accepted future IR selection, its next generated swell's 0 dB initial shape, neighbours and other instances stay independent. Do not attach Level to `ConvolutionIrEditor` or `ConvolutionIrBinding`, which own IR assignment rather than shape edits. #9 must later consume this semantic domain and existing validated edit commands with stable targets/mixed values; do not implement bulk editing here.

No undo/redo stack or multi-program preset mechanism exists for this control in the inspected baseline. Preserve actual float values in full-state snapshots; a future #9 undo record must store before/after dB values rather than dial fractions. Do not introduce history infrastructure under #108.

An accepted gain edit increments shape generation and rebuilds shaped/aggregate results through current worker scheduling. Preserve raw render keys, calibration revision/amount, source/IR assets and frozen eligibility. Cache/member identities already include actual gain, not dial fraction. Ordinary editor refresh must not publish edits or invalidate caches. No convolution or callback work is added.

### Preserve all admitted audio paths

Production gain remains `10^((calibration.baselineDb + shape.gainDb)/20)` times the current envelope, before path-specific protection and monitoring/output gains. Keep audition, live Song, plugin WAV and host offline bounce routing/membership policies unchanged. Compare equal accepted content and unity wet controls away from disclosed boundary ramps; separately exercise genuine protection to avoid expecting uncapped ratios.

## Risks / Trade-offs

- Saturated extended-value indicator can be mistaken for -12 -> retain actual numeric/accessible value, provide range help, and include -30/-60 native review cases.
- Different absolute and relative edit bounds can diverge -> explicit intent paths, shared domain constants and binding/accessibility tests, including stale target rejection.
- Existing evidence uses synthetic sources and historical production measurements -> do not claim new listening approval, current-binary measurements or universal loudness equivalence. Carry one short scoped native/listening checkpoint into Apply, then a brief final host smoke only for unresolved host behavior.
- Shared-control work may move under #107 -> reconcile the current branch before Apply; reuse the accepted family and avoid a competing abstraction.

## Migration Plan

No state/asset migration is required. Keep session JSON v11 and existing plugin wrapper, stored floats, calibration and engine revisions unchanged. There is no Level host parameter or normalized automation mapping to remap. Test zero Level parameters in the current processor interface; if a parameter is introduced concurrently, stop and resolve its mapping before implementation proceeds. Older binaries still understand the unchanged dB values. Rollback restores the prior UI mapping with no audio or asset conversion. Do not silently adopt Musical policy in Legacy projects.

Implementation begins only on a new Apply request after proposal review. Native family approval precedes wider rollout. Keep canonical specs unchanged until the normal sync/archive lifecycle is explicitly requested.
