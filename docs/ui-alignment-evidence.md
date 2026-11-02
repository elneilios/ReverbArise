# ReverbArise UI alignment evidence

Change: `align-reverbarise-ui-with-approved-design`.
The source of visual authority is `style/ReverbArise_UI_Layout_and_Style_Guide.md`
and the current Init/SamplesLoaded pair in `prototypes/`. This log separates
automated/native evidence from user approval and physical host acceptance.

## Approval checkpoints

The user requested visual approval before proceeding between stages.

| Checkpoint | Review scope | Approval |
| --- | --- | --- |
| A | Supplied logo, native typography/theme, eight-region composition, minimum/default sizes | Approved by user: Approve this direction; continue. |
| B | Inspector/control density, every canvas/knob/value pair, source shading, list, Reset and IR chooser | Pending; stop before final visual polish |
| C | Audition, Overview, Settings/diagnostics, all loaded/empty/error states | Pending; stop before final host acceptance/archive |

An offscreen test passing does not approve a checkpoint. Record the user's actual
response and any requested redirection here; do not infer approval from silence.
These intermediate reviews do not complete task 10.1 until the final native
empty/loaded composition is accepted.

## Baseline and compatibility

Baseline Git commit: `c1f7e6efeaad765b49bebef2c1050d5abb2b1009` plus the supplied
uncommitted planning/logo pack. Host identity remains `Preverb Spike`, bundle ID
`com.k7ma.preverb`, manufacturer `K7Ma`, plugin code `Prv1`; C++20/JUCE 9.0.4,
Windows x64 VST3. No host parameters are registered; one program is a stub.
XML `PreverbState` and swell JSON are version 7. No cache path, UUID, asset
checksum or format migration is planned.

The native harness now has independent fresh/no-capture and mapped/unselected
fixtures; the previous file named empty.png retained capture and is not a fresh
baseline. Selected fixtures retain exact ID/IR/render/shape checks around editor
construction. Existing processor/session tests cover legacy state/tone and
current asset reopening; results are recorded below after execution.

From the repository root in PowerShell:

```powershell
$cmake = "$env:ProgramFiles\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$ctest = Join-Path (Split-Path $cmake) "ctest.exe"
& $cmake --build --preset windows-msvc-debug --target preverb_swell_ui_test
$env:PREVERB_UI_M3_OUTPUT = "$PWD/out/ui-alignment-baseline"
& $ctest --preset windows-msvc-debug -R '^preverb_(swell_ui|processor|swell_session|persistent_capture|transport_playback)_test$'
Remove-Item Env:PREVERB_UI_M3_OUTPUT
```

Snapshots are native JUCE SoftwareImageType output, not HTML or mockup imagery.
They do not establish real FL Studio DPI/window embedding or musical acceptance.

## Requirement and diagnostic evidence checklist

All checks below start pending. The detailed current-control inventory and
dispositions are in the change's design.md, decision 8; task group 8 verifies
each original item, its retained/moved/retired binding and its consumer tests.

| Requirement | Task ownership | Expected evidence | Status |
| --- | --- | --- | --- |
| G01 eight stable regions | 3.1/3.3 | Region bounds across actual empty/loaded states | Pending |
| G02 flat surfaces/rules | 2.1/3.2 | Colour and border assertions plus native images | Pending |
| G03 brand/type/icons | 2.2-2.4 | Supplied vector/native font at sizes/scales | Pending |
| G04 global sensitivities | 4.1 | Independent values/restore/paused states | Pending |
| G05 list/names | 4.2/4.3 | 500 rows, custom names, bar timing fixtures | Pending |
| G06 per-ID IR | 6.3 | Bundled/file/custom default, chooser races | Pending |
| G07 aligned timeline/target | 5.1 | Sample/pixel oracle with actual shape | Pending |
| G08 shaded source selection | 5.2 | Both lines/handles, sparse coverage and edit parity | Pending |
| G09 all inspector pairs | 6.1/6.2 | Each knob/field/canvas roundtrip and precision | Pending |
| G10 gesture safety | 5.3 | Extreme/zero/full/far/close fixtures | Pending |
| G11 placement/support | 5.1/6.1 | Checked transforms, real pending support, raw identity | Pending |
| G12 continuous audition | 7.1/7.2 | Percentage geometry, samples, legacy mode projection | Pending |
| G13 cursor/isolation/tail/trim | 7.3 | Existing safeguards and actual playback cursor | Pending |
| G14 truthful Overview | 5.4 | Ruler/viewport/sparse navigation oracle | Pending |
| G15 host info | 7.4 | Unknown/seek/stopped readouts | Pending |
| G16 automatic state | 4.4/7.4/8.1 | Fresh-use and stale/error native states | Pending |
| G17 diagnostics | 8.1-8.6 | Every disposition + lifecycle/no-side-effects | Pending |
| G18 reset | 6.4/6.5 | Exact defaults, scope, transactional failures | Pending |
| G19 compatibility | 1.2/9.1/9.4 | Host/parameter/state/asset and audio regressions | Pending |
| G20 sizes/scales | 3.4/9.3/10.4 | Logical snapshots and separate real host matrix | Pending |

## FL Studio acceptance

Pending tasks 10.2-10.4: record FL Studio version, audio driver/sample rate/buffer,
Windows scale and monitor work area with the actual build identity. Verify
resize/focus/menu/chooser/diagnostics lifecycle, source/wet inspector editing,
Freeze/Solo/IRs/reset/names, live transport and two-instance export, and reopen.
Do not equate previous milestone acceptance or software scaled snapshots with
new host checks. Unavailable physical scales remain open unless the support
policy is explicitly revised with the user.

## Run records

No visual checkpoint or new host check is approved yet.

### Baseline run

