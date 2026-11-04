# Issue #6 — shared timeline integration

Branch: `feat/6-export-mixed-song-wav`. Existing Export work was saved as
`d551afc`; merge `13ec5be` incorporates main through PR #49 (`12182f2`) and the
merged #47 swell-list implementation. Integration changes remain on this branch.
No remote push, owner merge or OpenSpec archive is claimed here.

## Scope and merge reconciliation

- Consume `TimelineZoomControls` and `MusicalTimelineRuler` from #49 unchanged.
  `ActionButton` also remains byte-identical to main. No competing toolbar,
  waveform abstraction, Snap/grid or editing handles.
- Expose the existing adapter's accepted min/max spans; map logarithmic slider
  intents in Export, reflect the actual clamped view and reject queued intents
  after session/pass/displayed-composition replacement.
- Preserve complete-audible Fit, once-per-accepted-mix fitting, pointer zoom,
  Shift-pan, internal preview positioning, last-good pending navigation and
  current-file readiness. Body/chart and right-column geometry are unchanged.
- Resolve engine/editor conflicts by retaining both Export and main's session-
  epoch/list binding. Keep the new list hidden in Export, and preserve incoming
  frozen recovery logic and all list regression cases. No manual guardrail,
  style-token, debt inventory or saved-state changes.
- Owner approved the playhead interaction during this integration. Original
  visual and three grouped FL Studio passes remain retained. Only the integrated
  row/ruler checkpoint is open; do not ask for the old host checklist again.

## Actual validation

Initial Debug compile `20261104-135003-27c23661` caught a wrongly qualified
`songOrigin` call; corrected to its existing global namespace. Debug build
`135413-676a61df` passed. Focused native Export binding, shared timeline controls
and list ordering executables exited 0.

`scripts/build.ps1 -Configuration Debug -Test`:
**46/46 passed**, 146.13 seconds, exit 0. Logs:
`out/build-logs/Debug/20261104-135955-54062ea0/`.

Binding cases cover geometric mid-zoom, exact clamped endpoints, row/wheel/key
parity, real Fit target, stale queued Fit, pending navigation/action gating,
empty state, large-coordinate finite bounds and 6/8 ruler/cursor pixel alignment.
Panel checks cover supported sizes and 100/125/150/200% software scales. Full
native editor tests retain incoming list and unchanged Learn/header behavior.
These are not actual Windows DPI or screen-reader/FL Studio scaling checks.

UI enforcement passes: 41 files / 61 existing bounded occurrences. Shared
component files have no diff from main. Release/native approval results follow
after completion; no approval is inferred from automated images.

Release wrapper `20261104-140256-1fbd8021` passed **46/46 CTest tests**, 107.27
seconds, exit 0. Both configurations include merged list bindings/capture tests,
shared controls, Export audio/file/voice/persistence and workflow regressions.
Native reference mixes, pending/error, zoom/pan and supported-size/software-scale
images are in `out/issue6-shared-timeline/`; representative minimum, large and
pending images were visually inspected without clipping or layout changes.

Validated Release bundle:
`out/build/windows-msvc/preverb_plugin_artefacts/Release/VST3/ReverbArise.vst3`.
Inner binary SHA-256:
`9A17B0DFD44E4777BD07B56FF7815C034510CBE9EF1BACBE013AED6175083857`.
Input manifest SHA-256:
`0A360303B32F7B139490D7C7DD1CB054A5B1FA7DDFEBD22FF7A7CA2B0E860244`.
Machine-readable evidence: `out/issue6-shared-release-validation.json` and
`out/issue6-shared-validation-inputs.json`. Source is `13ec5be` plus the working
integration diff; the bundle is not installed automatically.

The previous approved playhead review window is replaced by the isolated
Release preview in `out/component-previews/issue6-shared-timeline/`. It retains
the silent real-engine clock and touches no DAW project. Integrated row/ruler
approval is pending; existing host/playhead approvals are not reopened. No new
FL Studio checklist is requested for these UI-only bindings.

## Coordination

Agent 2 received the ownership/baseline handover on #46 (comment 6632200424).
Editor/serialization integration should follow #6's merge; standalone Snap and
isolated timeline work remain independent. Export keeps creative schema v10 and
processor envelope v8, with no new persisted navigation preference.

