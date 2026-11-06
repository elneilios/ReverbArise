# Issue #73: stable playback Snap repair

## Scope and authority

Owner authorized Apply of `fix-stable-playback-snap`, local-only delivery and an
updated FL Studio installation after passing checks. Baseline is local proposal
`949ba4d`, with host evidence `e803323` and the offline fix already integrated.
The prior installed build is clean `02a3a17`, not a claim of acceptance of this
repair. The observed owner case was audible wet plus a Snap warning at a stable
approximately +16-sample source/run offset; the older live rejection is separate.

Grid quantization, ruler and inspector musical entry retain accepted captured
coordinates. Availability can use the dedicated validated current clock with
compatible rate/tempo/meter and the unchanged one-sample run guard. Rejected,
missing or stale playing authority cannot fall back to accepted-origin matching.
Telemetry, worker text, wet enable and asset readiness are not grid authority.
No offset tolerance, compensation constant, tempo map, callback work, output
protection, new control/style, persistent schema or plugin ID was introduced.

Native insertion coverage found that the old `addTarget` path copied the current
host tuple over accepted timing. Compatible generation/render restore paths did
the same. The repair retains accepted timing for those musically compatible
updates; first timing and genuinely different musical mapping follow existing
acceptance. No source/render sample position or identity is retimed. Tests cover
target insertion during shifted playback, stopped recovery and restore under a
shifted host clock.

## Focused implementation checks

Existing Windows MSVC Release/Debug build trees and pinned source caches are
reused. No cache clearing or cold-toolchain speedup is claimed. Build-only wrappers
and native CTest are reported separately; fixture checks are not native audio or
host acceptance.

Focused Release core/transport/processor tests passed during development. The
first UI assertion read throttled Diagnostics before refresh; its test now waits
for the existing refresh without changing the assertion or timeout. A later knob
fixture changed viewport without refreshing its inspector range; refreshing the
fixture resolved its coordinate expectation. The insertion/Stop assertion then
exposed the real accepted-anchor overwrite described above. These failed focused
runs remain in ignored `out/issue-73/snap-focused-release*.log` and
`snap-ui-release-3.log`; they are not counted as passing gates.

## Requirement coverage

| Contract | Native evidence |
| --- | --- |
| Captured grid under shifted clock | Exact owner decimal tuples; integer/rational sample oracle; positive/negative/zero offsets and strict default factory cases |
| Genuine failure remains gated | One-sample boundary, drift back to source origin, invalid timing/range, incompatible rate/tempo/meter, stale/unavailable current context |
| Lifecycle and isolation | Dedicated clock session/reset/run checks, Stop/new Play, independent instances, engine reset/restore and unchanged saved assets |
| Shared interactions | Native canvas drag, knob drag, wheel, keyboard, target insertion, Ctrl bypass, exact entry and rejected stale knob intent |
| Readiness independence | Real processor clock with absent IR/render and disabled wet output; ordinary run progress preserves a canvas gesture |
| Diagnostic distinction | Accepted editing anchor versus run validation reference, informational offset versus rejection, full copied/accessibility text and existing lifecycle checks |
| Audio preservation | Positioned asymmetric real-time/offline reference with variable blocks and zero input; shifted-run PCM equals supported unshifted live output; zero latency and retained offline offset/drift cases |
| Persistence | Existing current/legacy save/reopen suites plus shifted-context render restore without accepted-anchor overwrite; no grid clock serialized |

## Final gates and installation

First full Release on clean implementation `eed71ff` passed **70/71** in
**210.74 s** (`out/build-logs/Release/20261105-234952-59424d16`), failing the
retained-IR assertion in `ir_handover_test`. Its fixture restored the engine then
continued an offset playing context without another callback; restore correctly
retires the old run. The old automatic grid-anchor overwrite had masked the
missing current authority. The fixture now explicitly verifies the retired clock
is unavailable and supplies a real callback before continuing its playback/IR
scenario. Every retained-IR/audio assertion and timeout is preserved; no audio
admission code or drift guard was changed to pass the test. Two consecutive
focused native Release handover runs passed after this lifecycle correction;
the third reached a separate later timeout at line 118 (33.50 s total), which
was traced to deleting a dependency while its verified prepared window remained
warm at the same position. Stop alone does not require an asset reopen. The
fault fixture now seeks beyond that window to force fresh dependency validation
under a confirmed stopped clock, retaining the original invalidation assertion
and timeout. This is a fixture correction, not a passing repeat gate or a new
production cache/admission policy.

Focused final core/UI/transport/processor Release passed **4/4** in **23.19 s**
(`out/issue-73/snap-focused-release-4.log`), including shifted-context render
restore and the captured-anchor overwrite regression. The source was proposal
`949ba4d` plus dirty implementation before its `eed71ff` commit. These focused
results supplement, rather than replace, the final full gates below.

Full native Release on clean `bad5bec` passed **71/71** in **197.87 s**
(`out/build-logs/Release/20261105-235653-a0f108f4`). Production code is unchanged
from `eed71ff`; the later fault-fixture correction only forces the missing-asset
case to perform a fresh read. Its final focused repeats and native Debug gate
remain required below. Valid full Release coverage of unchanged production and
other tests is retained rather than rerunning the whole suite for a test-only
precondition correction.

Pending final native Debug gate, corrected-fixture repeats and verified system installation.

