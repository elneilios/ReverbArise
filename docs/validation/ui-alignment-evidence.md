# ReverbArise UI alignment evidence

Change: `align-reverbarise-ui-with-approved-design`.
The source of visual authority is `../design/ReverbArise_UI_Layout_and_Style_Guide.md`
and the current Init/SamplesLoaded pair in `../archive/prototypes/`. This log separates
automated/native evidence from user approval and physical host acceptance.

## Approval checkpoints

The user requested visual approval before proceeding between stages.

| Checkpoint | Review scope | Approval |
| --- | --- | --- |
| A | Supplied logo, native typography/theme, eight-region composition, minimum/default sizes | Approved by user: Approve this direction; continue. |
| B | Inspector/control density, every canvas/knob/value pair, source shading, list, Reset and IR chooser | Approved by user after two review rounds: rename moved to a list-row menu; Reset/Reset All are red text buttons beneath the knob grid; IR loader is a folder-open icon; time values show 3 decimals; inspector debug text removed; header status moved into the globals band; tagline vertically centred. Evidence: out/ui-alignment-stage-c/ (inspector-default, inspector-scrolled, ready, envelope-1100). |
| C | Integrated final composition: loaded UI, empty UI and Diagnostics | Approved by user on 2026-11-02: "Approve visuals; proceed to host validation". Reviewed out/ui-alignment-final-native/ready.png, fresh-1100.png and dedicated-diagnostics.png. |

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

The table below records current native verification; host acceptance remains separate. The detailed current-control inventory and
dispositions are in the change's design.md, decision 8; task group 8 verifies
each original item, its retained/moved/retired binding and its consumer tests.

| Requirement | Task ownership | Expected evidence | Status |
| --- | --- | --- | --- |
| G01 eight stable regions | 3.1/3.3 | Region bounds across actual empty/loaded states | Native verified; user acceptance approved 2026-11-02 |
| G02 flat surfaces/rules | 2.1/3.2 | Colour and border assertions plus native images | Native verified; user acceptance approved 2026-11-02 |
| G03 brand/type/icons | 2.2-2.4 | Supplied vector/native font at sizes/scales | Native verified; user acceptance approved 2026-11-02 |
| G04 global sensitivities | 4.1 | Independent values/restore/paused states | Native verified; user acceptance approved 2026-11-02 |
| G05 list/names | 4.2/4.3 | 500 rows, custom names, bar timing fixtures | Native verified; user acceptance approved 2026-11-02 |
| G06 per-ID IR | 6.3 | Bundled/file/custom default, chooser races | Native verified; user acceptance approved 2026-11-02 |
| G07 aligned timeline/target | 5.1 | Sample/pixel oracle with actual shape | Native verified; user acceptance approved 2026-11-02 |
| G08 shaded source selection | 5.2 | Both lines/handles, sparse coverage and edit parity | Native verified; user acceptance approved 2026-11-02 |
| G09 all inspector pairs | 6.1/6.2 | Each knob/field/canvas roundtrip and precision | Native verified; user acceptance approved 2026-11-02 |
| G10 gesture safety | 5.3 | Extreme/zero/full/far/close fixtures | Native verified; user acceptance approved 2026-11-02 |
| G11 placement/support | 5.1/6.1 | Checked transforms, real pending support, raw identity | Native verified; user acceptance approved 2026-11-02 |
| G12 continuous audition | 7.1/7.2 | Percentage geometry, samples, legacy mode projection | Native verified; user acceptance approved 2026-11-02 |
| G13 cursor/isolation/tail/trim | 7.3 | Existing safeguards and actual playback cursor | Native verified; user acceptance approved 2026-11-02 |
| G14 truthful Overview | 5.4 | Ruler/viewport/sparse navigation oracle | Native verified; user acceptance approved 2026-11-02 |
| G15 host info | 7.4 | Unknown/seek/stopped readouts | Native verified; user acceptance approved 2026-11-02 |
| G16 automatic state | 4.4/7.4/8.1 | Fresh-use and stale/error native states | Native verified; user acceptance approved 2026-11-02 |
| G17 diagnostics | 8.1-8.6 | Every disposition + lifecycle/no-side-effects | Native verified; retained-UI lifecycle approved 2026-11-02 |
| G18 reset | 6.4/6.5 | Exact defaults, scope, transactional failures | Native verified; user acceptance approved 2026-11-02 |
| G19 compatibility | 1.2/9.1/9.4 | Host/parameter/state/asset and audio regressions | Native verified; user acceptance approved 2026-11-02 |
| G20 sizes/scales | 3.4/9.3/10.4 | Logical snapshots and separate real host matrix | Native verified; user host/DPI acceptance approved 2026-11-02 |

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