## Owner placement refinement

The owner accepted shared-tool usage and requested moving the row below the
legend/status and above Preview. The row keeps its approved geometry and width;
the footer uses existing 6/12/24 spacing roles, including a 24 logical-pixel gap
above Preview controls. Header/right column, navigation and audio remain unchanged.
Chart height accommodates the footer; absolute horizontal mapping is retained.

Debug wrapper `20261104-142532-3e693b9d` built successfully. Focused Export
placement/binding checks and the full native UI executable exited 0, including
four supported sizes and software scales. New order/clearance assertions check
waveform → legend → status → toolbar → Preview. Native images are in
`out/issue6-toolbar-below-waveform/`; minimum and large views were inspected.
The prior 46/46 Debug backend results are retained for this layout-only change;
no audio or persistence code changed. Final Release and placement approval follow.

The owner explicitly asked to resend Agent 2's coordination reply. The full
ownership/revision reply is posted at #46 comment 6632200424: baseline `13ec5be`,
no further editor/session edits planned here, final editor/schema integration
after #6 merge, with isolated Snap/timeline work free to continue independently.
This records the GitHub reply channel, not a claimed direct cross-chat delivery.

Release wrapper `20261104-143025-2b6f675d` passed **46/46 CTest tests**,
83.03 seconds, exit 0, after the footer placement change. The validated bundle
remains `out/build/windows-msvc/preverb_plugin_artefacts/Release/VST3/ReverbArise.vst3`;
inner binary SHA-256 is
`C0716C70EF991917FAC618B5B07D75320AB077D8A37E412B4F074F9620C7A5DD`.
Source is `13ec5be` plus the shared integration/footer working diff. Machine-readable
evidence: `out/issue6-toolbar-release-validation.json`. No automatic installation
or additional host acceptance is claimed.

The first review launch exited on the existing asynchronous list-capture fixture
assertion before opening its window; a fresh launch passed the fixture and opened
the responsive native Export review (PID 3440). The completed wrapper/native test
results above are recorded separately from this review-launch retry. Placement-only
owner approval remains pending; all previous tool/playhead/FL Studio approvals stand.

## Final owner spacing refinement

The owner approved Export overall and requested one final spacing pass: related
caption-to-ruler spacing, tighter legend/navigation/Preview grouping, no redundant
wet-only guidance, and additional bottom playback clearance. All layout values
reuse existing 6/12 spacing roles; typography, control geometry, header/right
column, navigation, audio and persistence are unchanged. A status row is present
only for meaningful delivery feedback, dry-inclusive guidance or a silent mix;
there is no empty wet-only instruction row. The tile retains its song-aligned
readiness status and the header notice retains playback safety guidance.

Debug build `20261104-150051-e252c63b` passed. A new focused status-row check
exposed that its fixture mutated an already displayed shared plan in place;
production compositions are immutable. This test setup is being corrected before
claiming the current final validation pass. Earlier validation remains historical,
not evidence that the latest refinement has passed. Final results follow below.

Final validation after correcting the fixture to publish a new immutable plan:
- Debug wrapper `20261104-150419-0bcc866c`: build passed, exit 0.
- Focused `--test-export-paint` checks: exit 0, including caption/ruler gap,
  footer clearance and conditional status-row removal/reappearance.
- Full Debug native UI executable: exit 0; images in `out/issue6-final-spacing/`
  cover supported sizes and 100/125/150/200% software raster scales. Minimum
  1100×850 and 1600×1000 Export views were inspected without clipping.
- Release wrapper `20261104-150522-1e5d0240`: **46/46 CTest passed**, 60.07 sec,
  exit 0. The preceding Release run also passed 46/46; this later run includes
  the corrected fixture source. Prior Debug backend results remain applicable
  to this layout-only refinement.
- UI standards: 41 files / 61 existing bounded occurrences. Strict OpenSpec:
  19/19 passed. Diff whitespace checks pass. No guardrails/tokens were edited.

Current Release bundle: `out/build/windows-msvc/preverb_plugin_artefacts/Release/VST3/ReverbArise.vst3`.
Inner binary SHA-256: `0E925541E3C6C0160EBFAE4499F3D21894E00E01016AE3A66DEA4E5C7D629AFC`.
Source: `13ec5be` plus the shared integration/final spacing working diff.
The directly launched isolated Release preview is under
`out/component-previews/issue6-final-spacing/`; final visual approval is pending.
No additional FL Studio/DPI acceptance is claimed or requested. Existing host and
interaction approvals are retained; no merge/archive has occurred.

