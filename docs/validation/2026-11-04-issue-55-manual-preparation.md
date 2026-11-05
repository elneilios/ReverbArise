# Issue #55 — explicit manual preparation

Issue: [#55](https://github.com/elneilios/ReverbArise/issues/55).
OpenSpec: `fix-manual-swell-generation`. Integration baseline: `1f47acf`
(merged #38). Windows x64, Visual Studio 2026/MSVC, Release.
Work began 4 November and native verification continued 5 November 2026.

## Reproduction and cause

The native `preverb_manual_preparation_test --creation-only` against the baseline
successfully initialized the processor, accepted an imported WAV default, captured
non-silent dry audio with master/markers/legacy refresh off, and inserted a manual
target. It timed out at the Ready/Audition predicate (test line 62 at baseline),
confirming that insertion alone did not produce a usable render. This decisive
baseline ran with the same elevated native environment as the wrapper. An earlier
sandboxed probe timed out during fixture initialization and is not the issue
reproduction result.

`addTarget`, accepted input edits and selected IR import completion gated forced
preparation on automatic flags. The existing coordinator already supports forced
manual requests. The fix routes these commands through that path, releases the
affected deferred Learn dependency and pins its current source revision. Capture
settlement, saved automatic settings, future-default ownership and publication
guards retain their existing policy.

`PluginInspector::refreshValues` initialized wet bounds to zero, left them there
when no raw wet render existed, then displayed arrival minus zero and zero minus
arrival. At 48 kHz a target at sample 3,729,840 therefore produced +77.705 s
pre-roll and -77.705 s end offset. The fix leaves these existing disabled cells
blank until bounds exist. This explains that numeric signature; it does not prove
the contents of the owner's unavailable original screenshot, source or IR file.

## Native verification

- First worktree build: local cached JUCE/WavPack source clones, cold native
  compilation. The serial invocation was deliberately stopped and resumed with
  four workers, preserving compiled files. Baseline build passed; no baseline
  full-suite pass is claimed.
- First candidate wrapper run (`out/build-logs/Release/20261104-235935-c497bb17`):
  build passed, 63/64 CTest tests passed. The new test waited for dry+wet Audition
  before selecting Wet Only, conflating dry audition coverage with wet generation.
  Its fixture now explicitly selects Wet Only before readiness and includes dry
  post-roll in the late-song source. No production behavior was relaxed.
- Focused corrected `preverb_manual_preparation_test`: passed, exit 0, 4.07 s;
  both WAV and .wv paths plus edit/failure/binding/publication cases ran.
- Final incremental `scripts/build.ps1 -Configuration Release -Test`
  (`out/build-logs/Release/20261105-000505-84431aab`), using the worktree wrapper
  lock: configure/build passed; **64/64 native CTest tests passed**, exit 0,
  285.45 s test time. The new manual-preparation test passed in CTest in 3.96 s.
  Logs identify `1f47acf (dirty)` plus the issue patch; these were pre-commit
  builds, not clean release-package validation. This is a warm incremental run
  after the initial native build, not a measured cold-build speed comparison.
- UI standards: `scripts/check-ui-standards.ps1` passed (49 files, 61 existing
  bounded definition/debt occurrences). Debt inventory unchanged.
- `git diff --check`: passed.

The passing focused native test covers real processor capture and non-zero Wet
Only Audition output
for WAV and encoded imported .wv responses; creation and dependency edits with
all automatic policies off; future-default versus selected assignment; frozen
guards; independent render retention; late-song and unprepared inspector values;
missing coverage/IR, invalid import/timing, deletion and session replacement at
render publication. Existing native suites provide additional resource, callback,
source integrity, persistence and dry-only capture-policy coverage; all 63 existing
tests passed both candidate runs, and the corrected new test passed the final run.
Strict `openspec validate fix-manual-swell-generation --strict` also passed.
No fixture-only build result is substituted for these native results.

## Owner acceptance and limitations

Owner follow-up, 5 November: '+' insertion generated a swell as expected, but
double-click insertion failed until a first swell had been added with '+'.
The new `--first-insertion` native reproducer failed at the expected one-target
assertion against `db0c84e` (baseline build log
`out/build-logs/Release/20261105-001838-c4f57c32`). Snap required session timing
that was only pinned by the first accepted target. The timeline now bootstraps
the grid from valid current host timing only for an empty session with no saved
mapping, retaining capture-rate, existing-map and host-position validation.
The regression exercises first/subsequent snapped double-clicks and those guards.
The corrected native `--first-insertion` test passed, exit 0 (1.22 s).
Updated `scripts/build.ps1 -Configuration Release -Test` passed configure/build
and **64/64 native CTest tests**, exit 0, 270.58 s test time, including the full
manual preparation/first-insertion test (5.36 s). Logs:
`out/build-logs/Release/20261105-002004-f65a4739`, identified as `db0c84e (dirty)`
plus this follow-up patch. UI standards, strict OpenSpec validation and whitespace
checks also passed. This was an incremental warm build; no speedup claim is made.

Pending: a focused FL Studio recheck of the first double-click using the updated
candidate; prior '+' generation is owner-confirmed and need not be repeated.
Verify dry capture with automation off, manual insertion, Ready/wet waveform,
sensible timing and audible Perform Audition. Automated native/component results
are separate from host acceptance. No additional host/DPI coverage is claimed.
The exact `large_hall.wv` file was not supplied; synthetic supported WavPack input
tests the same import/decode/managed-assignment pipeline. #53 transport playback
remains a separate issue.

Initial review bundle (superseded by the first-insertion correction):
`out/review/issue-55-20261105/ReverbArise.vst3` (not installed over any existing
plugin). Its `Contents/x86_64-win/ReverbArise.vst3` matches the built binary:
SHA-256 `7F974125ECFE57BB91289FFC285D70BA10AF71C28159C3ECF9CFC6423C99DEDA`.
The owner check should explicitly select Wet Only for wet-audio verification;
Dry + Wet continues to require the existing captured dry-preview coverage.

Updated review bundle:
`out/review/issue-55-first-double-click-20261105/ReverbArise.vst3`.
Its copied native binary matches the updated build:
SHA-256 `80BA38B17043ED2565B14648BA0C899738A867C3BE0197521185BE78A463B564`.
Only the failed first-double-click path needs a focused host recheck; '+'
generation already has owner confirmation. Keep the PR draft until that result.

No new control family, style token or geometry is introduced. The inspector
change corrects unavailable data in existing disabled controls, retaining their
look and interaction. No new family/style approval is requested for this fix.
