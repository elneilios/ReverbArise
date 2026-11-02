# Shared mode header — issue #16

## Stage A: presentation and library components — 2026-11-02

Change: `shared-learn-perform-export-header`. Implementation branch:
`feat/16-shared-mode-header`. This stage introduces the presentation adapter and
isolated library controls; it does not yet replace the plugin's mode dropdown.

Verified Release checks in `out/build/windows-msvc`:

| Check | Result | Scope |
| --- | --- | --- |
| `preverb_workflow_state_test` | Pass, 0.04 s | Display mapping, 720 presentation/guard comparisons across modes/phases/readiness, existing independent eligibility and state-transition assertions |
| `preverb_workflow_processor_test` | Pass, 0.33 s | Explicit durable Capture/Live/Finalise XML round-trips plus display labels, metadata-free legacy policy, unchanged version/empty parameter inventory, retained source and existing host-save/routing checks |
| `preverb_capture_transaction_test` | Pass, 0.25 s | Existing atomic capture, explicit silence replacement, retained useful source and erased-last-useful-source regressions |
| `preverb_swell_ui_test` | Pass, 6.45 s (CTest 6.50 s) | Shared components plus existing native editor, knob wheel/typing/scrolling, selection and window regression fixture |
| Strict all-spec validation | 13/13 | Existing 12 specs and the new change |
| `git diff --check` | Pass | Changed source, docs and task formatting |

The guard chain is factored once into `workflow::entryBlock`; existing admission
and the presentation adapter consume it. Durable `modeName`, XML properties,
state versions, asset identities and mode/audio semantics are unchanged. Existing
production UI still uses the temporary dropdown until the next implementation stage.

Component tests use native JUCE mouse/key events, focus traversal, asynchronous
button messages and accessibility state/actions. They verify silent refresh,
model-owned selection, active/wheel no-ops, disabled reasons, request-time model
rejection, Tab traversal, arrow inspection, Enter/Space activation, callback
lifetime and semantic icon painting independent of component name. The test-only
message-loop helper is not enabled in the plugin.

The reusable state gallery is generated at 720x540 and 560x540 via
`PREVERB_UI_M3_OUTPUT`. The UI test executable supports `--review-controls` for
an interactive fixture that creates no audio processor and needs no FL Studio.
Local review artifacts are under `out/issue16-control-checkpoint-a/`.

Owner checkpoint A initially requested an adjustment: remove rounded corners
where segment options touch. The revised selector keeps rounding only at the
outer ends, with square internal joins and a shared border. Native geometry/pixel
checks verify this; existing interactions still pass. Revised review images:
`checkpoint-a-joined-states.png` and `checkpoint-a-joined-compact.png`.
**Revised component approval remains pending.** Task 2.4 is unchecked; no dependent
header integration, final host acceptance or complete library migration is claimed.

All results above are automated/native. No new FL Studio or actual Windows DPI
acceptance is claimed. The final host request remains one brief smoke test after
integration, with detailed edge cases proved by automation. The production Release
VST3 has not yet been rebuilt for delivery of this header change.
