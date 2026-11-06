# Issue #80 shared IR editor — implementation evidence

## Scope and baseline

Change: `shared-convolution-ir-editor`; branch: `codex/80-shared-ir-editor`.
Planning and Apply baseline: `dbfdb183f1dfd999c7f472093b135ab3221b8178`.
On 6 November 2026 `git fetch origin main` confirmed HEAD equals origin/main;
the connected GitHub search found no open #80 PR. The only other local active
change, completed `bound-waveform-paint-work`, does not own IR editor scope.
The user explicitly requested Apply after reviewing the planning artifacts.

## Execution map

| Task | Work / risk | Dependencies and verification |
| --- | --- | --- |
| 1.1 | Discovery/docs, low | Current main/PR/change inventory and native baseline below |
| 1.2 | Shared UI interaction, medium | Existing selector/icon/text policies; native component cases and UI lint |
| 1.3 | Async identity/binding, high | Component and existing engine guards; stale/lifetime/pass/freeze/import tests |
| 1.4 | Native family approval, medium | Tested component/binding; directly launched owner fixture before rollout |
| 2.1 | Learn binding, high | Family approval; Learn generation/pass/native cases and living guidance |
| 2.2 | Perform binding, high | Family approval; real editor/type/isolation/retained-output cases and guidance |
| 2.3 | Creation/provenance, high | Both consumers; explicit/Stop/manual ownership tests and guidance |
| 2.4 | Persistence/retained assets, high | Both consumers; save/reopen/import/handover and no-edit identity checks |
| 3.1 | Audio invariance, high | Integrated binding; deterministic PCM comparisons and existing admission paths |
| 3.2 | Integrated UI approval, medium | Both consumers and focused tests; compact/scale native composition review |
| 3.3 | Integration/guide gates, medium | Delivered behavior; guide skill, supported Release wrapper, UI/OpenSpec checks |
| 3.4 | Host acceptance, medium | Automated/native gates; test installer and one consolidated FL Studio smoke |
| 3.5 | Delivery, low | Scoped diff, evidence and one owner-reviewed issue PR |

## Affected ownership

- `Controls/IrSelector` / `DeliberateSelector`: retain deliberate no-wheel policy.
- New `Controls/ConvolutionIrEditor`: composite label/choice/file/status,
  accepted-state display, invocation catalogue, accessible description and help.
- New `ConvolutionIrBinding`: explicit default(session/pass) or selected
  swell(session/ID/input) targets, shared dispatch and safe chooser completion.
  Engine/importer remain validation and content authority.
- `LearnWorkspace` and `PluginEditor`: existing independent IR UI remains in
  production until family approval, then is replaced by these shared mechanisms.
- Existing diagnostic default-import recovery entry will route through the
  shared binding during integration. Unrelated Settings controls, envelope/Level,
  Export levels, audio engine, capture, cache and persisted formats are untouched
  at this family checkpoint.

## Verification

Environment: Windows x64, Visual Studio 2026 / MSVC 14.51, bundled CMake/CTest,
Release. This worktree initially had no build directory. The baseline wrapper
therefore used a cold configure/dependency/build, not a warm build or copied cache.

- Initial native build command: `./scripts/build.ps1 -Configuration Release -Test`.
  Logs: `out/build-logs/Release/20261106-141839-52f111e6/`.
  Result pending; do not infer success from configure/build progress. Configure
  began on unchanged product code. While dependencies compiled, the isolated
  family cases were wired into the existing UI test; this run therefore verifies
  the candidate too, not an untouched-main comparison. Product/engine code remains
  unchanged at this checkpoint. Existing main fixture oracles supply the baseline.
- UI lint after isolated family source creation passed: 63 files,
  60 existing explicitly bounded occurrences. No inventory regeneration.
