# Workflow foundation evidence

Change: capture-live-finalise-foundation; issue #10. Implementation in progress;
no native visual or FL Studio acceptance is claimed.

## Compatibility baseline — 2026-11-02

Source baseline: cd3e88b0d5714b1f9b5cde20ebe5cc012b4aab55. Processor XML
envelope version 7; swell JSON version 8 (separate domains). Bundle ID
com.k7ma.preverb, manufacturer/plugin codes K7Ma/Prv1; no registered host
parameters. Requested new build/product name: ReverbArise, preserving IDs.
Original VST3 component CID: ABCDEF019182FAEB4B374D6150727631;
controller CID: ABCDEF011234ABCD4B374D6150727631. Baseline Release SHA-256:
9ef65e3de3ccbb09276933bf2f385bfe8af309c5b841c721afa06a3d024ffb91.

Before workflow changes, Debug CTest passed 6/6 in 43.20 s: gain_staging,
ir_handover, swell_session, transport_playback, processor and capture. Additional
swell_engine/swell_asset/persistent_capture tests passed 3/3 in 7.41 s. Commands
used the configured out/build/windows-msvc tree with -C Debug and focused
preverb_*_test regular expressions. Existing independent PCM oracles remain the
baseline, not snapshots generated from the changed implementation.

Fixtures: persistent_capture_test preserves exact stereo sample vectors over
partial replacement, mapped silence, sparse gaps, old-manifest restore and
failed publication; swell_asset_test pins decoded Hall identity and validates
managed IR/render content hashes and missing/corrupt references. swell_session
and gain_staging tests roundtrip legacy versions and current musical calibration,
including frozen metadata, exact edits and invalid-state rejection. Processor
fixtures verify legacy/current state, dry/wet sample output and two-instance
export before mode setup changes. Assets created by fixtures are temporary;
their deterministic sample/content identities are asserted in the tests.

Factory WAV SHA-256 identities recorded unchanged:

Canonical decoded content hashes (rate double, channel int32, frame uint64,
then little-endian float32 samples): persistent source8 fixture
`27cef11381100414153548a069ad170379d701cce556ccdafef253005016837a`;
swell_asset raw7 fixture
`3317f0cefd4c1143171b93cfc90529dd2ac180fa9f023d7ec78a2eea6887b02c`.
The independent workflow transition/guard test passed Debug CTest (0.06 s).
This verifies the standalone model; processor wiring remains later work.

| Asset | SHA-256 |
| --- | --- |
| Hall | b7275a27fc7629a9a1a7191abda4fbb3c8a46372dc9071e36dc1112e4839b458 |
| Monumental | 0799b724acba5b66f3e933109a79454673da944d022f2ac13a8d6cb2a0532148 |
| Plate | fad9e920013101794af9050fbf88d784f65ccb7fd65523e963998a2853b10c1c |
| Dark Hall | 1f86a72cbf8fcdae256ea8a8765705fd227ae1adaf94594908c80fd2f370d65c |
| Shimmer | 16d1ea58c5db5ae1bf110ceea30da6a88276092a40bb34f25745f72f7d9eee7f |
| Whisper | 59f013118f8daccb87b93c9b125c385d620f50e72e710bede6eedcd93d5387c7 |

## Initial source/processor integration

Private transaction plus unchanged persistent-capture Debug checks passed 2/2
in 0.46 s. After splitting candidate preparation from acceptance, transaction
and new workflow processor checks passed 2/2 in 0.47 s. The latter uses explicit
host Play/Stop reports and verifies no accepted source before Stop, protected
replay, explicit replacement, automatic Live, Finalise dry-only under Wet Only
and offline callbacks, saved Finalise and legacy Capture restore. Subsequent
fault/delay/UI additions are not yet covered by these earlier pass counts.

## Focused regression checkpoint — 2026-11-02

Debug CTest passed 5/5 in 5.26 s: workflow processor, capture transaction,
workflow state, native swell UI and transport playback. The processor fixture
wraps realtime callbacks in allocation/deallocation guards. An isolated
`preverb_processor_test.exe multiple-export` run also passed after updating the
fixture for explicit Stop publication and retaining its numerical export oracle.
The closing Capture boundary is recorded before asynchronous settlement can
switch to Live; Live Stop does not shorten source windows.