The final review launcher again encountered the pre-existing asynchronous list-capture assertion before opening; its fresh retry opened successfully (PID 21292, responsive control-review window). This launch-only flake is separate from the passing focused Export, complete native UI and Release CTest runs; the unrelated list fixture was not changed.

## Compact legend/navigation row and tile guidance

The owner requested two further visual refinements: move dry-inclusive guidance
to the Ready WAV tile, and place compact Wet/Dry keys on the same row as the
right-aligned shared toolbar. The numeric scale caption is removed; the shared
amplitude mapping is unchanged. Legend entries use existing value-width/related
spacing roles. Control sizes, header, typography and audio/state behavior remain
unchanged. Delivery/silent-mix feedback remains available below the shared row.

The initial Debug build failed because new test code called getTooltip through
juce::Component; the test now uses the actual FileDragTile type. Debug wrapper
`20261104-151759-5ce7fb48` passes, and focused Export checks exit 0, including
left/right anchoring, shared vertical centre, minimum-size clearance and correct
mute-source tile guidance at dry-inclusive versus wet-only mix endpoints.
The final Release task is reopened for the latest visual refinement; previous
Release results apply to the earlier documented revision. Current native review
and final validation results follow below.

Current compact-row validation: full Debug native UI executable exited 0; supported-size and software-scale evidence is in out/issue6-inline-legend/. Minimum-size wet-only and 50% mix images were inspected: tile guidance fits, legend and toolbar align without overlap, and the numeric scale text is absent. UI standards pass (41 files/61 bounded occurrences), strict OpenSpec passes 19/19 and diff checks pass. Native visual acceptance and final Release validation of this latest refinement remain pending.

## Zoom-left percentage legend and right-drag pan

Owner requested swapping the combined row positions and trying right-drag for
Export pan. Zoom retains its approved dimensions, now pinned left; Wet/Dry keys
are pinned right and show their accepted composition percentages (including
previous mix values while a new request is pending). The waveform amplitude
mapping is unchanged. Tile guidance remains revision-bound.

Export explicitly chooses TimelineNavigation's rightDrag policy; the default
left/Shift-left policy remains available unchanged. Left-click/drag positions the
preview, right-drag pans only, right-double-click does not Fit, mouse-up cancels
the gesture, and Shift-wheel/keyboard/left-double-click Fit remain available.
No other workspace, audio, persisted state or host transport semantics changed.

Debug wrapper `20261104-152756-ded9d7f2` passed. Focused Export gesture/layout
checks and full Debug native UI both exited 0. Coverage includes right-pan/no
seek, release cancellation, retired Shift-left pan, pending pan, left seek,
viewport/ruler alignment, row anchoring and supported sizes/software scales.
Images: `out/issue6-right-pan/`. Minimum-size 50% mix was visually inspected.
Release wrapper `20261104-152927-a961feeb` passed **46/46 CTest**, 59.12 sec,
exit 0. UI standards pass 41 files/61 bounded occurrences; strict OpenSpec passes
19/19. Guardrails, shared tokens and typography remain unchanged.

Validated Release bundle: `out/build/windows-msvc/preverb_plugin_artefacts/Release/VST3/ReverbArise.vst3`.
Inner binary SHA-256: `66BA0CE7C32BAFEE7BA6AF872877551EDDD1D01F072480A93182C086F5B0FFBF`. Source: `13ec5be` plus integration/right-pan working diff. Isolated Release preview PID 58052 is responsive. Visual/gesture owner approval is pending; prior host approvals stand. No merge/archive.

## Final owner acceptance and criterion cross-check

Owner explicitly approved the final layout and right-drag pan on 2026-11-04.
This completes the shared ruler/zoom integration checkpoint; previous playhead,
control-family and three grouped FL Studio passes are retained without retesting.
The three groups used FL Studio 26.1.7 / 48 kHz / 512 samples. New gesture/ruler
acceptance is native, not a claim of repeated host validation or OS-DPI coverage.