- Isolated native family/binding cases passed (exit 0) on the current working
  candidate after adding bundled/managed command coverage. Entry points are
  `preverb_swell_ui_test --test-ir-editor` and `--review-ir-editor`. While the cold
  wrapper compiled other test executables, `out/ir-family-preview/build.cmd`
  compiled a thin runner for the same `tests/ConvolutionIrCases.h`, using the
  generated Release include/definition/link settings and completed libraries
  read-only. It compiled its own JUCE events translation unit with the test's
  modal-loop flag. All preview output stayed outside the active wrapper tree;
  no competing CMake/MSBuild operation touched that tree. This MSVC native result
  is distinct from the still-pending wrapper/CTest result.
- Cases cover notification-free accepted-state refresh, content-key catalogue
  reorder, keyboard choice, pointer/Return file action, no wheel reassignment,
  long labels/full help, compact geometry and unavailable state. Real engine
  binding cases cover future-default/selected isolation, bundled/managed/file
  commands, selection changes, cancel/decode failure/latest request, pass lock,
  input supersession, frozen/deleted targets, mode/session changes and binding
  destruction with a captured asynchronous completion.
- An initial fixture compared complete assets across separate imports of the
  same audio. Existing import behavior creates fresh filenames; the fixture was
  corrected to compare content identity for reimports and preserve exact asset
  comparisons for unchanged assignments. No importer behavior was changed.
- A final unavailable-default case verifies that a missing managed default says
  `Default IR unavailable`, rather than claiming its retained display name is
  accepted. The current source passed the same native cases in
  `ir-family-preview-next.exe` (exit 0). The original review executable remains
  running, so its Ready-state presentation is preserved; it predates only this
  unavailable-default feedback correction. An attempted relink of that running
  executable was refused by Windows; output was redirected to the second binary
  without closing or replacing the owner's review window.
- Strict OpenSpec validation and `git diff --check` passed. Full wrapper/CTest
  remains in progress and is not claimed passed at the family checkpoint.

## Approval and coverage

No family or integrated native approval has yet been received. No FL Studio
or physical Windows-DPI result is claimed. No production integration, audio
invariance result, guide currency review or release/test installation is complete.
The family checkpoint must be approved before wider production rollout.

The owner selected **Request changes to the family** at this checkpoint.
Specific appearance/interaction feedback has been requested and is pending;
task 1.4 remains open and production integration has not started. The existing
review window is retained. The initial full Release wrapper is still building,
with no CTest result yet available.

The prepared native review was launched directly (PID 64152) and its single
`ReverbArise control review` window was brought forward and inspected with the
computer-use tool. The 900x600/100% family view shows both real bindings, separate
scope/status, accessible choice/import controls and Ready/Frozen/Empty/Busy/Failed/
Locked plus 900/100%, 1100/125%, 1400/100% selectors. Busy/failure are labelled
read-only presentation specimens; Ready edits real isolated temporary fixtures.
The window remains open for owner review. This is a family checkpoint, not
integrated plugin composition or host/DPI approval.

## Owner refinements — field feedback and context ownership

The owner requested more closely grouped field feedback and red failures,
referencing Fluent UI's field pattern. The review candidate now places messages
`spacing::tight` below the input, wraps at the actual helper font without shrinking,
uses existing `danger` ink for actionable failures, and preserves readable feedback
when inputs are disabled. Redundant idle accepted/ready messages are omitted.
These scoped changes are documented as a review candidate, not an approved general
field-system migration. Actual helper font metrics and compact wrapping are tested.

The owner then clarified that `Future swells only` and `Selected swell only` belong
to the hosting screen. The shared component's scope property and scope child were
removed, along with scope-message assignment in the binding. The native preview
hosts those explanations beside its Learn/Perform section headings, outside the
IR fields. Production Learn/Perform integration must preserve that ownership.
The binding still owns context-specific target/session/pass validation; the control
receives only generic field data, availability and feedback.

The current source passed the native component/binding/host-ownership cases in
`out/ir-family-preview/ir-family-preview-host-scope.exe` (exit 0). Tests verify
both hosts use the same actual component type and scope labels have no shared IR
editor ancestor. UI lint, strict OpenSpec validation and whitespace checks passed.
The current source was built into a separate review executable. Relaunch was
held because elevated process inspection found the prior `fields-v2` review
still open (PID 68516), despite sandboxed inspection not exposing it. Computer
Use could locate that window but could not activate it: `GetCursorPos failed:
Access is denied. (0x80070005)`. The prior window was preserved and no duplicate
was opened; it does not yet show the host-scope refinement. Family acceptance
remains open (3/13 tracked tasks complete).
The initial full wrapper/CTest result remains pending; no host/DPI result is added.

