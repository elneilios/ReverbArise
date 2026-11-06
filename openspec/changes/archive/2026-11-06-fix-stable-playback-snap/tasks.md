# Tasks

## 1. Separate accepted grid coordinates from run validation

- [x] 1.1 Expose a dedicated value-only current transport clock through the engine/editor boundary, separate from telemetry and render readiness. Add native tests for current session/reset/run ownership, Stop/new Play, restore and independent instances; verify unavailable/stale reads cannot authorize a grid and callback ownership, bounded publication and allocation behavior remain unchanged. Document the control-versus-diagnostic authority in the existing timing guidance.
- [x] 1.2 Extend the shared grid result/factory with explicit run validation while retaining accepted quantization coordinates and strict default callers. Extend host_grid_test with the exact owner tuples, zero/positive/negative stable offsets, missing/rejected/stale context, drift-to-saved-origin fallback rejection, one-sample boundary, rate/tempo/meter, missing sample position, division/overflow and independent integer/decimal grid oracles. Verify no hard-coded compensation or shared guard change and record the new authority semantics in docs/development/perform-navigation.md.

## 2. Bind every Perform interaction to the same grid

- [x] 2.1 Route timeline availability, evidence, drag setup, snapped keyboard actions and target insertion/viewport helpers through the shared result. Extend native component tests for offset Play/Stop, seeks/loops, empty/stopped behavior, real drift, stale gestures, Ctrl/Shift, canonical bounds, missing wet/IR/output and unchanged saved assets. Verify every accepted snapped anchor lands on the captured grid while normal progress does not cancel a gesture; update affected canvas guidance.
- [x] 2.2 Carry the same authority and ownership through inspector knob/wheel invocations and their revalidation; audit accepted-grid ruler and exact musical conversion inputs without re-anchoring Learn/Export. Extend binding tests for canvas/knob/wheel/keyboard equality, exact unsnapped entry, reset, selection/session/source/mode/clock changes and preserved unrelated values. Verify existing KnobValue/Snap/grid controls and geometry are reused; document invocation semantics and any genuinely necessary owner control-review gate before rollout.
- [x] 2.3 Update existing diagnostic formatting/accessibility/Copy diagnostics to identify accepted grid versus validation reference and informational offsets versus genuine rejection. Extend text and native binding assertions for admitted +16 offset and subsequent rejected drift, closed/reopened Diagnostics, no saved-state/job mutation and contained readable text. Update docs/development/perform-live-diagnostics.md and apply maintain-user-guide with explicit semantic review and read-only currency verification.

## 3. Integration and local delivery evidence

- [x] 3.1 Extend processor/transport/persistence integration cases for shifted live-grid availability with unchanged positioned asymmetric audio, zero input, variable blocks, seeks/loops, save/reopen, zero latency and independent instances. Retain accepted offline offset/drift regressions and original live rejection tests. Verify no audio-policy change, no source/render identity rewrite, no tempo-map claims, no experimental protection or adjacent UI geometry adoption; record requirement coverage and replay assumptions in dated docs/validation evidence.
- [x] 3.2 Run full native Debug and Release via scripts/build.ps1 -Configuration Debug|Release -Test, UI standards, strict OpenSpec validation, user-guide currency and whitespace checks. Record exact source/dirty state, warm/fresh cache status, commands/logs, results, failures/recovery and remaining host/DPI limits; verify the final diff implements only this grid repair and preserves the combined offline fix. Prepare a Release test bundle and one concise same-project Snap-during-Play plus snapped-edit check without asking for repeated accepted bounce/audio evidence. Keep all work local.

## Workflow follow-up

- Proposal completion requires a fresh owner Apply request before implementation.
- Keep support-existing-tempo-maps / #71 separate; its local mapping/precision and persistence gates are not satisfied by this constant-tempo capture.
- If an updated local FL Studio installation is requested within delivery, use install-test-vst3 with the host closed, full bundle verification and retained previous bundle; installation is not host acceptance.
- Do not push, open a PR, merge, sync canonical specs, archive or close #73 without the corresponding owner-authorized review step. The older live rejection remains unresolved unless separately reproduced and scoped.
