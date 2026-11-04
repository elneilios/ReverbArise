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
Task 2.1 remains pending until causal values/state are available.

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

OpenSpec tasks 1.1-1.4 are complete (4/11). Task 2.1 is the causal-evidence gate;
repair/retry/lifecycle tasks and final fix acceptance remain open. Although native
Debug/Release validation passed for this diagnostic increment, task 4.1 remains
open for the eventual repair's final integrated validation. Task 4.2 is not a
request to perform acceptance on a diagnostic-only build.

The diagnostic Release bundle is staged under
`out/issue-53/diagnostics-release/ReverbArise.vst3` with a SHA-256 inventory and a
whole-bundle ZIP for internal evaluation. It is not an official packaged release
or an installed replacement. Build provenance is base `9b8e70f` dirty as recorded
by the wrapper; the subsequent issue commit contains the corresponding source.
Use a preserved FLP copy and retain the original plugin installation when loading
the diagnostic build. A playing/stopped observation of the Current callback,
Historical last rejection and Worker sections is the next evidence required.
