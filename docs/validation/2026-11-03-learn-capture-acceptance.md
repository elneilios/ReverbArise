# Learn workspace — final acceptance, 2026-11-03

Issue: [#11](https://github.com/elneilios/ReverbArise/issues/11). OpenSpec:
`learn-capture-workspace`. Branch: `docs/11-learn-capture-workspace`.
Implementation baseline: `dc003a7` / merged main `653f13a`.

## Status

Native component checkpoints A and B are owner-approved, including the focused
click/keyboard retry. The owner also approved the focused persistent-pin and
musical-readout refinements on 2026-11-03. Current Debug and Release each pass 38/38 tests. The three consolidated FL Studio smoke
groups passed on the preceding Release build. No commit, push, PR, archive or
merge is implied.

## Issue #11 acceptance crosswalk

| Criterion | Implementation / automated evidence | Owner / host status |
| --- | --- | --- |
| Independent Recapture and Automatic controls; four combinations | `LearnPassPolicyCases` in `preverb_processor_test`: real capture, counters, source/dry-output samples, replacement protection and explicit Perform generation | Native UI approved; host smoke passed |
| Off hides generation settings while dry capture remains | `preverb_swell_ui_test`, `preverb_workflow_processor_test`; hidden preferences retained | Native UI approved; host smoke passed |
| On exposes real IR/file import and horizontal sensitivity + typed percentage | `LearnWorkspace`, shared Switch/SliderControl/NumericInputControl; geometry, wheel/key/stepper, stale invocation and font tests | Numeric family and checkpoint A approved |
| Unsupported Character/context controls are feature-gated | No fictitious Character DSP/preset controls; approved proposal explicitly defers algorithms | Native composition approved |
| Bundled/custom Default IR available before Play; failed/cancelled import preserves accepted choice | `preverb_ir_choices_test`, `preverb_generation_configuration_test`, `WorkspaceNoticeCases`; stable helper and delayed contextual progress/error | IR feedback approved; host smoke passed |
| Managed custom choices/default survive reopen and external-file removal | `preverb_swell_session_test`, `PassOwnedGenerationCases`, imported collection v9, existing WAV/WavPack asset tests | Host reopen smoke passed; file-removal edge case automated |
| Deferred automatic assignment uses immutable pass settings and preserves edits/freeze/suppression | `preverb_capture_context_test`, `preverb_capture_transaction_test`, `preverb_automatic_targets_test`, `preverb_automatic_coordinator_test`; A/B contexts, boundary arrival, copy/restore and fault/race oracles | Automated; brief host IR/reopen smoke passed |
| One large source Overview; no duplicate strip/list/inspector/audition in Learn | `WorkspaceLayout`/`LearnWorkspace`; native geometry and Perform-return checks at 1100×850 and larger | Checkpoints A/B approved |
| Truthful sparse/audio/silence/unavailable/provisional display and stable auto-fit | `LearnTimelineCases`, native pixels, actual worker progress, keyboard range inspection; 86–90 then 1–4, loops, negative source and large sparse spans | Native B approved; usual-scale host smoke passed |
| Lock navigation/preferences through capture/Stop validation; useful acceptance enters Perform | `preverb_generation_configuration_test`, `preverb_workflow_processor_test`, all four real-processor policy cases and callback guards | Host smoke passed |
| Existing Perform editor returns with state/navigation | `preverb_swell_ui_test`; retained selection/view, IRs, gain, monitoring and guarded modes | Owner B return-navigation pass; host smoke passed |
| Approved style, minimum/larger windows and DPI | Approved static-Inter header unchanged; shared Learn typography/spacing/focus tokens; native 1100×850/1400×1000 and software raster 100/125/150/200% views | A/B approved. OS-DPI/host scale matrix **not performed**; usual host scale passed (numerical DPI not supplied) |

Full historical commands, fixtures, failures/fixes and approvals are in
[the implementation evidence](learn-capture-workspace-evidence.md). Images under
`out/issue11-keyboard-inspection/` include empty, recording, acceptance, cancelled,
failed, mixed-coverage, focused inspection and Perform views. Software raster
scales do not stand in for Windows/FL Studio DPI checks.

## Compatibility and boundaries

- Reuse #16's Learn/Perform/Export display names and approved header. Internal
  Capture/Live/Finalise enums, saved mode strings, bundle/cache identities and
  zero-parameter host contract remain unchanged; no second rename/migration.
- #5 full-timeline audition, #6 dedicated Export composition, #9 Perform bulk IR
  selection, #1 Pattern/Song discrimination and Character DSP remain outside this
  change. Inspection never seeks or controls the DAW. Existing audio/export
  regressions remain required, rather than new UI promises for these features.
- #8 root relocation, portable packaging and destructive cache cleanup are not
  implemented. Reuse immutable managed PCM/IR/render assets. Source manifests
  read schema 1/2; workflow writes schema 2 with coherent generation provenance.
  Creative JSON is v9; processor envelope remains v8. Keep older project/cache
  copies for rollback to binaries lacking these new metadata versions.
- Gain calibration, convolution algorithms, plugin identity and approved IR assets
  are preserved. Dry-only pass scheduling is the intentional breaking behavior:
  it overrides legacy forced-Recapture refresh; explicit Perform work remains.
- Working audio allowance remains 256 MiB. Encoded context metadata is separately
  capped at 8 MiB / 4096 contexts / 65536 spans. Published progress metadata is
  bounded at 2,195,512 bytes, below its 4 MiB limit, with no retained PCM. Callback
  allocation/deallocation guards and transactional storage faults are automated.

## Final validation record

| Command | Actual native result | Logs |
| --- | --- | --- |
| `scripts/build.ps1 -Configuration Debug -Test` | **38/38 passed**, 118.75 s, wrapper exit 0 | `out/build-logs/Debug/20261103-134638-74846905/` |
| `scripts/build.ps1 -Configuration Release -Test` | **38/38 passed**, 48.31 s, wrapper exit 0 | `out/build-logs/Release/20261103-135047-ff3a309d/` |
| `openspec validate --all --strict --no-interactive --json` | **15/15 passed**: one change, fourteen specs | `out/issue11-openspec-final.json` |
| `git diff --check` | Passed | No whitespace errors |

Both configurations include product/version contracts, shared header/control and
native-input UI tests, capture/context/fault/migration/default/custom IR tests,
automatic/gain/streaming/audio guards, and transport/multi-instance export sample
oracles. The first final Debug sweep failed because the native test injected its
click before window activation settled. Only test setup changed: wait for the
native peer's focus while pumping activation messages, verify the hit target,
then retain the original click/focus/key assertions. Two focused UI runs from
CTest's working directory passed, followed by the full runs above. The approved
production UI and behavior were not changed for that test correction.

Release telemetry's measured logical metadata ceiling is **2,097,200 bytes**;
Debug's is **2,195,512 bytes**, both below 4 MiB. Release live-resource fixtures
measured **61,020,048 bytes dense / 9,332,832 bytes sparse**, below the 268,435,456-
byte working budget. The full Release test transcript is preserved at
`out/issue11-final-release-LastTest.log`; callback allocation guards passed.

Product version is **0.1.0**, with zero host parameters. Component/controller IDs
remain `ABCDEF019182FAEB4B374D6150727631` /
`ABCDEF011234ABCD4B374D6150727631`; bundle ID remains `com.k7ma.preverb`.
Processor envelope v8, creative JSON v9 and capture reader schema 1/2 (workflow
writer 2) are confirmed. The bundled IR manifest remains unchanged:
`7d78ef0fc2d6012922f50fbd7fb0dd22fdfb1abaa673aefe02e1061d78ae786a`.

Exact build-input manifest: **263 files**, SHA-256
`6e18c226aa3b172c23c2bb2e9066ff9ce8b6b06fad0075dd4bf69867be2bd301`, retained at
`out/issue11-final-build-inputs.json`. This identifies the uncommitted source atop
`dc003a744b6938d5dd05675b69d0be5f13ef14aa`, not a new commit.

| Binary | SHA-256 |
| --- | --- |
| Debug ReverbArise VST3 | `5ad62df5e102010d80d91497288073dffb07d14d36800a07f2e33a5e4690101b` |
| Release ReverbArise VST3 | `a1ee81786a429ac1d4f4eadde62fb91c0093830d01dd68e04aac8f346d54a9b2` |

Host-tested baseline Release bundle (subsequently rebuilt below):
`out/build/windows-msvc/preverb_plugin_artefacts/Release/VST3/ReverbArise.vst3`.
Exact machine-readable identities: `out/issue11-final-identities.json`.
Final review will rerun strict validation/diff checks after any accepted fixes
and host results before task 7.3 closes.

### Refined inspection contract

The owner approved the eight inspection refinements in this change: persistent
left-click pin across focus changes, right-click no-op, waveform-scoped Escape,
distinct inspection visuals, read-only recording/settlement interaction,
reliable musical/time/sample coverage readout, ephemeral state and coupled reset.
Focused implementation and native preview acceptance passed on 2026-11-03; these
refinements were not in the Release binary used for the host smoke below.

## Consolidated host smoke — passed

On 2026-11-03 the owner reported **“All three pass — usual setup”** for all three
groups below using Release SHA-256
`a1ee81786a429ac1d4f4eadde62fb91c0093830d01dd68e04aac8f346d54a9b2`.
Usual setup: FL Studio 2026 v26.1.7 build 5653, FL Studio ASIO 48 kHz /
512 samples. No changed configuration was reported. Numerical host/OS DPI was
not supplied; this does not claim a full DPI matrix or host coverage of the
subsequent inspection refinement. Earlier native visual approvals are retained.

1. **Dry learning / protection:** in a fresh Learn instance, Automatic Swells Off
   and Recapture Off, play a short section and Stop. Actual source should appear,
   no swells should be generated and useful accepted source should enter Perform.
   Return to Learn, change the upstream sound and replay the same section with
   Recapture still Off: the previously captured waveform should remain protected.
2. **Generate / replace / return:** enable Automatic Swells, choose a custom IR A
   (or a bundled response), arm Recapture and play the section again. Include one
   seek to an uncaptured later position and back if practical. Generation controls
   and mode navigation should stay locked until Stop acceptance. Then Recapture
   should disarm, Perform should open, the replacement source should be present
   and new swells should use A. A later default B must not reassign those swells.
3. **Save/reopen:** save and reopen once. Confirm the accepted source, future
   default/sensitivity, retained imported choice, existing swell IRs/edits and
   saved mode survive; Recapture must be Off. Return between Learn and Perform
   and confirm the timeline/editor remain usable at the usual host size/scale.

No repeat of fault injection, extreme sparse ranges, exact-zero fixtures, frozen
recovery, pending export or the full DPI matrix is requested. These remain
explicitly automated/not host-tested; no #10 host waiver is inherited. Record
actual passes/failures/untested steps and the tested build/configuration here.


## Final inspection refinement — accepted and validated

The owner approved the revised native inspection preview on 2026-11-03. The
header/composition/control approvals and three host smoke passes above remain
accepted. No further host test was requested for this read-only refinement.
The eight refined interaction requirements are implemented in
`Source/UI/LearnWorkspace.h` and `Source/UI/LearnTimelineModel.h`, with focused
model/native tests in `tests/LearnTimelineCases.h` and `tests/swell_ui_test.cpp`.
Screenshots: `out/issue11-inspection-contract/`, including the unfocused pin and
real recording/acceptance states. The launched review remains open for the owner.

Current-source validation:

- `scripts/build.ps1 -Configuration Debug -Test`: **38/38 passed**, 132.83 s,
  wrapper exit 0; logs `out/build-logs/Debug/20261103-145813-af692d06/`.
- `scripts/build.ps1 -Configuration Release -Test`: **38/38 passed**, 44.69 s,
  wrapper exit 0; logs `out/build-logs/Release/20261103-150104-e555666c/`.
- Coverage includes product identity, shared UI, waveform/inspection, capture
  context/transactions, generation, persistence/migration, gain, streaming,
  transport/export and callback guards. Fixtures are native MSVC/CTest evidence;
  native screenshots/component event dispatch are not host or OS-injected tests.
- Strict all-item OpenSpec validation: **15 passed**. Diff whitespace check passed.

The 263-file current build-input manifest is
`out/issue11-inspection-build-inputs.json`, SHA-256
`829e3fa6c1567d880008d6bd16b6d91e66e8945f1b86930b0dbf453350669e45`.
Current identities are in `out/issue11-inspection-identities.json`; baseline
host-tested identities above are preserved separately.

| Current binary | SHA-256 |
| --- | --- |
| Debug ReverbArise VST3 | `cd56f32adece3c5204e4fef7336a970bfbc0bfc473cc0d4d7ecbae218ff225ec` |
| Release ReverbArise VST3 | `5b7afc04952804918bd2a8a0b41d2a5c45cfac4a596e149c286650c4121f9434` |

The current Release bundle uses the same path stated above. Plugin identity,
version, schemas, approved IR assets and memory limits remain as recorded in the
baseline section; this final refinement changes read-only UI interaction only.
The issue criterion crosswalk and adjacent-issue boundaries above were reviewed;
there are no unresolved owner acceptance decisions. Full OS-DPI/FL Studio scaling
and injected faults remain explicitly untested in the host; automated coverage
and actual usual-setup host passes are distinguished throughout this record.
No archive, commit, push or PR action is included in this completion.


## PR #39 main integration — 2026-11-03

Integrated `origin/main` at `09b3704` (19 previously missing commits) into the
issue branch after implementation commit `598b998`. Git merged without conflicts.
The three auto-merged documents (`docs/README.md`, development setup and IR library
guidance) retain the Learn references and new upstream links; their local Markdown
links resolve. Imported AGENTS/delivery guidance and release scripts/tests match
main under Git-normalized comparison. No upstream instructions were overwritten.

`git diff HEAD -- Source CMakeLists.txt CMakePresets.json cmake resources
scripts/build.ps1 scripts/build-support.ps1` was empty before the merge commit:
no native implementation/build inputs changed. The archived #11 directory and
its four canonical specifications likewise remain identical to `598b998`.
No re-archiving, UI changes, new state transitions or capture-context changes
were made. Existing Debug/Release 38/38 results and owner approvals remain valid;
full suites and host UAT were not repeated for unchanged native inputs.

Focused integration results (existing validated Release executables, exit 0):

- `preverb_workflow_processor_test`: pass.
- `preverb_capture_context_test`: pass.
- `preverb_generation_configuration_test`: pass.
- `preverb_ir_choices_test`: pass.
- `preverb_capture_transaction_test`: pass.
- `openspec validate --all --strict --no-interactive`: 17/17 items passed.
- `tests/release_wrapper_test.ps1`: 39 simulated packaging/orchestration checks
  passed; no native rebuild, CTest, DAW check or real release/publication was
  performed by this fixture.
- No unresolved merge entries; staged whitespace check passed.

Issue #38 is a separate future implementation. The current successful Stop →
Perform transition remains expected for #39. The deliberate future replacement
(Stop stays Learn; explicit Generate creates swells and enters Perform) is not
implemented here and does not change the current canonical or archived #11
contract. PR #39 remains for owner review and merge.
