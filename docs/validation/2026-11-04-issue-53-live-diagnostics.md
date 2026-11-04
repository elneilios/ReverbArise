# Issue #53 diagnostic implementation evidence — 4 November 2026

Issue: https://github.com/elneilios/ReverbArise/issues/53
OpenSpec: `diagnose-perform-transport-silence`.

## Scope and causal status

The initial increment added read-only callback/worker observations and the existing
Diagnostics binding. The subsequent owner-approved live clock repair is described
below; historical pre-fix results remain preserved. No stored source/render
positions, project identities or timing tolerance constants were migrated.

Code inspection confirms that playing admission can reject an existing
publication and that Stop clears the play-gated incompatibility flag. This does
not prove an origin mismatch in the owner's instance. Old screenshots lack the
precision needed to compare the one-sample mapping tolerance. Injected gate tests
prove telemetry and classification, not equivalence to the preserved FLP failure.
The subsequent owner recording below establishes the failed predicates; the initial delivery had left task 2.1 pending.

## Environment and verification

Windows x64 / Visual Studio 2026 MSVC, repository wrapper and populated dependency
cache. Starting revision `9b8e70f` with the issue implementation dirty; no cold-cache
speedup is claimed. Wrapper logs retain the exact environment and native output.

- Callback-only native Debug: `scripts/build.ps1 -Configuration Debug -Test`;
  46/46 CTest tests passed, 204.00 seconds test time. Logs:
  `out/build-logs/Debug/20261104-175306-02cc83db/`.
- Integrated native Debug: `scripts/build.ps1 -Configuration Debug -Test`;
  46/46 CTest tests passed, 283.42 seconds test time with native screenshot
  generation enabled. Logs: `out/build-logs/Debug/20261104-180105-00b2eee9/`.
  The UI test rendered its full fixture set (96.94 seconds); this timing is not
  comparable to the earlier run without screenshots. Subsequent source cleanup
  removed formatter shadow warnings and retained cancellation text when asset
  reads return no error; final Release validates those diagnostic-only edits.
- Final native Release: `scripts/build.ps1 -Configuration Release -Test`;
  46/46 CTest tests passed, 96.14 seconds test time. Logs:
  `out/build-logs/Release/20261104-180744-fc08a3d0/`. Screenshot generation was
  disabled. This covers the final diagnostic source, including the cleanup above.
- `openspec validate diagnose-perform-transport-silence --strict` and
  `git diff --check` passed.
- UI standards guard passed with 42 scanned files and 61 explicitly bounded
  existing definition/debt occurrences; inventory was not regenerated.

Added callback tests cover timing dimensions, invalid callback range, workflow
and enable exclusion, a historical rejection surviving Stop, copied values
surviving publication retirement, concurrent readers and callback
allocation/deallocation guards. Added worker checks verify stopped preparation,
source/host mapping rejection, unchanged last-attempt history during eligibility
checks, disabled playback and re-service after compatibility returns. Native UI
checks cover full-precision reason formatting, distinct historical/current
observations, real snapshot binding, shared typography, contained measured bounds
and unchanged hidden-panel refresh behavior. Existing JSON/capture checks verify
opening Diagnostics leaves persistent session and source revisions unchanged. Wrapper/lint fixtures remain distinct from actual
MSVC compilation and native CTest execution.

The native [Diagnostics evidence screenshot](2026-11-04-issue-53-live-admission.png)
was visually inspected. It shows a test instance stopped with a compatible
publication, not the owner's failing FL Studio instance. Existing flat viewport,
Label and shared helper text role are reused; no control-family variant, new
visual token or policy exception was added. Original warnings in SwellList and
existing numeric conversions are outside this increment.

## Host and preservation limits

No failing FLP, host-loaded instance, captured source or managed asset was modified.
FL Studio reproduction, audible output acceptance and host/DPI coverage remain
unverified. After the diagnostic build is available, gather one playing/stopped
pair across a known wet range using the
[living field guide](../development/perform-live-diagnostics.md). Keep current
callback, historical rejection and worker observations distinct. Do not repeat
passing automated cases as owner UAT.

## Handoff status

At the diagnostic/evidence handoff, tasks 1.1-1.4 and 2.1 were complete (5/11).
The owner subsequently approved the revised live mapping plan. Current repair
verification and remaining owner acceptance are recorded below. Although native
Debug/Release validation passed for this diagnostic increment, task 4.1 remains
open for the eventual repair's final integrated validation. Task 4.2 is not a
request to perform acceptance on a diagnostic-only build.