## Popup and Settings fixes after Copilot handoff (2026-11-02)

Completed Copilot's partial separate-window work rather than reverting the
compact inspector changes. The Swell row menu now retains the event component
for scale/lifetime but explicitly anchors its target rectangle to the click's
screen position. The previous whole-list target rectangle placed it far from
the three dots. Rename/Delete remain pinned to the captured UUID.

The gear now opens an editor-owned Settings window with Playback, Processing,
Swell, Audition and Recovery tabs. Normal settings are grouped and labelled;
none are allocated over the main workspace. Recovery > Open diagnostics opens a
separate editor-owned diagnostics window. Close hides each window; editor
teardown destroys them before the theme. Diagnostic text formats only while
visible, at most 5 Hz, with immediate refresh on reopen. Legacy diagnostic
bound/shape tools remain pending the separately tracked consolidation.

Native tests locate controls through their actual tab/window ownership, assert
unchanged session JSON/render count/capture revision and main-region geometry,
close/reopen diagnostics, verify hidden diagnostic text does not refresh, and
open a real native popup from a scrolled 500-row fixture to assert the popup
is within 12 logical pixels of its click anchor. Native snapshots of all five
Settings pages are in out/ui-menu-settings-fix/settings-*.png. Snapshot writers
rewind and truncate before replacing a PNG so review images cannot show a
previous run's first image stream. These captures do not prove FL Studio's
physical peer scaling.

Debug UI harness and VST3 build succeeded. Focused CTest: UI 16.95 s and
processor 24.88 s, 2/2 passed (41.88 s total). Processor coverage includes
existing state/audio/capture and callback regressions; no DSP changes were
needed for these UI fixes. Strict OpenSpec validation: all 10 items passed.
git diff --check passed.

Nested Debug VST3: 33,824,768 bytes; SHA-256
b617256ce2dcc74b978da165193e2e1151f9cc846208375a34e6c0a08f2c77ad.
Bundle: out/build/windows-msvc/preverb_plugin_artefacts/Debug/VST3/Preverb Spike.vst3.
No system installation, commit or push performed.

Task 8.1 is now complete (16/45 total). Task 8.2's dedicated-window implementation
and Debug lifecycle checks are present, but its normal Release verification is
not claimed. Remaining diagnostic consolidation/retirement, final visual gate,
Release and FL Studio/DPI checks stay open. User visual approval of checkpoint B
and the new Settings surface remains pending; no approval is inferred from
Copilot continuing work or from automated tests passing.

### Settings review response

The user accepted the current Settings implementation for this iteration:
"Settings is something we'll come back to later for another pass, but this
works fine for now." A later Settings polish pass remains a follow-up. This
acceptance covers the current grouped Settings/Recovery surface; it does not
establish FL Studio row-menu positioning, physical DPI acceptance, Release
verification, or approval of every remaining checkpoint B/C requirement.


### FL Studio menu confirmation and compact-default request

The user explicitly confirmed that the three-dot menu position is now good in
FL Studio. This closes the specific popup-position check, not the entire host
acceptance task. They then requested that the current minimum become the
opening size, and that default text move closer to off-white before further work.
The requested opening size is 1100x850 logical pixels; maximum/resizing support
and reference-size tests are retained. Text tokens are now #F8FAFC primary and
#E1E6EC secondary; dark wet-body/handle helper text uses the secondary token.
The background remains #0F1418 and the supplied wordmark is unchanged. Current
design/spec documentation records this explicit user revision to the earlier
1400x1000 opening default and text baselines.


Compact/brighter implementation verification: native UI CTest passed in
15.81 seconds, including fresh-editor 1100x850 assertions and all existing
resize/gesture/window tests. Explicit final layout after constructor setup
prevents the minimum-size no-op setSize path from leaving legacy diagnostic
controls parented inside the inspector. Added first-open ownership assertions
cover that case; actual ready.png was visually inspected after the correction.
Strict validation: 10/10 passed; git diff --check passed. Native review image:
out/ui-compact-brighter/ready.png (with fresh empty/scaled/size fixtures nearby).
Final nested VST3 SHA-256:
dca300490cc245c12112d766a94d0103f2c7fbe068d8a218229510bea6d9438a.
The Debug bundle remains at the documented build path. No installation or push
was performed. Visual approval of the revised size/brightness remains pending.

