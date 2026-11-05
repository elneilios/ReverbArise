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

Pending final native Debug/Release wrapper gates and verified system installation.
The completed results below will identify source revision, dirty state, logs,
installed identity/hash and retained prior bundle. Canonical specs are not synced;
the approved delta governs this local repair. Issue #71 remains separate, with
its own host-map coverage/precision and persistence review requirements.

## Minimal owner check after installation

On a copy of the same fixed-tempo project, confirm the installed About identity,
start playback and verify Snap stays available. Try one source/timing gesture
with Snap On and confirm its inspector value agrees; Stop should retain the
accepted grid. Native tests own drift, missing assets, modifiers, persistence and
audio edge cases. No repeat accepted bounce or five-block diagnostic collection
is requested unless a fresh failure occurs. FL Studio acceptance and untested
host/DPI coverage remain distinct from installation and automated passing.
