# Focused offline export clock repair — 5 November 2026

Branch `codex/fix-offline-export-clock`, new managed worktree
`C:\Users\neila\.codex\worktrees\offline-export-clock\k7ma-preverb`, based on
fetched `origin/main` **2d1c849**. The separate output-protection branch remains
untouched. No push or PR is authorized. Issue #71's tempo-map work is separate.

## Evidence and implemented boundary

The owner diagnostic retained prepared sample **3399344**, PPQ **144**, and
offline host sample **3399344**, PPQ **143.999322222222219**, both 48 kHz,
122 BPM, 4/4, valid. The source/host origin difference is exactly **16 samples**.
That exceeds the old source-origin guard. It is distinct from the prior A/B
build's 128-sample delay. A later parked observation agreed with the source;
one failing observation alone does not prove offset stability across a bounce.

Offline preparation now pins its own per-instance fixed-tempo run reference and
reuses the existing `LiveClockSnapshot::accepts` validator for retained IRs,
asset admission, subsequent callbacks and the actual mixer. Source/render
coordinates and musical compatibility remain unchanged. Unsupported within-run
origin drift still fails at the existing one-sample tolerance. The global
mapping helper, Live, Capture, Snap and internal Export code are unchanged.
Existing backward snapshot restarts, lifecycle and completed-wet tail behavior
remain. There is no hard-coded 16-sample compensation or output-protection code.

Failure text includes prepared, pinned reference and failing host timing plus
source/run origin deltas. Copy diagnostics reuses `Controls/ActionButton` in the
existing Diagnostics row, retaining full clipboard text and current Snap/live/
worker evidence. No new control family, style token or exception was introduced.
See [living guide](../development/offline-export-clock.md).

## Native verification

Windows x64, Visual Studio 2026/MSVC, JUCE 9.0.4 and pinned WavPack 5.9.0 source
caches reused through ignored `out/export-clock/warm-dependencies.cmake`.
The caches were clean: JUCE tag/HEAD `3fb5cbc0340758596dac39f7c8de47d562b4896e`;
WavPack HEAD `5803634a030e2a11dba602ba057b89cc34486c67`.
This is a **fresh binary build tree**, not a warm binary build. No CMake cache
or plugin binary was copied from another checkout. Build parallelism is four;
CTest stays serial because native UI fixtures assert desktop keyboard focus.
No performance speedup or DAW deadline guarantee is claimed.

- `scripts/build.ps1 -Configuration Release -Test`: **68/68 passed**, CTest
  **231.86 s**. Logs `out/build-logs/Release/20261105-104646-789d6997/`;
  tested source was `2d1c849` plus this worktree's changes.
- `scripts/build.ps1 -Configuration Debug -Test`: **68/68 passed**, CTest
  **458.47 s**. Logs `out/build-logs/Debug/20261105-110850-f4e58752/`;
  same production/test source as the Release gate.
- Early focused Release `preverb_processor_test`, `preverb_transport_playback_test`
  and `preverb_ir_handover_test` passed after their targets linked; the later
  full Release gate passed them again without source changes.
- Exact owner tuple: global source-origin comparison still rejects it, while
  the explicit runtime clock admits resident synthetic wet at its original
  sample position. Subsequent two-sample drift is rejected by the inner mixer.
- Processor export fixtures: offset 16 / -31 bounces equal their original
  zero-offset sample arrays exactly across different block sizes, either host
  Playing flag and concurrent independent instances. Output is explicitly
  nonzero. Saved swell definitions/assets remain equal and host latency is zero.
- A three-sample change from the pinned offset reference fails with the exact
  run-origin delta. Existing sticky failures, unavailable members, gain plans,
  routing, resource limits and tail cases remain in the processor suite.
- IR handover fixture: offset offline timing retains the accepted audible IR
  despite a pending/failed requested replacement, without swapping composition.
- UI fixture: native Space invocation copies all expected diagnostic sections,
  preserves session state, verifies accessible button title/contained geometry
  and contains no experimental protection section.
- UI standards passed: **51 files / 61 existing bounded occurrences**, inventory
  unchanged. Strict OpenSpec validation and diff whitespace checks passed.

[Tested source hashes](2026-11-05-offline-export-clock-source.json) identify the
eight changed production/test files independently of the later local commit.
All native gates passed on the first complete run; no failed test was waived.
Separate installer/build-script mock fixtures were not rerun because those
scripts were unchanged. Native passing is separate from the owner's original
FL Studio bounce acceptance; host/DPI acceptance is not claimed.

## Installation

Installation verification is recorded here after the installer completes.

Historical follow-up: local commits `09b5696` and `a5b36d2` on
`codex/fix-offline-export-clock` record the original installation and owner's
"Export worked!" result. The owner's later explicit request to combine that fix
with #73 diagnostics is delivered separately in the
[combined test installation](2026-11-05-issue-73-combined-test-install.md).
Original source hashes and native results above remain historical.
