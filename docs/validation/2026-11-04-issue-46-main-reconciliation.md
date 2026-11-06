# Issue #46 — latest-main reconciliation

After final #46 host acceptance, the PR readiness check exposed an advanced
remote `main`: `3912ace966a6c2dbad5467685ed790cfddcda2f0`. The PR metadata still
reported the older Export baseline; a fresh Git fetch identified merged #52
(#4 spec/archive) and #57 (#53 live transport admission fix/diagnostics).

Merge source: `c4434200ba3887adc440a1c69d3335f2ffbee972`. The sole text conflict
was `docs/README.md`: retain both Perform/navigation and live-diagnostics links.
All shared production code merged automatically. Preserve #53's independently
validated per-run transport clock, admission diagnostics and tests unchanged;
retain #46's loop/Snap/knob/presentation behavior. No new DSP implementation or
control-family change is introduced by conflict resolution. The current Export
contract is unchanged by these later main commits. #4's upstream spec/archive is
retained; #46 is not synced/archived by this merge.

## Validation

`scripts/build.ps1 -Configuration Release -Test` at clean merge source `c443420`
passed preflight/configure/build and **57/58 native CTests in 232.19 s**. Logs:
`out/build-logs/Release/20261104-222123-42578241`. The single failure was imported
#53 workflow fixture readiness: it assumed the previous default Wet only audition
despite having only a tiny dry capture. #46 deliberately defaults to mixed mode
and rejects audition without full dry listening-window coverage. Live transport
readiness and manual mixed-audition readiness are separate contracts.

Fixture-only follow-up `0101e3a982a3c55d7615423a9dd0d07a381977d1` explicitly selects
Wet only for that clock/preview identity oracle and restores its original mode.
It retains both readiness assertions, before/after audio sample comparisons,
clock-origin rejection/admission, encoded session identity and captured-reference
checks. No production code changed to make the test pass. Focused native workflow,
Perform bindings, audition and hover checks passed **4/4 in 4.24 s**; logs
`out/build-logs/Release/20261104-main-reconcile-fixture-build.log` and
`20261104-main-reconcile-fixture-tests.log` in the same directory. Thus all 58
unique tests have passing coverage across the full and corrected focused runs;
this is not a single 58/58 wrapper run. No redundant full rerun followed the
fixture-only correction.

UI standards pass: **48 files / 61 bounded occurrences**, unchanged debt
inventory. Strict OpenSpec: **19/19 items**; the count drops from 20 because main
has archived the separate #4 change. Diff checks pass. Existing build cache,
MSVC toolchain and pinned JUCE 9.0.4 are reused, with the wrapper build lock.

The actual final integrated native preview is directly open from
`out/review-issue46-main-integrated/perform-main-integrated.exe`, using the ordinary
production editor. The prior completed preview was replaced. No revised visual
interaction or new control-family approval is needed for the documentation merge
and fixture setup correction.

## Bundle and acceptance scope

Separate complete six-file integration bundle, copied and hash-verified:
`C:\Users\neila\.codex\worktrees\670e\k7ma-preverb\out\host-smoke\issue46-integrated-c443420-20261104-222123\ReverbArise.vst3`.
Inner binary SHA-256:
`63E3121266CDFC2999CA5C37F2147B6E5824FCC40D1C936DF809B17512A58495`.
The adjacent manifest records source, merged-main revision, logs and all hashes.
No installed plugin or previously accepted bundle was overwritten.

The owner's #46 **Host smoke passes** applies to source `31a969b` and its exact
[presentation bundle](2026-11-04-issue-46-presentation-adoption.md). Main's #53
has its own [owner host acceptance](2026-11-04-issue-53-live-diagnostics.md#owner-acceptance-passed).
Neither report is relabelled as a host run of this combined binary. No further
host/version/DPI coverage is claimed, and no accepted edge cases are requested
again absent a concrete unresolved host failure. The remaining work is PR owner
review/merge; all 21 #46 implementation/acceptance tasks are recorded complete.