Baseline native build passed. Session, transport, processor and persistent-capture CTest cases passed (4/4); the corrected UI executable passed independently and generated fresh-1100/fresh-1400, mapped-unselected and selected native baselines. The first new fixture exceeded the Windows stack with its inline capture queue; heap allocation matching existing fixtures fixed it. No production appearance or audio code was changed before these captures.

### Checkpoint A build and native verification

Status: **awaiting explicit user visual approval**. Implementation is paused at
this boundary; detailed eleven-control inspector/IR/reset/source-line work and
final Settings/diagnostics cleanup remain pending.

- Debug native UI and VST3 builds passed.
- `ctest --preset windows-msvc-debug -R '^preverb_swell_ui_test$'`: 1/1 passed,
  14.21 seconds. Covers fresh/mapped/selected fixtures, fixed region rectangles
  through all existing edit/error/diagnostic snapshots, Inter/SVG loading,
  background pixels, Settings reachability and preserved engine edit behaviour.
- Strict OpenSpec validation: 10 items passed, zero failures.
- Native 1x images: `out/ui-alignment-stage-a/fresh-1400.png`, `fresh-1100.png`,
  `mapped-unselected.png`, `ready.png`, plus existing edit/size/error fixtures.
- Native software scaled images for fresh/default selected at 125%, 150%, 200%
  are saved alongside them. These are not physical Windows/FL Studio DPI tests.
- Regular Inter loaded from supplied TTF; bold/italic roles explicitly use
  system Segoe UI. Supplied dark SVG is embedded unchanged; Inter-OFL.txt is
  present in bundle Resources.
- Bundle: `out/build/windows-msvc/preverb_plugin_artefacts/Debug/VST3/Preverb Spike.vst3`.
- Nested binary: 33,611,264 bytes; SHA-256
  `498f7154f24065f6231fe36ea590a8fee42512dd458f91d9b8dac7b450720fd4`.

Tasks 1.1-1.3, 2.3, 3.1, 3.2 and 3.4 are complete (7/45). General reusable
controls/theme completion, the full inspector and final integrated/host checks
are not marked complete merely because this shell is ready to review.
No new user or host visual acceptance has been inferred.

### Checkpoint A approval

The user explicitly approved the direction and authorized continuation into checkpoint B. This approval covers the shell/branding/typography/composition, not final inspector controls, host validation or completed change acceptance.


## Checkpoint B implementation and native evidence (2026-11-01)

Eight additional tasks are verified: 3.3, 4.3, 4.4, 5.3, 6.1, 6.4, 6.5 and
6.6. OpenSpec now reports 15/45 complete. The CLI source line was checked and
apply instructions refreshed after every individual checkbox update.

The selected inspector has eleven paired controls, grouped into six rows with
independent scrolling; Enabled, Solo, Freeze, actual per-swell IR and the Reset
split action are visible in the initial view. Source boundaries use full-height
neutral lines/circular grips and a flat translucent span; the selected target is
yellow/dashed with its own top marker. Wet handles are outlined circles without
changing the accepted stacking/centre-line/gesture geometry. New manual and
automatic names derive from arrival's Playlist bar; saved/custom names are kept.
See ui-theme-and-layout.md for units and reset/assignment semantics.

Verified builds: native UI harness, session, engine, automatic-target tests and
Debug VST3. Focused CTest run: 4/4 passed in 20.56 seconds. The final UI-only
rerun after the source/timing supersession guard passed in 14.86 seconds.
No claim of the full configured suite or host acceptance is made at this gate.
Strict OpenSpec validation: all 10 items passed. git diff --check passed.

New focused assertions include eleven reachable paired cells; typed signed
pre-roll/endpoint placement inversion; invalid scalar/overflow rejection;
Escape cancellation; selection changes cancelling an in-progress knob gesture;
exact odd samples above 2^53 and checked relative source edits; bar names,
same-bar UUID independence and custom-name preservation; pinned IR assignment
across selection/generation/deletion/freeze; precise wet-reset defaults; and
transactional Reset all source/IR/activity restoration with missing-IR failure
leaving the saved definition untouched. Existing extreme/stacked/zero/full fade
and wet-body gesture fixtures continue to pass.

Two reset edge cases found by the new tests were corrected: preserve 20 ms
creation release before a raw render exists, and explicitly prepare Reset all
input changes even when master automatic processing is paused. Validation
failure feedback is retained separately from background monitoring status so
an unrelated preview update cannot immediately hide it.

Native JUCE software review images in out/ui-alignment-stage-b/:
- inspector-default.png: default selected-swell composition.
- inspector-scrolled.png: lower tension/level controls and support readout.
- envelope-1100.png: minimum 1100x850 composition, real edited envelope.
- ready.png, mapped-unselected.png, fresh-1100.png, fresh-1400.png and the
  existing gesture/support/error/long-list fixtures are regenerated as well.
Software 125/150/200% images are simulated rendering only, not physical host DPI.

Final nested VST3 binary: 33,769,472 bytes; SHA-256
fe1ecd3b2cbf40e4751d7d6ea6318257c6304f1b6c2bb03455a175178c9e3f5c.
Bundle location remains out/build/windows-msvc/preverb_plugin_artefacts/Debug/
VST3/Preverb Spike.vst3. No installation, commit or push was performed.

Checkpoint B visual approval is pending. Settings/recovery currently use the
interim overlay and retain legacy precise-bound/preset controls. Final scoped
Settings, dedicated diagnostics, remaining control interaction/race coverage,
complete theme/transform/overview verification and audition polish remain open
for the next stage; physical chooser/DPI/FL Studio checks remain unclaimed.
Review warning precedence when consolidating action/export feedback in task
7.4 so a reset validation error cannot obscure an export warning.