Native Capture/Live/Finalise visuals were presented for review. The user asked
whether these controls are temporary; the confirmed scope is a minimal issue #10
UI, with the dedicated Capture workspace in #11. That question is not visual
approval. Explicit visual approval, full Debug/Release results and FL Studio
acceptance remain open at this checkpoint.

The full configured Debug build and CTest suite subsequently passed 32/32 in
108.22 s. The added frozen-recovery exclusion cases passed in a further processor
run (44.15 s): disabled, muted, solo-excluded and incompatible-tempo frozen wet
cannot qualify for Live without source. The compatible frozen member still
exports audible wet. Dense/sparse live-resource fixtures used 60,819,856 and
9,332,848 bytes respectively, under the 268,435,456-byte allowance.

The user explicitly approved the temporary mode/Recapture/read-only visuals on
2026-11-02. This completes the native visual gate, not FL Studio or DPI acceptance.

## Renamed delivery identity

Release bundle: `out/build/windows-msvc/preverb_plugin_artefacts/Release/VST3/ReverbArise.vst3`.
Its `Contents/x86_64-win/ReverbArise.vst3` SHA-256 is
`49564d9a88c14863c98bc5b4f5efd73fccdba90caab38aaba5a50d742519fc0b`.
Generated Release module metadata confirms the original component
`ABCDEF019182FAEB4B374D6150727631` and controller
`ABCDEF011234ABCD4B374D6150727631`. CMake retains `com.k7ma.preverb`, `K7Ma` and
`Prv1`; processor tests assert the unchanged empty host-parameter inventory and
supported legacy-state restore. No installed old bundle has been removed or
replaced by these build commands.

## FL Studio acceptance checklist and results

Final automated gate: complete configured Release build and CTest passed 32/32
in 30.38 s. Final changed Debug workflow/native UI fixtures passed 2/2 in 4.84 s,
in addition to the full Debug 32/32 run above. Commands used the Visual Studio
CMake/CTest binaries, build tree `out/build/windows-msvc`, configurations Debug
and Release, and `--output-on-failure`. Logs are retained locally as
`out/workflow-full-debug-{build,ctest}.log`, `out/workflow-release-{build,ctest}.log`
and `out/workflow-final-debug-test.log`.

The mixed-mode fixture checks actual nonzero Live wet against a separate
Finalise instance's exact unchanged dry samples in the same offline callback
sequence. Its wet endpoint explicitly overlaps the tested source window; the
initial zero-overlap fixture correctly had no audible wet at/after song zero.
Native tests additionally reject delayed IR assignment in Capture/Finalise,
canvas dragging while locked, and an old typed edit across a mode roundtrip
even before a UI refresh. Snapshots cover recording/settling/failure and idle
modes at 1100x850 and 1400x1000. A screenshot-helper compile error was corrected
before these final passes; no product behavior was changed by that correction.

Strict `openspec validate --all --strict` passed all 12 items; `git diff --check`
passed. All six bundled WAV hashes and the Release binary hash match the values
recorded above. Host checks below are tracked individually for this exact build.

Record the exact Release binary hash below, host version, sample rate/buffer,
routing and monitoring adjustments with results. Earlier host passes belong to
earlier builds and do not complete these checks.

1. Fresh Song Capture passes dry, locks mode switching during Play/settlement,
   and enters Live after useful audio is committed on Stop. Try Pattern too;
   unavailable timing must be explained rather than fabricated. An empty or
   all-silence pass must not automatically enter Live.
2. Live loops/seeks play accepted swells without capture or continual source
   replacement. Repeat with the editor closed and include the final event.
3. Return to Capture with Recapture off: mapped audio and mapped silence stay
   protected; only previously unmapped positions are added after Stop.
4. Arm Recapture independently of Auto markers, play changed audio, then Stop:
   only encountered positions change together and the arm clears. Cancel a pass
   through recovery controls and verify prior accepted source remains usable.
5. Check saved Dry + Wet and Wet Only routing in all modes. Capture/Finalise
   retain dry passthrough; Finalise host WAV bounce has no scheduled wet. Live
   reentry joins the current position without replaying old lead-in.
