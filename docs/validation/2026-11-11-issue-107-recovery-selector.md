# Issue #107 Recovery onset selector validation

On 11 November 2026 the owner authorized reuse of the approved C2 dropdown for
Settings → Recovery → Onset suggestions and waived a separate preview approval.
The adoption uses the existing selector presentation and no new visual tokens.

The native selector binding test exercised the integrated Settings page's name,
empty guidance and body height. With deterministic suggestions on a native peer,
it verified cancellation, one explicit keyboard commit, a refresh that leaves
the popup open, retention after reorder, an out-of-viewport 64-bit sample,
rejection after a stale mapping, first-visible fallback and an empty list. The
shared popup test covers pointer, keyboard, accessibility, focus and overflow.
The production Confirm button still calls the existing engine command under the
existing workflow eligibility checks.

`scripts/build.ps1 -Configuration Release -Test` passed all 100 tests on Windows
MSVC from `7924e57` with the Recovery changes. It reused the existing build tree.
The initial build found a namespace error in the new test assertion, corrected
before the passing run. The final CTest run took 328.96 seconds. Logs are under
`out/build-logs/Release/20261111-023052-9877b918/`. UI standards, strict change
validation, guide currency and whitespace checks passed. FL Studio, physical
Windows DPI and a live screen reader were outside this run.