The diagnostic Release bundle is staged under
`out/issue-53/diagnostics-release/ReverbArise.vst3` with a SHA-256 inventory and a
whole-bundle ZIP for internal evaluation. It is not an official packaged release
or an installed replacement. Build provenance is base `9b8e70f` dirty as recorded
by the wrapper; the subsequent issue commit contains the corresponding source.
Use a preserved FLP copy and retain the original plugin installation when loading
the diagnostic build. The owner subsequently supplied the playing/stopped recording analyzed below.

## Owner recording: stable playing versus stopped origin offset

Source: `20261104-1822-15.7563438.mp4`, 11.2 seconds, 6,871,741 bytes;
SHA-256 `21ba13a5d3549f74dd9197c821455e56a97348fd538dd2fd8e12854a2951b492`.
Unmodified frames at 0/2/4/6/8/10 seconds were decoded and inspected locally under
ignored `out/issue-53/video-evidence/`. The recording and full project UI frames
are not committed. This recording shows **7 ready swells**, distinct from the
original issue report's 61-swell instance; the same failed gate is observed.

The published/source tuple remains sample **3,399,344**, PPQ **144**, 48 kHz,
122 BPM, 4/4, valid. Its inferred origin is approximately **-0.262295081746**
samples. Publication and current/prepared revisions remain **30**; worker attempt
remains **16**, with its last actual result `Published complete window`.

| Recording time | Callback sample | Callback PPQ | Origin delta vs publication | Observed state |
| --- | ---: | ---: | ---: | --- |
| 0 s | 3,436,475 | 145.572232638889488 | ~16 samples | Playing, origin rejection |
| 2 s | 3,524,262 | 149.290987500001989 | ~16 samples | Playing, origin rejection |
| 4 s | 3,620,901 | 153.384722916670171 | ~16 samples | Playing, origin rejection |
| 6 s | 3,718,770 | 157.530562500005061 | ~16 samples | Playing through swell, origin rejection |
| 8 s | 3,815,410 | 161.624340277784341 | ~16 samples | Playing, origin rejection |
| 10 s | 3,399,344 | 144.000000000000000 | 0 samples | Stopped; old prepared window retained |

Across inspected playing frames, the callback is enabled, workflow allows live,
routing is Dry + Wet, linear wet gain is 1, range is valid, and rate/tempo/meter
are unchanged. Playing inferred origin is approximately **15.7377048** samples;
the difference from publication is within 0.000001 sample of 16. Callback counts
vary (including 245, 10, 139, 62 and 190), so this is not a whole-current-block
mismatch. Worker observations independently show the same delta and
`Source/host mapping incompatible`. At Stop, the delta returns to zero and the
worker reports `Prepared window retained` without a new attempt/publication.

The specific cause of silence is established: `transportMappingMatches` requires
origin difference <=1 sample. Both callback admission and worker preparation
compare the stopped/source origin against the playing origin and reject it.
The first playing observation is inside the old window [3,387,344, 3,495,344),
so window coverage alone does not explain that admission rejection. Later
positions exceed that window while the same source/host guard prevents preparing
replacement pages. This is compatibility refusal, not evidence of slow FFT or
publication-queue throughput. `missing=false` does not establish coverage when
mapping admission already failed.

JUCE's VST3 wrapper directly copies `processContext.projectTimeSamples` and
`projectTimeMusic` into the host observation. Processor diagnostics publish those
fields coherently. The evidence proves their cross-state offset reaches both
consumer paths; it does not establish whether FL Studio latency compensation or
another host mechanism creates the offset. No host-cause speculation is treated
as a finding.

### Deterministic native replay

`tests/transport_playback_test.cpp` replays the exact 0-second timing tuple against
a revision-30 publication with resident synthetic wet and unity dry input routing.
It reproduces origin rejection with valid callback range and configuration,
unchanged dry-only output, clearing on Stop and persistent rejection on replay.
PCM is synthetic; this is a replay of the owner's observed timing mechanism, not
a reproduction of the entire project or its captured audio.

- `scripts/build.ps1 -Configuration Debug`: native MSVC build passed (tests not
  requested by the wrapper), built from `ee1c6e6` dirty. Logs:
  `out/build-logs/Debug/20261104-182902-4effc9b7/`.
