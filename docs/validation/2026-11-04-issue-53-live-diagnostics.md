# Issue #53 diagnostic implementation evidence — 4 November 2026

Issue: https://github.com/elneilios/ReverbArise/issues/53
OpenSpec: `diagnose-perform-transport-silence`.

## Scope and causal status

This increment adds read-only callback/worker observations and the existing
Diagnostics binding. It does not change timing tolerance, rewrite source/render
positions, reset an instance or claim to repair the reported silence.

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

OpenSpec tasks 1.1-1.4 and 2.1 are complete (5/11). Repair/retry/lifecycle tasks
and final fix acceptance remain open. Task 3.1 needs owner review of the mapping
contract change described below before behavioral edits. Although native
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