6. Check two instances in a Live WAV bounce, plus mixed Live/Finalise instances;
   only the Live instance contributes wet. Pending jobs must not change the
   accepted snapshot halfway through a bounce.
7. Save/reopen Capture, Live and Finalise; mode survives, Recapture/voices/pass
   do not resume. Save during Capture/settlement, reopen, and confirm only the
   last accepted source was referenced. Duplicate an instance and recapture in
   one without changing the other's saved source.
8. Open a pre-upgrade project copy: always Capture with Recapture off, preserving
   edits/IR/gain/freeze. Select Live explicitly after dependencies validate.
   Verify compatible frozen-wet recovery if dry source is missing; ordinary
   missing-source swells remain unavailable. Check host scaling/disabled states.

### User-reported results — 2026-11-02

- Pass: fresh instance Capture, Song Play/Stop, automatic entry into Live, then
  replay with audible swells and no source recording. This is the specific
  four-step check presented to the user; it does not cover Pattern playback,
  silence-only passes, locked switching or the other host checks above.
- Host version/rate/buffer and routing/monitoring adjustments for this run have
  not yet been reconfirmed. Broader tasks 6.2/6.3 remain unchecked.
- Pass: return to Capture with Recapture off, change/remove an upstream event,
  play the previously mapped section, Stop and return to Live. The original
  waveform/swells remain; the user specifically confirmed the old swell remains
  audible where the new performance no longer has an event. This verifies
  protected mapped audio; additive new coverage and mapped-silence protection
  still require their own host checks.
- Pass: explicitly arm Recapture in Capture, play the changed section and Stop.
  The user confirmed automatic Live entry, the arm clearing, and replayed swells
  reflecting the changed performance. Cancellation/failure rollback and
  untouched positions outside the encountered section remain separate checks.
- Pass (four separately confirmed steps): Live with Wet Only produces swells
  only; Finalise passes original dry audio without swells; a short Finalise WAV
  export contains dry only; returning to Live restores Wet Only routing.
- Pass: Live Dry + Wet through repeated song loops and seeks, repeated with the
  plugin window closed. Swells join at the current position, the final loop-edge
  swell remains audible, and no glitches or continual regeneration were reported.
- Pass: two instances on different mixer tracks in Live both contribute swells
  to WAV export, including final events. A second export with one instance in
  Finalise retains only that instance's dry contribution while the Live instance
  still contributes swells. Pending-job export snapshot behavior is not included
  in this particular confirmation.
- Pass: Live/Finalise modes and edits survive explicit project save/reopen.
- Capture restore passes when the user explicitly saves first: Capture survives
  and Recapture is off. Failure: changing a previously saved mode to Capture did
  not make FL Studio prompt to save on close. The user also reports other saved
  plugin settings fail to mark the project modified. This reopens host-notification
  verification; the existing serialization pass is conditional on explicit save.
- FX duplication was not tested: the user reports no direct FX duplicate action
  in their FL Studio workflow. No clone-isolation host pass is inferred.

### Host save-notification correction

The user confirmed a durable workflow mode change (not just temporary Recapture
arming), and reported the same missing save prompt for other plugin settings.
The processor had no non-parameter-state-change notification path. The fix uses
JUCE AsyncUpdater to coalesce engine/capture events and deliver
`ChangeDetails::withNonParameterStateChanged(true)` on the message thread.
Constructor defaults and explicit state loading do not forward pending user-edit
notifications. Accepted source/automatic Live and later managed IR/render changes
also notify without requiring an open editor. No host parameters or state format
were added. Selection, worker progress, transport, rejected/no-op edits and the
temporary Recapture arm are excluded.

The workflow processor fixture now installs an AudioProcessorListener and checks
the actual flag, message-thread delivery, automatic Live notification, mode
changes, coalesced Level/monitoring/marker/preferences edits, and silence for
unchanged/invalid edits, selection and Recapture. Its focused Debug run passed.
Full rebuilt Debug/Release results and the new binary identity are recorded below
when available. The FL Studio save-prompt retest remains open.