- `out/build/windows-msvc/Debug/preverb_transport_playback_test.exe --issue-53-expected-wet`:
  **expected pre-fix failure**, exit 1 at the actual wet-output assertion
  `left[244] > .25f`. Complete prepared synthetic wet stays silent.
- In the idle wrapper tree, matching native CTest Debug with
  `-R '^preverb_transport_(playback|time)_test$' --output-on-failure`:
  **2/2 passed**, 0.18 seconds. Normal tests verify the rejection mechanism and
  the existing timing guard; the opt-in expected-wet mode demonstrates the bug.
- Full suites were not repeated for this test/evidence-only increment; prior
  full Debug/Release results above apply to the diagnostic production code.

### Repair decision requiring owner review

The existing spec explicitly classifies a >1-sample origin change as unsupported;
the approved design excludes a new tolerance and task 3.1 requires owner review
before changing the mapping contract. The recording therefore identifies a
contract/host-compatibility mismatch rather than permission to raise the guard
to an arbitrary 16-sample threshold.

Recommended direction for review: preserve all stored absolute source/render
positions, keep rate/tempo/meter and asset/session dependency checks, and validate
an ephemeral playing transport mapping separately from the parked/source mapping.
Account for the demonstrated stopped/playing offset, continue checking stability
within playing transport, and revalidate before worker publication. Do not
silently migrate saved mappings, shift rendered audio, hard-code 16 as a global
correction, or remove genuine incompatibility reporting. The exact admission and
rejection rules must be captured in revised planning artifacts and reviewed before
implementation. No behavioral repair or tolerance change has been made.

## Implemented live clock repair

The callback now owns an ephemeral live run mapping, published independently of
read-only telemetry for worker validation. Parked/source origin difference is
informational. Within a run the existing strict one-sample mapping guard remains
unchanged; invalid timing/ranges do not fake Stop, and seeks, loops and output
controls do not silently rebase a rejected mapping. Session/audio reset requires
fresh runtime ownership. Fixed asset metadata and generation checks still gate
actual output. Stored sample positions and the shared strict mapping helper are
unchanged. Capture, Audition, internal Export and host offline retain their
original timing contracts.

The worker validates this same live authority before eligibility/fast-path reuse
and again before publication. It rejects superseded clock/session/source/render
work and stale windows that no longer cover the current destination, then retries
without claiming candidate readiness. Existing resident PCM can join at its
original absolute position; no 16-sample correction or waveform regeneration is
needed merely because the host starts playing.

### Regression coverage and results

- The exact recorded-value `--issue-53-expected-wet` mode now exits **0** with
  actual resident synthetic wet, including first-block admission; previously it
  failed its wet-output assertion. Source/publication origin delta remains 16
  samples while the actual run reference is compatible.
- Managed-asset engine cases cover **7 current, 61 current and 61 frozen** swells
  at 48 kHz / 122 BPM / 4/4, with zero, 16 and 64-sample cross-state offsets.
  They assert warm/cold/later/loop samples, within-run drift refusal, unavailable
  timing, live/Perform gating, confirmed Stop/Play, independent instances,
  unchanged persisted data/asset identities and no replay of elapsed wet.
- Held preparations deterministically exercise Stop/Play, restore, shape/render
  revision, source generation and late destinations. An injected publication
  fault verifies bounded retry. The stale-window case rejects an obsolete window
  before consuming that fault, proving revalidation rather than eventual luck.
- Real processor cases compare Audition samples before/after offset live playback,
  preserve captured source references and persisted session encoding, and compare
  internal Export samples while host timing has the offset. Accepted IR replacement
  live samples and host offline samples retain their existing references.
- Callback allocation/deallocation and concurrent clock/diagnostic-reader checks
  pass. Fresh-run rate/tempo/meter incompatibilities still reject assets. The shared
  strict transport-time tests remain unchanged.
- Native Debug wrapper `scripts/build.ps1 -Configuration Debug -Test`:
  **46/46 CTest checks passed**, 176.44 seconds; logs
  `out/build-logs/Debug/20261104-191911-bb91516d/`. This includes the final worker
  coverage guard. The later line-wrap cleanup and extra 61-current case are covered
  by final Release and supplementary focused Debug validation below.