The user subsequently approved the compact-default preview: "Size and brightness
look good." This signs off the 1100x850 opening default and brighter off-white
text visually; it does not imply completion of other outstanding host/DPI checks.


### Bar-and-beat naming refinement

The user requested Swell Bar <x.y> and explicitly chose bar-and-beat positions.
New manual/automatic names now use the shared core musical-position formatter:
Swell Bar 18.1 at 34 seconds under 120 BPM/4/4, Swell Bar 18.3 at 35 seconds,
and Swell Bar 18.1.25 for the quarter-beat offset at 34.125 seconds. Fractional
beats retain the existing formatter's precision with trailing zeros removed.
Repeated labels receive (2), (3), etc.; existing/custom/saved names are preserved.
The former UI formatter header forwards to the Core helper, so labels and
inspector positions share the same mapping and no state migration is introduced.

Session, automatic-target, viewport/musical-position and native UI tests passed
4/4 (5.09 s); native screenshot rerun passed in 16.78 s. Coverage includes
same-beat and identical-position collisions, different meter/origin, preserved
names through reanalysis/reset/placement/save/reopen and existing UI regressions.
Native review images are in out/ui-bar-beat-names/. The Debug VST3 was rebuilt;
nested binary SHA-256:
9fe8a3c158790d8fb818fd2c4f913bf7a8288e193a669f54c67f721d454e1160.
Task 4.3 and related naming/design/usage requirements were revised to this
explicit request and remain verified complete (overall progress 16/45).

## Final integrated native audit — 2026-11-02

This entry supersedes earlier pending native run records, not user/host acceptance.
The current access/disposition inventory is in `docs/design/ui-diagnostics-and-acceptance.md`.

- Full configured Debug build succeeded. Clean full CTest rerun: **25/25 passed**
  (`LastTest.log`: 09:23–09:24 GMT). The preceding run's sole failure was an
  obsolete `Swell 2` assertion in the transition feedback fixture after the new
  bar/beat naming; it now checks the actual member name. Focused corrected test
  and the clean full rerun passed; no audio behavior was weakened.
- Release UI/VST3 build succeeded; Release native UI test passed (2.07 s),
  including opening/closing both owned windows during real callback/capture and
  destroying the editor with those windows open. Full Release suite was not run.
- Native fixture output: `out/ui-alignment-final-native/` (58 PNGs), including
  `fresh-1100.png`, `fresh-1400.png`, `mapped-unselected.png`, `ready.png`,
  `many.png`, missing/waiting/stale/error/support and near/far/extreme states,
  Settings tabs and `dedicated-diagnostics.png`. The sparse overview oracle is
  a synthetic geometry fixture, separately labelled from production PCM views.
- Software render scales 1/1.25/1.5/2 cover supplied logo, typography, rules,
  handle/range and percentage geometry. Reviewed empty/selected/diagnostic and
  scaled output. These are software renders, **not physical FL Studio DPI passes**.
- New focused oracles cover independent saved global knobs; actual mix thumb
  positions and sample gains at 0/50/65/100%; latent Wet Only restore; Shift fine
  gesture; identity/deletion-safe exact edits; integer precision; source start/end
  parity; 120 BPM sample/pixel placement; negative lead-in; current sparse gaps
  despite cached detail; actual Overview data/navigation; active tone disclosure;
  and explicit IR assignment through freeze/invalid/cancel/selection/deletion and
  restored-session epoch races, with WAV/WavPack coverage.
- Diagnostics observations format only while open at a guarded 200 ms cadence;
  no hidden diagnostic polling/string work. Main status still refreshes at 10 Hz.
  Writable duplicates and inline allocation are removed. Recovery commands remain
  explicit; lifecycle does not start preview/tone/render or clear capture.
- **Lifecycle discrepancy remains open in 8.3:** current Close hides the reusable
  editor-owned content and disables diagnostic refresh. The design's decision 8
  additionally says Close destroys content/listeners. Content currently survives
  until editor destruction, with SafePointer ownership. This distinction is not
  silently treated as satisfying the stronger disposal wording; obtain a decision
  or implement disposal before checking 8.3.