After the owner reported that the desktop was accessible and requested the swap,
the old preview was closed normally and the host-scope revision was launched
directly (PID 69340). Computer Use verified the single revised native window:
scope explanations are under the hosting section headings, outside the IR fields,
and the Failed specimen retains close red inline feedback. The window remains
open for owner review; no family approval is inferred from requesting the swap.

The owner subsequently said **looks good**, approving the revised shared family
for Learn/Perform integration. This accepts the closer field feedback, existing
danger-colour failure treatment and host-owned scope explanation. Task 1.4 is
complete (4/13 tasks); integrated composition and final host acceptance remain
separate. The initial cold Release wrapper completed with **75/75 CTest passed**
(604.04 s CTest). That run precedes production integration and does not replace
the final integrated gates.

## Production integration and focused native evidence

Learn and Perform now instantiate the same `ConvolutionIrEditor` and
`ConvolutionIrBinding`. Their independent catalogue/chooser/status handlers were
removed, including affected legacy layout references. The Diagnostics default-IR
import action calls Learn's shared default binding. Existing component names are
retained where useful for accessibility/testing. Hosts own scope explanations;
Perform's exact Level and fixed Reset remain independent. Production standards
revision 8 records the approved field family using existing tokens. The sole
removed Learn `setMinimumHorizontalScale(1.f)` occurrence allowed deleting exact
debt entry `ui37-008`; the inventory was not regenerated (59 remaining occurrences).

Warm supported Release builds passed after integration. Canonical run logs are
under `out/build-logs/Release/20261106-154942-b8380dac/` (build only).
Focused native commands and results on the working candidate:

- `preverb_learn_generation_ui_test`: passed. The real shared dropdowns feed
  explicit Generate and eligible opted-in Stop transactions, isolate default and
  selected edits, retain manual creation snapshots and automatic-to-manualEdited
  provenance, and protect manual/edited/frozen definitions.
- The same test compares no-edit dry/raw/shaped PCM exactly: **589824 / 48004 /
  1924 float values equal**, with unchanged capture revision, saved swell settings,
  IR/render identities and render-job count after repeated production refresh.
- `preverb_perform_composition_preview --test-responsive`: passed, exercising
  supported compact/intermediate/larger geometry, both Learn tabs, all Perform
  sections, exact values, divider clearance and existing mode/settings routes.
- `preverb_perform_composition_preview --test-responsive-scale`: passed, covering
  actual presentation scaling/input/preferences/recovery. This is software/native
  presentation evidence, not a physical Windows-DPI or FL Studio result.
- The existing keyboard-Generate test formerly stopped dispatching after a 30 ms
  click-posting window, then blocked the GUI while waiting. Its wait now dispatches
  messages until the actual mode change and reports generation diagnostics on
  failure. The corrected test passed; no Generate behavior/guards were weakened.
- Existing geometry/feedback tests were updated for nested shared controls and
  the approved red inline failure state; an offscreen fixture's parent visibility
  is evaluated within Learn rather than against its intentionally hidden root.

No DSP, callback, persistence-format, cache, render scheduler or routing source
was modified. Existing audition/live/export/offline deterministic regressions
remain the broader audio/admission oracles; final full-suite evidence is pending.

The latest `preverb_swell_ui_test --test-ir-editor` and full
`preverb_swell_ui_test` both passed (exit 0). The latter's canonical focused log
is `out/issue-80-editor-native.log`, including Learn/default pending/failure,
selected retained replacement, freeze/empty, exact edits and many-swell states.
The shared binding cases additionally round-trip saved sessions, delete both
external IR originals, reopen retained imported choices without duplication,
reuse them through the shared command path and prove copied-instance default
edits preserve original settings and existing render references.

## Final automated gate