- Final native Release wrapper `scripts/build.ps1 -Configuration Release -Test`:
  **46/46 passed**, 75.66 seconds; logs
  `out/build-logs/Release/20261104-192255-54dcfc50/`. A single diagnostic panel was
  rendered via `PREVERB_UI_LIVE_OUTPUT`; other screenshot output was disabled.
- Supplementary focused native Debug on final text and all expanded cases:
  **6/6 passed**, 82.41 seconds, after wrapper build
  `out/build-logs/Debug/20261104-192803-298743a7/`. The idle-tree CTest filter was
  `^preverb_(transport_(playback|time)|swell_engine|ir_handover|workflow_processor|swell_ui)_test$`.
  The final recorded-value `--issue-53-expected-wet` invocation also exited 0.
- A final test-only lifetime cleanup makes fixture workflow state outlive its
  engine worker. Debug wrapper build `20261104-193625-0219615b` passed, followed
  by the focused native engine test passing in 67.96 seconds. Production source
  and the validated Release binary were unchanged by this cleanup.
- UI standards guard passed (42 files / 61 existing bounded definition/debt
  occurrences); strict OpenSpec and diff checks passed. PowerShell wrapper/lint
  fixtures are distinct from MSVC compilation and the native C++ tests; registered
  fixture checks also pass within the full CTest gates.

Builds reuse populated caches and report base revision `548ea0f` dirty. The earlier
screenshot-enabled Debug run passed 46/46 in 257.20 seconds before the final
coverage guard, with logs `out/build-logs/Debug/20261104-191340-7d66168e/`.
Timings have different configurations/output scopes and are not a speed comparison.
Two new fixture setup mistakes (a >5-second capture commit and a metadata revision
instead of the persisted PCM fingerprint) were corrected using the existing
admission/fingerprint APIs; production source/asset guards were not weakened.

The final [native diagnostic screenshot](2026-11-04-issue-53-fixed-live-admission.png)
was visually inspected. Live and worker references have separate rows; a glyph
width assertion checks the panel text fits without shrinking its shared font.
No new control family, style token or exception was introduced. This synthetic
native screenshot is not proof of host playback or DPI acceptance.

### Remaining owner check

The native recorded-timing failure is fixed. One brief Song playback of the known
swell on the previously failing instance/copy is still required for task 4.2.
Automated edge cases, Audition/Export sample comparisons and asset integrity checks
need not be repeated by the owner. FL Studio/DPI coverage remains unclaimed until
observed. Do not merge or archive based solely on native passes.

Installation is pending coordination: the installed standard-path VST3 changed
externally during validation and FL Studio loaded it. Its binary SHA-256 was
`1A38813E7960935DD058C18C48F5ED60903B0C6C4B4467AEDB8DDB8A78AB6BD1`,
different from this branch's tested Release binary
`5CE4A70D2A391C9957F2F2F8EBE3ADC11EEFAD4389536DE67AF6B718224D271E`.
The pre-copy identity guard stopped before any backup or installation mutation.
Do not overwrite that unknown loaded build until its installer is coordinated.
### Owner test installation

The owner identified the external build as one they installed, then confirmed FL
Studio was closed. On 2026-11-04 at 19:46 UTC, the complete installed bundle was
copied to `out/issue-53/installed-backup-20261104-194616/ReverbArise.vst3` and all
six files verified against its backup manifest. The initial ordinary copy was
blocked by Windows access controls; an administrator-approved helper then installed
the tested fix from commit `e1fe0b7`. Independent post-install SHA-256 verification
confirmed all six installed files match the staged Release manifest. The installed
binary hash is `5CE4A70D2A391C9957F2F2F8EBE3ADC11EEFAD4389536DE67AF6B718224D271E`.
No project or captured/rendered asset was modified. Owner Song playback acceptance
remains pending; task 4.2 is still unchecked.
### Owner acceptance passed

After reopening FL Studio with the verified installed fix, the owner responded
"yes it is working now" to the requested Perform/Song playback through the
previously silent swell. This completes the brief host acceptance for task 4.2
on 2026-11-04. Project/asset preservation and unchanged Audition/Export remain
supported by the native identity and sample-reference checks above; the owner
was not asked to repeat those passing cases. Broader host/version/DPI coverage
is not implied. OpenSpec Apply now reports 11/11 tasks complete. The change is
ready for owner PR review; merge and archive remain separate actions.