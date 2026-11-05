# Issue #68 transport readout checkpoint

## Scope and execution map

OpenSpec: `stabilize-perform-transport-readout`. Branch:
`codex/68-stable-transport-readout`, based on main `84894ee`.

- Tasks 1.1/1.2: medium-risk GUI formatting/component work; component geometry,
  actual glyph fit, availability, rounding, accessibility and software-scale checks.
- Task 1.3: native owner presentation review; prerequisite for production integration.
- Task 2.1: medium-risk editor bindings; native refresh/mode and state-invariance checks.
- Task 2.2: low-risk guidance/standards/currency work after acceptance.
- Tasks 3.1/3.2: integrated native verification and review delivery.

The candidate component retains existing footer typography (15-logical-pixel
embedded Inter Regular), primary text ink and existing spacing. It adds no
highlight/background, interaction, transport or DSP behavior. Independent field
geometry and bounded overflow replace value-dependent combined-text centring.
See [component policy](../development/transport-readout.md).

## Environment and preparation

Windows x64, VS 2026/MSVC, Windows SDK 10.0.26100, pinned JUCE 9.0.4 and WavPack
5.9.0. This worktree began without a build tree. The initial configure explicitly
points FetchContent source overrides at existing pinned source directories in
`C:/git/k7ma-preverb/out/build/windows-msvc/_deps/`; build products stay in this
worktree. No cache is copied, cleared or modified in the source checkout.

Initial sandboxed configuration failed because MSBuild could not read the Windows
SDK registry. Elevated retry completed using the same local sources, with the
per-directory build lock held throughout. Logs: `out/transport-initial-configure.log`
and `out/transport-initial-configure-elevated.log`.

## Verification status

UI standards passed with 53 scanned files and 61 unchanged bounded occurrences.
Strict scoped OpenSpec validation and `git diff --check` pass.

The first Release wrapper run (`out/build-logs/Release/20261105-215536-2952bb22/`)
compiled shared/plugin/VST3 targets but was explicitly interrupted while building
unrelated tests to bring the native review forward. This is **not** a full wrapper
build pass or full CTest result. Its child processes stopped before focused work;
other chats' independent build processes were left untouched.

With the directory idle and its build lock held, the focused native Release target
`preverb_transport_readout_test` built successfully (two build workers); logs:
`out/transport-focused-build.log` and `out/transport-focused-build-final.log`.
Native CTest `^preverb_transport_readout_test$`: **1/1 PASS**, 0.75 s total
(0.70 s test), including supported 1100x850, 1400x900, 1400x1000, 1600x1000
logical footer widths and software 100/125/150/200% rendering. No generic speedup
claim is made. Initial dependency configuration/source reuse does not make the
first compilation a warm build; the corrected test-only rebuilds were incremental.

Two harness failures were corrected: the initial synthetic position fixture changed
samples without advancing PPQ, and the first accessibility assertion dereferenced
a null handler without a native peer. The fixture now supplies consistent host
coordinates, creates the required desktop peer and explicitly verifies a non-null
handler. These failures do not establish a production transport defect.

Evidence: 35 distinct PNG files in ignored `out/transport-readout/` (the two 1400-wide
footer overflow cases share identical dimensions/output), plus CTest's LastTest.log.
Normal-limit, unavailable and bounded-overflow screenshots were visually inspected
at the minimum width. Full text remains in static-text accessibility and help.

Reviewed preview executable SHA256:
`f3c3ca21f18d75a9335fef61e488fb3ef084c5c20e041c9c12255263222d9f6a`.
Native interactive `--review` preview launched directly (PID 63704). It cycles
synthetic playback values and offers limit/unavailable/overflow cases. Explicit
owner look/feel acceptance remains **pending**; preserve the review window.

No production editor integration, plugin installation, FL Studio test, physical
Windows DPI/multi-monitor review or owner acceptance is claimed at this checkpoint.
