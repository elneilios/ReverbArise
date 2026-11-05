# Issue #73: fixed-tempo clock investigation

## Scope and attribution

Local branch `codex/73-live-grid-clock-investigation`, planning commit `5390da5`,
based on main `84894ee`. [Issue #73](https://github.com/elneilios/ReverbArise/issues/73)
tracks grid/live rejection separately from the existing offline repair.
Implementation results below will identify their tested revision and dirty state.
No new host observation, installation or accepted repair is claimed.

Supplied owner observations use 48000 Hz, 122 BPM, 4/4:

| Observation | Reference | Host | Established fact |
| --- | --- | --- | --- |
| Offline rejection | sample 3399344, PPQ 144 | sample 3399344, PPQ 143.999322222222219 | Source/host origin difference +16 samples |
| Later historical live rejection | run sample 3399344, PPQ 143.999322222222219 | sample 3795967, PPQ 160.801390972220531 | Run/host origin difference approximately -16 samples |
| Same live report | publication origin -0.262295081746 | host origin -0.262295041699 | Near-zero publication delta is a different comparison |

Original Snap-only loaded binary/revision and its exact contemporaneous tuple are
unavailable. The later offline build is attributable to implementation `1532f37`,
binary SHA256 `0A96FCCD07633CBAE7D00FB9A445E762A97C59F59AB6C3A2E7783F00B1605BD3`.
The report identifies committed capture revision 2, 61 swells in the earlier
instance, and later publication revision 14/live run 1/session 3/reset 3.
It includes source range [3752880,3776487), IR Large Wide Echo Hall.wav and
215819 rendered frames for the selected swell. Complete source/render content
fingerprints for those host observations are unavailable here; do not infer them
from names or a later installed build. No project/assets are modified to recover
that information. Injected native fixtures use synthetic audio and own identities.

## Existing offline repair and integration status

Local branch `codex/fix-offline-export-clock` retains implementation `1532f37`,
installation `09b5696` and subsequent owner result `a5b36d2`.
Inspect with `git show codex/fix-offline-export-clock:<path>`:

- `openspec/changes/fix-offline-export-clock/proposal.md`
- `openspec/changes/fix-offline-export-clock/tasks.md`
- `docs/validation/2026-11-05-offline-export-clock.md`
- `docs/validation/2026-11-05-offline-export-clock-installation.json`
- `tests/MultipleExportCases.h`

At investigation entry, `origin/main` is `84894ee`; a subsequent read-only
`git ls-remote origin refs/heads/main` confirmed remote head
`84894ee96f2d978886fceb0c3c09980de6457fa3`. `git branch --contains
1532f37` lists only the local offline/source-edge branches, not main or this
branch. The offline implementation is not integrated here. Its native results
were 68/68 Debug and Release, with independent zero/+16/-31-offset bounce arrays,
different block sizes/Playing flags and concurrent instances. The owner reported
"Export worked!", 7414 offline blocks and completed wet with host tail timing
reset. These are retained historical results, not new tests on current main.

| Surface | Existing evidence | Remaining work |
| --- | --- | --- |
| Offline missing wet | Local fix and accepted owner bounce | Separate main integration/review path; no repeat host bounce requested |
| Snap unavailable during Play | Current code compares accepted origin to UI host timing | Expose precise reason/input provenance; current warning's actual host sequence unresolved |
| Live historical rejection | Run origin differs by -16 despite near-zero publication delta | Retain first rejecting and preceding admitted callbacks; reproduce predicate independently |
| Actual alignment | Existing fixed-time native contracts | New independent supported-clock sample/audio assertions and preservation checks |

This change must not port offline code, output protection, tempo maps or unrelated
source-edge changes. Snap evidence cannot certify live admission; a historical
live rejection cannot invalidate the accepted offline bounce.

## Execution map

- 1.1/1.2: low-risk evidence/branch inspection; independent of code.
- 2.1: medium-risk equivalent grid result reporting; unit/integer oracle checks.
- 2.2: medium-risk UI binding; depends on 2.1, component/state/accessibility checks.
- 3.1: high-risk callback-owned observation/concurrency; fixed work, allocation
  guards, coherent-reader and lifecycle tests; no admission policy changes.
- 3.2: medium-risk UI formatting; depends on 3.1, native text/component checks.
- 4.1/4.2: high-risk transport/audio regression evidence; depends on observations,
  independent sample oracle, actual output and source/render/persistence checks.
- 5.1: cross-change native Debug/Release/UI/OpenSpec gates, after source stabilizes.
- 5.2: guide semantic review/currency after delivered behavior stabilizes.
- 5.3: final causal assessment and one unresolved-host collection procedure.

## Grid result semantics and focused verification

`HostGrid::make` still evaluates the original capture-rate and
`transportMappingMatches` predicates, then the same division/conversion math.
An optional value-only evidence output records accepted/observed inputs and the
first rejected prerequisite: absent sample position, capture rate, invalid
timing/origin, host rate, tempo, meter, accepted-grid origin, division or grid
conversion. Existing callers keep the optional grid API. The editor's evidence
uses the same timeline inputs; outside Perform it explicitly does not evaluate a
current grid. Unrepresentable individual snap candidates still return empty.

Early focused native Release `preverb_host_grid_test.exe` exited 0 on
`5390da5 (dirty)`, while the wrapper continued building other independent
targets. The integer/meter/tie oracle, +16/-31/zero offsets, exactly one versus
two samples, absent/nonfinite inputs, separate rate/tempo/meter reasons,
invalid division and overflow checks passed. This is an MSVC-built executable,
not a PowerShell fixture. The complete wrapper/CTest results are recorded below
after completion; this focused pass does not replace those gates.

Early focused native Release `preverb_processor_test.exe multiple-export`
exited 0. It exercises both zero-input instances, variable real-time partitions,
the independently declared five-sample admission ramp, unchanged captured-source
reference and existing save/restore/parallel-bounce safeguards. A later assertion
adds explicit before/after saved swell equality; the final gate must build/run
that assertion too. Stable-clock parity here is distinct from the historical
offline branch's nonzero-offset cases.

Focused Release `preverb_transport_playback_test.exe` exited 0. The two injected
owner-tuple sequences retain the correct first rejecting callback and admitted
predecessor despite later failures/loop flags, invalid timing and Stop. New Play,
clock reset and session replacement produce a new associated bundle. A clock
failure without a publication retains no fabricated admitted predecessor.
Existing rate/tempo/meter/range, seek/loop, retirement and recovery tests remain
active. The added asymmetric two-channel integer-sample oracle matches live and
offline mixing across partitions including exact five-sample joins/tails.

Native payload measurement: causal bundle **1192 bytes / 149 atomic words**.
Publication uses 149 payload stores plus two sequence stores, once per observed
positive-frame callback with a live clock. A bounded read makes at most eight
attempts, each at most 149 payload loads plus two sequence loads. There is no
dynamic trace or reader spin. Concurrent callback publication/control reads and
publication retirement pressure passed with no watched audio allocations or
deallocations. State contains values only, no publication/asset pointers. These
bounds and stress checks do not assert an FL Studio callback deadline.

Early focused Release `preverb_swell_ui_test.exe` exited 0. Native binding checks
exercise available -> +16-sample grid rejection -> compatible stopped grid,
Ctrl bypass, full accessible diagnostic text, normal helper font, glyph width,
scroll containment, unchanged serialized session/capture/render count and the
existing canvas/inspector exact-entry/stale-gesture suites. Existing minimum and
larger-size component checks run in that suite. This is native component evidence,
not Windows DPI or FL Studio acceptance. Later format precision/state assertions
and final source edits require the final wrapper gates recorded below.

## Causal interpretation and remaining host question

The recorded +16-sample source-to-host difference explains why the accepted
source grid rejects that tuple. It does not imply live audio is rejected: live
uses a separately pinned playing reference. Conversely, the later approximately
-16-sample run-reference delta explains the historical live rejection even when
the publication delta is nearly zero. Existing requirements intentionally reject
that within-run change. These comparisons reproduce the rejecting predicates;
they do not establish why FL supplied different observations or when it first
changed. The original Snap-only report has no same-callback provenance.

The native sequence fixtures deliberately inject both a shifted first callback
followed immediately by source-origin timing and a stable shifted prefix followed
later by source-origin timing. Either can explain a later snapshot. The retained
first rejection and admitted predecessor distinguish them in an actual future
host run; no invented drift tolerance or hard-coded offset is justified here.

Recommended follow-up: keep the accepted offline repair on its own integration
and review path; obtain the missing live transition evidence before approving a
Snap mapping or live admission change. A successful offline bounce should not be
repeated solely because a live history row exists. If a repair is proposed, name
its exact triggering sequence, safety invariants and independent sample oracle,
revise the planning artifacts and obtain owner approval before implementation.
Issue #73 remains open; this investigation does not deliver that repair.

## One future FL Studio evidence check

No installation or host operation was performed for this change. After a
separately authorized test-build installation, use a preserved FLP copy and keep
all captured/rendered assets; no recapture, clearing or regeneration is needed.

1. Confirm the loaded plugin's Settings > About build information and identify
   one known ready wet interval. Open Settings > Recovery > Open Diagnostics
   while stopped and retain the source/grid timing and current run state.
2. Start Song playback before that interval. When the warning/silence occurs,
   retain fixed-grid inputs/reason, run start, preceding admitted callback, first
   timing rejection, current/latest historical rows and worker coverage. Record
   the affected swell/source/render identity if available. These rows may have
   different provenance; preserve their block/run/session/reset labels.
3. Stop and retain the restored current grid versus retained run evidence. Use
   Copy diagnostics only if the separate offline change's existing action is
   integrated; otherwise retain the displayed/accessibility detail. No additional
   host bounce or automated edge-case repetition is requested without a specific
   new regression.

This checks the one unresolved host sequence. It is not a request to repeat the
accepted #53 playback, accepted offline repair, source-edge listening test or
unrelated host/DPI coverage. Original/current host attribution, actual transition
cause, Windows DPI and new host audio acceptance remain unverified.

## Native integration gates

Windows x64, Visual Studio 2026/MSVC 14.51.36231. Reused clean pinned source
caches: JUCE `3fb5cbc0340758596dac39f7c8de47d562b4896e` (9.0.4), WavPack
`5803634a030e2a11dba602ba057b89cc34486c67` (5.9.0). This worktree started with a
fresh binary tree, not copied binaries or a copied CMake cache. The source-only
cache seed initially lacked wrapper-required generator/home metadata; preflight
failed before configure/build/tests. Adding this tree's own matching metadata
resolved that setup error without deleting a cache or bypassing the build lock.
Build parallelism is four; CTest is serial for native UI focus ownership.

First `scripts/build.ps1 -Configuration Release -Test`: configure/build/test
exited 0, **71/71 passed**, CTest **250.35 s**, at `5390da5 (dirty)`.
Logs: `out/build-logs/Release/20261105-201423-1b2f9991/`.
This was a fresh Release binary build with warm dependency sources. Some late
diagnostic/verification refinements occurred during compilation; final incremental
Release and fresh Debug gates on the settled source are still required below.
No speed comparison is claimed. The full CTest matrix includes native C++ tests
and separately named PowerShell/CMake fixture/check gates; those fixture checks
do not substitute for the native grid, transport, processor and UI regressions.
