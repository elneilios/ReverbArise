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
The owner explicitly approved the revised components and continuation to header
integration on 2026-11-02. This closes component checkpoint A (task 2.4), not the
integrated header checkpoint, final host acceptance or whole-library migration.

All results above are automated/native. No new FL Studio or actual Windows DPI
acceptance is claimed. The final host request remains one brief smoke test after
integration, with detailed edge cases proved by automation. The production Release
VST3 has not yet been rebuilt for delivery of this header change.

## Stage B: integrated header — 2026-11-02

The shared WorkflowHeader now replaces the temporary dropdown and owns the
wordmark, italic tagline, library selector, semantic Settings action and concise
role/status text. It uses the approved square internal joins. The old name-based
Settings glyph branch and string-returning guard adapter are removed after their
consumers migrate; one typed guard remains authoritative. Functional source
capture/live wet wording is retained. The targeted old-mode-wording audit finds
only the durable `modeName()` strings and legacy XML restore parser.

The expanded native header fixture checks ten states at three widths (1068,
1360,1560, matching editor content widths) with real native peers: empty,
dry-only, ready, recording, draining, validating, failure, incompatible timing,
frozen recovery and Export. It asserts child containment/no overlap and segment
availability/reasons. Existing editor fixtures check actual pointer/keyboard mode
requests, active-choice no-op, recording locks, selection-edit invalidation and
Settings/Diagnostics lifecycle with the new controls.

The first standalone header fixture incorrectly queried accessibility without
a native peer and hit a null handler. Giving that fixture a real peer and checking
the handler fixed it; the native UI test then passed. This was a fixture setup
failure, not a verified host crash. Native snapshot tracing remains available in
test output to identify future failing stages.

Integrated focused Release tests passed (workflow processor/state and native UI).
Full Release build and CTest subsequently passed **32/32 in 41.29 s**. Strict
all-spec validation passed **13/13**, and patch formatting checks passed. The
additional changed Debug fixtures also passed **4/4 in 7.96 s** (workflow state,
workflow processor, capture transaction and native UI, including assertions).
This is focused Debug coverage, not a full Debug-suite claim.

Delivery bundle:
`out/build/windows-msvc/preverb_plugin_artefacts/Release/VST3/ReverbArise.vst3`.
Its `Contents/x86_64-win/ReverbArise.vst3` SHA-256 is
`3a6fd0996b60a6e47d74b8dad9556392d46d45b6c30371a4b070eec438a1eb24`.
Generated module metadata retains component
`ABCDEF019182FAEB4B374D6150727631` and controller
`ABCDEF011234ABCD4B374D6150727631`. No parameter/state version/asset migration is
introduced; saved-mode compatibility assertions still pass.

Native evidence is under `out/issue16-header-checkpoint-b/`: matched Perform,
Learn and Export full-editor captures, a larger layout, header-state strips and
125/150/200% offscreen scale renders. These are geometry/native evidence, not
in-host DPI or FL Studio acceptance. **Integrated checkpoint B needs adjustment:**
the owner requested changes; detailed header feedback is pending. Task 3.5 remains
open and no final host acceptance has been requested.
The final owner host request is limited to one brief feel/mode-journey/save-reopen
smoke test after visual approval; other edge cases rely on automated regressions.

Committed native header examples at the minimum content width:

![Learn header](images/shared-header-learn.png)

![Perform header](images/shared-header-perform.png)

![Export header](images/shared-header-export.png)

## Stage C: typography and conditional-notice refinement — 2026-11-02

The owner approved the header layout/buttons but requested larger, heavier mode
labels, an italic mode description and removal of routine source-protection text.
They then explicitly chose a conditional inline important-message bar below the
header, with temporary reflow rather than overlays. The plan/delta/tasks now
include this refinement; final integration/build checks were reopened because
the earlier full-suite and VST3 hash describe the pre-refinement build.

The native component stage uses 18-pixel bold navigation and a 13-pixel italic
description. Operational status is removed from the header. The new library
NoticeBar accepts one message/detail/severity and has information, warning and
error styles. It hides on no message, retains keyboard focus elsewhere, avoids
duplicate announcements on identical refresh and keeps detail accessible.

Focused Release verification passed: workflow-state and native-UI tests, followed
by the final native-UI fixture at **14.25 s** (CTest 14.34 s). The fixture covers
the typography/header geometry and NoticeBar visibility, severity pixels,
accessible title/detail, repeat suppression and nonmodal/focus behavior. This is
not a claim that the earlier 32/32 Release or 4/4 Debug results cover the final
message-bar integration; that binding is not implemented yet.

Review example: `out/issue16-message-bar-checkpoint/checkpoint-c-controls.png`.
The owner requested direct preview launch for future UI/component approvals;
this is recorded in AGENTS.md and the UI development standards. The native
`--review-messages` window was opened directly, without FL Studio. The owner
requested further styling adjustments; details are pending. Component checkpoint
3.6 and production notice binding 3.7 remained open at that review. No new host acceptance was claimed.

### Refined component approval — 2026-11-03

The owner approved the revised native preview: true Segoe UI Semibold mode
labels, Segoe UI Regular body text, brighter off-white secondary text and a
shared centerline for logo, tagline, selector and Settings. The italic mode
description sits independently below the selector. Message-bar styling was
approved in the earlier review. Checkpoint 3.6 is complete; this is native visual
approval, not new FL Studio/DPI acceptance. Native UI verification passed in
6.86 seconds, including resolved typeface styles and alignment assertions.
Review capture: `out/issue16-message-bar-checkpoint/checkpoint-c-centered-regular.png`.

### Production notice integration — 2026-11-03