| Issue #6 criterion group | Evidence |
| --- | --- |
| Dedicated 66/33 workspace, controls, no overlay/summary | Final native owner approval; supported-size/software-raster checks |
| Wet-based entry, 0% after entry, audition-derived initial mix | Model/processor admission, migration and preference fixtures |
| Exact 0/50/100% mix and actual waveform contributions | Independent sample oracles, overlap/antiphase tests and native reference images; host group 1 passed |
| Complete audible Fit/preview start and song-origin WAV | Boundary/negative/sparse/quiet-tail file oracles; shared ruler/navigation/cursor tests; host groups 1–2 passed |
| Preview equivalence, Export bypass, no capture/reconvolution | Paged voice/kernel/callback/workflow tests and host routing pass |
| Missing dry warned as silence; honest readiness | Coverage/failure/stale-result tests, pending/error native images; approved policy |
| Durable revisions and saved percentage/reference | Atomic file/fault/restore/instance fixtures and host persistence pass |
| Real Playlist drag and Save cancel/success | Owner host group 2 passed; forced unsupported-drag host configuration untested, fallback authority tested |
| Safe guidance and interaction | Header bypass notice, Ready-tile dry-inclusive warning, native row/gesture approval |

Final Release and Debug/native results and exact binary identity are recorded in
the preceding section; no implementation inputs changed after those runs.
Strict OpenSpec/diff checks are rerun for documentation/task closeout. #5's
Perform audition, #8's broader workflow, #38's generation transition and #46's
Perform redesign remain separate. Existing Learn Stop→Perform and source
provenance are preserved and covered by incoming workflow/list regressions.
No current #6 acceptance remains unresolved. Actual Windows DPI/scaling and
external screen-reader behavior remain unverified; software raster coverage is
not a substitute. The asynchronous list-capture review-launch flake recorded
above is retained as a known fixture limitation, not silently removed.

OpenSpec is implemented and accepted, not synced/archived by this agent. The
owner retains merge and archive authority. The native approval window is retained.

## PR #50 review — transient final publication timing read

The reported path was reachable: exportAttempt had been set before the final
contextProvider compatibility read, so returning on that miss could suppress all
same-key retries. The correction revokes only that unfinished attempt and keeps
one private request-keyed ExportFile reference. A later valid pass re-verifies
and reuses the immutable file; the full capture/timing/ownership checks still
precede Ready/history publication. Changed keys, mode exit, restore and Clear
release the private reference. Previous accepted files remain untouched; no PCM
or worker wait loop is added.

The deterministic engine regression makes contextProvider return unavailable
timing only once a new final-name Swells WAV exists. Its first injected miss
therefore occurs at the final publication checkpoint, not the early worker read.
While timing remains unavailable it checks Preparing, prior file/history retained,
no current playback and exactly one new WAV. Restoring timing must reach Ready
with that same single file. A second case supersedes the deferred request and
asserts its mix never enters accepted history. Existing seek/audio allocation,
restore, file recovery and mode-exit cases also run in this focused executable.

Actual validation (focused scope explicitly requested by owner):
- Debug wrapper build `20261104-155610-dfc4812d`: pass, exit 0.
- Debug preverb_song_export_engine_test: pass, exit 0;
  log `out/issue6-publication-retry-debug.log`.
- Release wrapper build `20261104-155811-56641249`: pass, exit 0.
- Release preverb_song_export_engine_test: pass, exit 0;
  log `out/issue6-publication-retry-release.log`.
- Release preverb_song_export_test: pass, exit 0; file/sample/fault/working-set
  regressions, log `out/issue6-publication-file-regression.log`.
- Strict OpenSpec 19/19 and diff checks pass.

These are direct native focused tests, not a new full CTest run. The previous
46/46 full-suite results and accepted UI/FL Studio evidence are retained for
unaffected behavior, as the owner requested. No further host testing or visual
approval is needed for this engine-only correction. Source: `58d5872` plus the
publication-retry review diff; build does not install the bundle automatically.
Updated Release inner binary SHA-256: `EE136957767C9AE61950F8C9A037F43F7421F63929BD4452A6C5DBE67F5C6E28`; bundle path remains `out/build/windows-msvc/preverb_plugin_artefacts/Release/VST3/ReverbArise.vst3`.