- Existing full-suite regression oracles preserve legacy/current version-7 state,
  capture/assets, IR/render IDs, tone, freeze/placement, preview safeguards,
  callback allocation/lock-free paths, loops/final bounds and multiple-instance
  offline export. Host identity remains `Preverb Spike`, `com.k7ma.preverb`,
  `K7Ma`/`Prv1`; zero registered parameters. No schema or asset migration.

Reproduction (PowerShell; CMake/CTest absolute paths are documented above):

```powershell
& $cmake --build --preset windows-msvc-debug
& $ctest --preset windows-msvc-debug --output-on-failure
$env:PREVERB_UI_M3_OUTPUT = "$PWD/out/ui-alignment-final-native"
& $ctest --preset windows-msvc-debug -R '^preverb_swell_ui_test$' --output-on-failure
Remove-Item Env:PREVERB_UI_M3_OUTPUT
& $cmake --build out/build/windows-msvc --config Release --target preverb_swell_ui_test preverb_plugin_VST3
& $ctest --test-dir out/build/windows-msvc -C Release -R '^preverb_swell_ui_test$' --output-on-failure
```

Exact replacement bundle binaries (not installed or pushed):

| Configuration | Bundle under `out/build/windows-msvc/preverb_plugin_artefacts/` | Nested binary bytes | SHA-256 |
| --- | --- | ---: | --- |
| Debug | `Debug/VST3/Preverb Spike.vst3` | 33781248 | `e206e86ace8fd8b85c07baafa3c8aa7627eedce3b1e054ff62dc9d9b419ab277` |
| Release | `Release/VST3/Preverb Spike.vst3` | 10482176 | `824dcc072c6cec5537aa27a841132c7ed68e23cf3e754c4187d16a45605f792a` |

Final integrated checkpoint C and tasks 10.1–10.4 remain pending. The individual
compact/brightness, Settings and popup-position approvals above remain valid.
Do not archive based on native test results. Use the current host checklist in
`docs/design/ui-diagnostics-and-acceptance.md`; prior milestone passes are historical.

Audit close-out: strict validation of this change passed; `openspec validate --all
--strict` passed **10/10**, and `git diff --check` passed. Reviewed the four delta
specs against retained behavior and current diagnostic dispositions. Verified
checkboxes were updated individually using the CLI-returned file/line, refreshing
apply instructions after each: **40/45 complete**. Five remain: diagnostic
close-content lifecycle review (8.3), final visual approval (10.1), FL Studio
editing (10.2), playback/state/export (10.3), and physical DPI (10.4).

## Checkpoint C and diagnostic lifecycle approval — 2026-11-02

The user explicitly approved the final loaded/empty/Diagnostics images:
"Approve visuals; proceed to host validation". Task 10.1 is satisfied; physical
FL Studio/DPI checks remain separate. The user also selected "Approve retained
UI; update the design". Decision 8 and the lifecycle scenario now explicitly
allow editor-owned content reuse on Close, with no hidden diagnostic work,
immediate current-data refresh on reopen and safe disposal at editor teardown.
This resolves the discrepancy recorded above; the existing Debug/Release native
lifecycle and active capture tests verify the approved behavior (task 8.3).
No implementation or binary changed in this approval update.

Current progress: **42/45 complete**. Only tasks 10.2 (FL Studio editing), 10.3
(playback/state/export) and 10.4 (physical Windows DPI) remain open. Earlier
40/45 and pending-C entries above are historical audit records superseded here.

## Final user host acceptance — 2026-11-02

After being presented the three remaining groups (FL Studio editing,
audio/state/export, and physical DPI usability at 100/125/150/200% where the
display permits), the user replied **"all approved"**. Record tasks 10.2–10.4
as user-approved acceptance of those listed checks, with no failures or scale
exceptions reported. This is user-reported host acceptance, not an automated
host test or a new claim derived from the software snapshots. The user did not
provide new per-scale screenshots, work-area measurements or host settings.

Last supplied host configuration in this conversation: FL Studio 2026 Producer
Edition v26.1.7 build 5653, Windows 64-bit; FL Studio ASIO, 48000 Hz, 512-sample
buffer. These are the previously reported settings, not newly measured or
separately reconfirmed with this approval. No current limitation was reported.

All **45/45 tasks** are complete. Final native/host approval supersedes earlier
pending entries in this historical log. No source/binary change, installation,
commit, push, spec sync or archive was performed by this acceptance update.