`./scripts/build.ps1 -Configuration Release -Test` used the preserved warm tree.
Configure/build passed. The full CTest run at
`out/build-logs/Release/20261106-155349-a5572ac0/` completed **74/75 passing** in
280.27 s. `preverb_processor_test` timed out once in the existing asynchronous
Live-handoff wait at `LearnPassPolicyCases.h:32`; its initial cold baseline had
passed. No product or fixture code was changed in response to this timeout.

The isolated `preverb_processor_test learn-policy` passed, then the **complete**
processor test passed via CTest `--rerun-failed` in 11.79 s (11.81 s total).
Canonical retry log: `out/issue-80-processor-retry.log`. All 75 tests therefore
have passing results on this candidate; do not describe the original full run
as 75/75 green. The timeout's cause is not established and remains recorded.

The suite includes the unchanged deterministic reverse-convolution, shaping,
source-reader, audition, live transport, processor offline/export and persistence
oracles. Together with the exact no-edit PCM comparison and unchanged DSP/callback
diff, this verifies baseline sound/content and separate admission/routing behavior.
No callback I/O, blocking, allocation or new processing stage was introduced.

Strict OpenSpec validation, UI standards (59 remaining bounded occurrences),
whitespace checks and explicit offline-guide review/currency passed. Guide prose
now explains shared IR controls, host-owned scope, accepted/requested feedback,
red failure recovery and freeze guards. Existing Capture/Timing guide screenshots
remain representative. No host/DPI acceptance is inferred from automated results.

## Integrated native checkpoint

The prepared production review was launched directly with
`preverb_perform_composition_preview --review-ir-integrated` (PID 47548), title
`Issue #80 - Shared IR integrated review`. It starts both Learn and Perform on
Sound, supplies sparse/empty/dense fixtures and supported size choices, and uses
the real Settings Appearance path for 125% user scale. No audio device is opened.
Computer Use inspected both modes at 900x600/100%: host-owned scope copy, shared
IR picker/import, existing exact Level and fixed Reset remain readable and
bounded. The window is left open on Learn Sound for owner review. Task 3.2
awaits explicit integrated acceptance; physical Windows-DPI and FL Studio results
remain unclaimed. Tracked progress is **10/13 complete**.

### Integrated review feedback — command-button hover

The owner accepted the layout/composition, but identified the folder button's
inconsistent hover and requested a correction plus a deferred wider audit.
`IconActionButton` had an independent painter: hover recoloured the glyph aqua
while leaving the outline neutral. `ActionButton` instead retained semantic
content colour and highlighted the outline. Both now call
`ActionButton::paintSurface`, so folder/import and Settings glyph buttons share
the ordinary command-button idle/hover/pressed/focus/disabled surface.
No new token is introduced; production standards revision 9 records this scope.
Native rendering parity tests cover both glyph roles against a normal command
button, while ensuring hover does not recolour glyph content.