`WorkspaceNotice` projects explicit workflow/capture/action/IR error fields and
the existing offline-result prefixes into one current notice. Failures precede
recording/settlement and contextual Recapture/Export guidance. Routine idle Learn
and Perform stay quiet. `PluginEditor` inserts/removes the 32-pixel row only on
presence changes; bottom bands remain anchored, and model edits remain unchanged.
Tests cover priority/resolution, minimum/larger geometry and real mode/Recapture
bindings. Native component tests retain focus and repeated-refresh checks.

- Release: **32/32 passed, 40.78 seconds**.
- Focused Debug workflow processor, capture transaction, workflow state and
  native UI: **4/4 passed, 6.43 seconds**.
- Strict OpenSpec: **13/13 passed**; `git diff --check` passed.
- Release VST3 rebuilt at the existing artifact path above. SHA-256:
  `72308283e5912bd84040b7b5a5a11c2c7e286cefd5640fa47d04013beef18c7e`.
- Component/controller CIDs remain
  `ABCDEF019182FAEB4B374D6150727631` /
  `ABCDEF011234ABCD4B374D6150727631`.
- Captures: `out/issue16-integrated-notices/`, including Learn/Perform/Export,
  Recapture, recording, settlement, failure and 1100/1400/larger layouts.
  Ready/fresh captures include 125%, 150% and 200% software transforms;
  these do **not** establish actual in-host DPI behavior.

The real processor/editor fixture was opened directly with `--review-workspace`.
The owner requested adjustments before host testing; details are pending, so
integrated checkpoint 3.8, final host check 4.3 and PR completion 4.4 remain open.
The preview is preserved. No new host/DPI acceptance is claimed. Reopen affected
build/regression tasks if the requested adjustments change implementation.

### Reserved header notice row — 2026-11-03

The owner requested relocating notices beneath the logo, reserving space so one
notice does not move the workspace, and retaining the divider above global
settings. The header now reserves 82 pixels: the original primary-row alignment
is preserved, with a 32-pixel notice area beneath the branding and left of the
mode description. The full divider sits below the reserved row. Notice visibility
no longer changes geometry. The existing single-highest-priority notice policy
remains; additional simultaneous messages, if later introduced, may grow the
header rather than overlap content.

The revised native UI test passed in **6.39 seconds** (CTest 6.45), including
geometry invariance across notice visibility, no intersection with header
content, and actual divider-pixel assertions across native editor states.
Strict OpenSpec validation passed **13/13**. Captures and scale transforms are
in `out/issue16-reserved-notice/`. A new real-editor preview was opened directly;
layout approval is pending. Final Release build/regression evidence above is
superseded for delivery until the reserved-row rebuild completes.

The reserved-row Release rebuild completed: **32/32 tests passed, 35.46 seconds**.
VST3 SHA-256:
`59f25c8dd816f4e019d00b7c0801b3b0078ff784c6405e011e1b923f1b1d2c5e`.
This supersedes the previous delivery hash. The owner requested further visual
adjustments; their details are pending. Integrated approval and host acceptance
remain open, and affected verification will be reopened for further code changes.

The owner approved the reserved message-bar positioning and requested a larger
mode description aligned with that row. It now uses 16-pixel Segoe UI Italic
(previously 13) and shares the notice row's vertical center, independently of the
primary logo/tagline/control centerline. Native UI tests passed in 6.68 seconds
(CTest 6.73), including actual editor alignment and font-height assertions;
strict OpenSpec passed 13/13. Review captures: `out/issue16-description-aligned/`.
A revised native editor preview was launched directly. Final integrated approval
is pending, and final regression/build tasks remain reopened for this revision.

The owner approved this final integrated layout on 2026-11-03 and requested
continuation to final checks. This closes checkpoint 3.8; it does not claim new
FL Studio or physical DPI validation. The native preview remains available.

### Approved delivery build — 2026-11-03

Final approved typography/alignment and reserved message row are included in:

- Release: **32/32 tests passed, 41.28 seconds**.
- Focused Debug: **4/4 passed, 7.07 seconds** (workflow processor/state,
  transactional capture and native editor/component assertions).
- Strict OpenSpec: **13/13 passed**; patch formatting check passed.
- Release binary SHA-256:
  `6fc0cfa0054173de1d085139dd3ab37bfc138502c1777095546bb88d477de889`.
  This supersedes earlier delivery hashes in this chronological record.
- Generated component/controller IDs remain unchanged as recorded above.
- Final native captures and 125/150/200% software scale renders are under
  `out/issue16-final-approved/`; the three committed Learn/Perform/Export images
  above now show this final build. Physical host/DPI coverage is still untested.

All component and integrated visual approvals are recorded. Only the brief final
FL Studio mode/control-feel/save-reopen smoke and final PR evidence completion
remain open; no detailed automated edge-case repetition is requested.

### Final host acceptance — 2026-11-03

The owner reported **Pass** for the single consolidated FL Studio smoke:
reloading the rebuilt VST3, Learn → Perform → Export → Perform, header/Settings
control feel, and saving/reopening with the mode preserved. This completes task
4.3 and the requested host acceptance without repeating automated audio/capture/
export edge cases. No additional physical monitor/DPI configurations were tested.
The old Preverb Spike automatic-discovery migration remains unverified: the owner
previously replaced the effect manually. Plugin identities and durable mode/state
keys remain unchanged; no automatic relink claim is made.

All component and integrated appearance feedback is resolved and approved. Shared
controls delivered here are SegmentedSelector, semantic Settings IconActionButton
and NoticeBar, composed by WorkflowHeader and the snapshot notice adapter. Other
control families retain their existing implementations for incremental workspace
refactors; this is not a whole-plugin library migration. Issue #11's dedicated
Learn workspace remains separate. PR #24 is left for owner review and merge.