Final-fixture Release handover passed five consecutive runs (**5.91 s**,
`out/issue-73/snap-final-handover-release.log`) after a clean `f7a6294` wrapper
build. Debug configure/build then passed on that revision, but its UI test
captured the previously unexplained transient assertion: source end was exactly
20000, selected phase was Rendering / "Convolving", and the neighbour was Ready.
The worker had legitimately advanced beyond Stale before inspection. That Debug
run was cancelled after the known failure, before completing the remaining
engine/processor cases; it is not a pass (`out/build-logs/Debug/20261106-000204-9c752c2f`).

The UI fixture now takes one coherent model copy and verifies the input revision
advances exactly once, the requested endpoint/shape and unrelated ready swell
are preserved, the selected state is a valid preparation/ready state, and any
already-ready render matches its new source bounds. It no longer assumes a
specific transient worker phase survives until an arbitrarily timed read. This
adds revision/render-coherence checks without altering production or timeouts.
The corrected final UI fixture and full Debug gate remain to be verified below.

### Completed final verification, 6 November

Clean final source is `1c6085753477914a70f574c7dfe6a22ee5785512`; production
code remains identical to `eed71ff`. Its Release wrapper build passed
(`out/build-logs/Release/20261106-001108-38acc2fc`), followed by the corrected
native Release UI fixture passing **1/1**, **12.20 s**
(`out/issue-73/snap-final-ui-release.log`). The full **71/71 Release** result on
unchanged production above remains applicable; final test-only corrections are
covered separately instead of mislabelled as another full Release suite.

The final `scripts/build.ps1 -Configuration Debug -Test` passed configure,
build and **71/71 native CTest**, **499.48 s**, on clean `1c60857`:
`out/build-logs/Debug/20261106-001147-ee11c9be`. The corrected UI case passed
inside that full run (**54.20 s**), as did retained IR, engine, processor,
streaming, lifecycle, concurrency, persistence and existing offline cases.
The final source diff after `eed71ff` contains test/evidence changes only.

UI standards passed (**51 files / 61 existing bounded occurrences**), strict
OpenSpec validation passed, whitespace checks passed and user-guide currency
passed after explicit semantic review. The production guide/layout/controls and
debt inventory are unchanged. Native results do not establish new FL Studio or
DPI acceptance. Heavy native work was coordinated with #58; one bounded
two-worker incremental compile slot was allowed during early Debug, with no
concurrent host installation. These timings are validation measurements, not
performance/speedup claims.

### Verified local FL Studio installation

The existing normal-write ACL denial at the same system destination was already
established during the preceding combined installation. After all gates passed,
the supported `scripts/install-test-vst3.ps1` was run with Windows UAC elevation,
hosts closed, no ACL changes and no manual bundle copying. The installer rebuilt
the Release VST3 target at clean `1c60857` and returned **0**. Its plugin-only
build did not rerun CTest; the native gates above preceded installation. Build
log: `out/build-logs/Release/20261106-002400-ab3b223f`.

- Identifier: **v0.1.0-local**, Git `1c6085753477914a70f574c7dfe6a22ee5785512`, source unmodified.
- Installed: `C:\Program Files\Common Files\VST3\ReverbArise.vst3`.
- x64 binary SHA256: `0114e1f8480c69cba8ca74b407b76cd9b0c46653ea1789fa9f1ff69cae0a7abd`.
- Retained previous bundle: `out/vst3-install-backups/20261106-002409-9092f151b7f349c78cedf40040f9d35f/ReverbArise.vst3`.
- Independent verification: all **six installed files** match the final Release
  output and all **six retained files** match the saved pre-install inventory.
  The previous binary hash is `dc53c456aa3cb2f82f2ff2a962c59d9ee8fa6357808bd64eb81ea3f5c36e06af`.

Logs, pre-install inventory and verification JSON are retained under ignored
`out/issue-73/snap-*`. Preserve the backup before removing this worktree/output.
The install helper disabled reuse of its MSBuild worker servers so the UAC
launcher's descendant wait could complete; dependency/build caches were retained.
No DAW was terminated, owner preview closed, other installation changed, project
asset touched, release published, PR opened, push performed, spec synced or change
archived. The system build includes the existing offline repair and diagnostics.
Canonical specs are not synced; the approved delta governs this local repair.
Issue #71 remains separate, with
its own host-map coverage/precision and persistence review requirements.

## Minimal owner check after installation

On a copy of the same fixed-tempo project, confirm the installed About identity,
start playback and verify Snap stays available. Try one source/timing gesture
with Snap On and confirm its inspector value agrees; Stop should retain the
accepted grid. Native tests own drift, missing assets, modifiers, persistence and
audio edge cases. No repeat accepted bounce or five-block diagnostic collection
is requested unless a fresh failure occurs. FL Studio acceptance and untested
host/DPI coverage remain distinct from installation and automated passing.

## Owner FL Studio acceptance — 6 November 2026

Following delivery of installed test build `1c60857`, the owner confirmed:

- Snap stays available during playback.
- The snapped edit matches the inspector.
- Wet audio is still working.

This accepts the requested local fixed-tempo Snap smoke check and confirms
audible wet in that owner run. No further repetition of this check or the
previously accepted offline bounce is requested. It does not establish new
host/DPI coverage, tempo-automation support or a reproduction of the older live
rejection. All work remains local; no push, PR, issue closure, spec sync or
archive follows automatically from this acceptance.