The targeted wider audit is [#107](https://github.com/elneilios/ReverbArise/issues/107),
created at the owner's request after checking for a matching open issue. It
tracks remaining name-based legacy button styling and semantic state ownership;
no unrelated whole-plugin migration is included in #80.

The previous integrated window was closed normally for the correction rebuild.
Task 3.2 retains the layout acceptance but awaits the corrected native interaction
checkpoint. Task 3.3 was reopened because the shared button code changed after
its earlier verification; the earlier 74/75 plus processor retry remains valid
historical evidence, not the final hover-correction result.

The correction passed the supported warm Release build
(`out/build-logs/Release/20261106-161712-4702e17a/`), native shared-family/button
state cases, full `preverb_swell_ui_test` (`out/issue-80-hover-editor.log`) and
`preverb_perform_composition_preview --test-responsive-scale`. State checks
compare stable straight-edge palette samples on the opaque workspace and verify
that both icon glyphs stay neutral across idle/hover/pressed/disabled states;
curved-corner antialiasing is not treated as semantic state evidence.
UI lint, guide currency and whitespace checks passed. Guide prose remains correct
without additional musician-facing changes for this consistency correction.

The corrected integrated review was reopened directly (PID 68224). Computer Use
verified actual folder hover at 900x600: an aqua outline with an off-white glyph,
matching ordinary command buttons; no file chooser was activated. The window is
left open for the scoped hover check. The broader full-suite gate will be refreshed
after that review, avoiding interference with the owner's active native interaction.

### Integrated review feedback - idle IR-to-Level gap

The owner approved the corrected folder hover, then identified excessive blank
space before Level. The composite reserved two feedback lines even when its
accepted/ready status was empty. The revision removes that reservation: no
message means no message height; present feedback occupies one or two lines at
the normal helper font. Hosts measure at their actual field width and reflow
Level on status transitions. Existing content padding separates Level; Reset
remains outside the scrolling fields. Component tests cover collapsed and wrapped
heights, and actual responsive editor checks assert Level follows the composite
with exactly the named padding. Standards revision 10 records the refinement;
native spacing acceptance remains pending.

The initial sandboxed wrapper attempt (`20261106-162816-172de207`) stopped at
configure because Windows SDK registry access was denied. The supported wrapper
was restarted with the required native permissions; no cache was cleared.

The owner accepted the revised spacing ("looks better") after explicitly
approving the corrected hover. This completes the integrated native composition
checkpoint (task 3.2), retaining the earlier layout and family approvals.
Physical Windows-DPI and FL Studio coverage remain separate and unclaimed.

The warm native Release build passed (`20261106-163113-882ca5fd`). Focused
shared-field/binding cases, actual responsive bounds checks, responsive-scale
checks and the full native editor suite (`out/issue-80-gap-editor.log`) passed.
The first iteration caught a test-only JUCE rectangle API error, corrected before
the passing build; the initial focused field check then exposed a fixture still
using the old fixed height, corrected to use the host sizing contract. An initial
sandboxed responsive run failed fixture admission; the same unchanged native
check passed with normal Windows runtime permissions. UI standards, guide
currency, strict OpenSpec and whitespace checks passed.

The revised integrated preview was launched directly (PID 19268); owner review
completed even though Computer Use inspection was stopped with Escape. The final
full wrapper gate is refreshed after this acceptance.

### Final post-approval native gate

`./scripts/build.ps1 -Configuration Release -Test` passed on the warm existing
MSVC tree: **75/75 CTest tests passed**, zero failures, 227.99 seconds. Logs:
`out/build-logs/Release/20261106-163338-3e0825a7/`. This is the final revised
field/button implementation at baseline `dbfdb18` plus the issue's dirty diff,
not the earlier 74/75 run with retry. No cache was cleared and no timing speedup
is claimed. UI standards (63 files, 59 bounded occurrences), guide currency,
strict OpenSpec validation and whitespace checks passed. The guide's scoped
semantic review needs no further prose for the spacing-only refinement.
Task 3.3 is complete; remaining required acceptance is one FL Studio smoke,
followed by issue-branch commit/PR delivery.

### Final host-test installation

Applied `install-test-vst3` using `scripts/install-test-vst3.ps1` with FL Studio
closed. The first normal-permission attempt built successfully but could not open
the system installation lock (ACL denial). The prescribed UAC-elevated rerun
succeeded; no host was terminated, ACL changed or installation manually copied.
Warm plugin-only Release build passed; CTest was not repeated by the installer
(the final 75/75 gate above already passed). Installer logs:
`out/issue-80-vst3-install.log`; build logs:
`out/build-logs/Release/20261106-163844-d9187272/`.

Installed bundle: `C:\Program Files\Common Files\VST3\ReverbArise.vst3`.
Previous bundle retained at
`out/vst3-install-backups/20261106-163852-36605cb56ec4461eb3d5c8678e4937b4/ReverbArise.vst3`.
Installed binary SHA256:
`206366BACA798EB179B7BA2B18B055EE283D75F617CA7D3E00CF5945398214B1`.

The owner was asked for one final FL Studio smoke: verify/rescan the system
bundle, inspect both Sound contexts, play audio and save/reopen a test project.
This host acceptance is pending, so task 3.4 stays open. No extra automated
edge-case or native layout acceptance was requested again.