The complete corrected Debug build and CTest passed 32/32 in 106.91 s. The
corrected Release binary SHA-256 is
`ec64df783e96e147124f745f8938e951cbf836150e30b3efad239c4b056197e6`;
it replaces the earlier hash only for this correction round, not the historical
host results above. Output path and VST3 component/controller IDs are unchanged.
The complete corrected Release build and CTest passed 32/32 in 35.46 s. Logs:
`out/workflow-dirty-full-debug-build.log`, `out/workflow-dirty-debug-ctest.log`,
`out/workflow-dirty-release-build.log`, `out/workflow-dirty-release-ctest.log`.
Strict all-spec validation remained 12/12 and diff whitespace checks passed.

For the still-open FX copy test, Image-Line documents dragging the effect slot's
**Save preset as...** menu item onto another Mixer track/FX slot to copy the
effect and settings ([Effects Slots manual](https://ww2.image-line.com/fl-studio-learning/fl-studio-online-manual/html/mixer_plugin.htm)).
This supplies a host workflow for the check; it is not a reported pass.

- Corrected-build host pass: the user reports that FL Studio now prompts to save
  for any real setting change. This accepts the host dirty-state notification
  fix on Release SHA-256 `ec64df783e96e147124f745f8938e951cbf836150e30b3efad239c4b056197e6`.
  It does not independently reconfirm every restored value or complete the
  remaining copy/isolation, interrupted-capture and edge-case host checks.
- Corrected-build host pass: FX copy via the documented preset-copy workflow
  retains prepared swells in both instances. Recapturing changed audio in only
  one copy updates that copy while the other retains its original captured
  source and swells. All three steps were separately confirmed by the user.
- Corrected-build host pass: with Capture selected and Recapture off, audio added
  beyond previously captured coverage is captured on Play/Stop and the plugin
  enters Live. The user confirmed that this produced their latest screenshot.
  Protecting previously captured silence remains unconfirmed: a blank waveform
  region alone does not establish whether its source positions were mapped.
- Corrected-build host pass: the user subsequently confirmed that the silent
  region shown remained protected in Capture with Recapture off when replayed
  with changed input. This accepts the requested silent-region protection
  behavior. Its exact-zero versus quiet-tail storage classification was not
  independently inspected; both stored coverage kinds must remain protected.
- Corrected-build host pass: during armed Capture playback, the mode selector
  is disabled. Cancelling through Settings > Recovery > Cancel recapture before
  Stop, then stopping and selecting Live, retains the previously accepted source
  and swells. Both mode locking and cancelled-pass preservation were confirmed.
- Corrected-build host pass: Save As while an armed changed-source Capture pass
  is still playing, before Stop; stop and close without saving subsequent changes;
  reopen the saved copy. The user confirmed Capture mode, Recapture off, the
  previously accepted source retained, and no resumed capture pass. This covers
  saving during recording, not a separately delayed validation-phase save.
- Corrected-build host pass: Pattern playback in a prepared instance with
  Capture selected and Recapture off preserves the previously captured source
  and swells when looping a changed Pattern over mapped positions. The user
  explicitly confirmed this second edge check. Silent-only capture is not
  included in that confirmation.
- Corrected-build host pass: the user confirmed they have been using the same
  pre-upgrade project throughout and had already completed the requested legacy
  reopen check successfully: Capture entry with Recapture off and existing
  swells/IRs/edits preserved. No additional legacy reopen is requested.

## Final acceptance — 2026-11-02

The user explicitly selected **Accept automated edge-case coverage** for the
remaining silent-only, delayed validation/save/restart, pending-job export and
frozen-recovery checks. This supersedes the earlier pending manual gates for
those cases; it does not turn them into FL Studio passes. The correction round
is pinned to Release SHA-256
`ec64df783e96e147124f745f8938e951cbf836150e30b3efad239c4b056197e6`.
Earlier host results retain their recorded build association; notification-fix
passes and subsequent host confirmations are identified separately above.

| Issue #10 acceptance | Evidence and acceptance basis |
| --- | --- |
| Deterministic modes and disabled states | workflow_state/workflow_processor, native UI tests and approved mode visuals; user mode/routing/capture-lock passes |
| Additive capture, protected data and independent explicit Recapture | capture_transaction exact sample/coverage oracles; user Song/Pattern protection, silent-region protection, new coverage, armed Recapture and cancellation passes |
| Recording/settlement locks with reasons | user recording lock pass; held-validation model/processor/UI checks accepted as automated coverage for settlement |
| Useful Stop selects Live independently of wet preparation | user fresh/Recapture Stop passes; deterministic processor and worker tests for useful, mapped-only, silent-only, invalid and failed passes |
| Duplicate/stale completion cannot republish | workflow_state, capture_transaction and workflow_processor generation/epoch/publication-fault/held-validation tests; accepted automated race coverage |
| Live never captures; Finalise emits dry only | numerical processor/transport/mixed-mode fixtures; user Live replay, Wet Only roundtrip, Finalise WAV and two-instance/mixed-mode export passes |
| Reopen preserves mode/source/creative state | user explicit saves, legacy project, save during recording, FX-copy independence and corrected save prompts; state/transaction fixtures cover pending validation and old immutable references |
| Host transport and audio-thread regressions | user Song/Pattern, loops/seeks/final events/editor-closed passes; full Debug and Release 32/32, callback allocation guards, IR handover/streaming and pending-export/frozen-recovery fixtures |

Scope remains issue #10's behavioral foundation. Dedicated Capture workspace is
#11; full-timeline audition is #5; Finalise/export composition is #6; storage
portability/cleanup is #8. Existing IR assets and gain calibration behavior have
unchanged hash/numerical regression evidence. No new host automation parameters
or plugin identifiers were introduced. The save-notification fix adds only
coalesced non-parameter host notifications.

Host metadata last supplied by the user was FL Studio 2026 Producer Edition
v26.1.7 build 5653, FL Studio ASIO, 48000 Hz and a 512-sample buffer. The user
continued with the same project; those settings were not separately reconfirmed
for this final run. Do not present them as freshly measured. Additional DPI
scaling, delayed-validation host interruption, pending-job host export and
missing-source frozen recovery were not newly exercised in FL Studio. The user
accepted the specified edge-case automation; extra DPI testing remains a stated
host limitation. Native layout checks at 1100x850/1400x1000 are not DPI passes.

Capture UI follow-up for #11: the user could not readily distinguish captured
quiet audio, exact-zero silence coverage and unmapped gaps in the temporary
waveform view. Preserve this usability feedback for the dedicated workspace.

Final sign-off checks passed after recording the approved validation decision:
strict all-spec validation 12/12, `git diff --check` clean, and unchanged final
Release binary hash. This sign-off round changes documentation/tracking only;
the recorded 32/32 Debug and 32/32 Release results still apply to that binary.
No commit, push, GitHub issue change or archive operation was performed.

## PR #18 review amendments — 2026-11-02

The owner approved a narrow Stop-policy refinement: a successful explicit
Recapture replacing mapped audio with silence returns to Live if useful accepted
source remains elsewhere. Repeating an identical silence replacement also returns
to Live without publishing a redundant source revision. Erasing the last useful
source remains Capture; fresh silence, empty and ordinary mapped-only passes do
not gain automatic Live entry. Worker tests cover these cases and saved-state
notifications. The new silence-replacement regression failed before the fix.

Finalise readiness and frozen recovery now share the same ready-render timing
check: a present, nonempty committed render must match the current sample rate,
tempo and meter. Unit cases reject each incompatible field, missing frames,
non-ready phases and invalid timing; the processor test rejects Finalise with
incompatible render metadata even when captured source remains compatible.
This is foundation eligibility, not issue #6's full export/asset preflight.

**Correction to legacy host evidence:** the owner clarified during PR review
that they manually replaced the original Preverb Spike effect. Earlier legacy
state/reopen confirmations therefore do not establish automatic discovery of
ReverbArise from a project still referencing Preverb Spike. That exact host check
remains unverified. Unchanged component/controller identifiers and automated
legacy-state tests are supporting compatibility evidence only. No fresh FL Studio
pass is claimed for these review amendments.

Review-build verification: full Debug build and CTest **32/32** (118.98 s),
full Release build and CTest **32/32** (36.42 s), strict all-spec validation
**12/12**, and `git diff --check` passed. The rebuilt Release module at the bundle
path above has SHA-256
`b391afb9382ec25a59715fcda57c56935992014eee837a080f0224ea3c231e43`.
Archived planning and previous dated acceptance records remain unchanged.
