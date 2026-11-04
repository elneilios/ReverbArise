# Issue #38 — latest-main reconciliation

## Scope and preservation

The owner requested merge preparation for PR #56 against latest `main`, preserving
accepted work and fixing integration regressions without weakening tests. Merge
commit `3ca116e` has parents `293875b` (#38) and `3912ace` (main). Main includes
Export PR #50 and issue #53's transport fix through PR #57. PR #48 remains open;
the owner explicitly selected **Latest main only**, so its unmerged Perform
workspace is not imported or represented as tested in this combined build.

The only textual conflict was in `tests/swell_engine_test.cpp`. Both the #38
provenance/edit protection block and #53's `runLiveClockEngineCases()` invocation
are retained. Production files merged automatically and were reviewed. The
per-run live clock, TransportPlayback and SwellTransport files match main
exactly. ExportVoice, ExportWorkspace and shared TimelineNavigation also match
main. The existing #38 SongExport metadata exclusion for `generationSource`
remains; it does not alter audible export composition or WAV identity inputs.

No accepted control styling, callback admission policy or Export interaction was
replaced. Existing owner-authored archives from main are retained; no #38 spec
sync/archive, issue closure or PR merge is performed.

## Retained owner acceptance

The owner confirmed the #38 host setup as **FL Studio 26.1.7.5653, 48000 Hz,
512 samples**. Reuse the overall working workflow report and the successful
low/high/low retry with pending automatic renders from the
[canonical #38 evidence](2026-11-04-issue-38-generation-workflow.md). Native Learn
and initial Perform selection/framing approval remains valid. Source `a235fa9`
and its recorded Release hash identify that accepted #38 implementation.

Main's independent [#53 host acceptance](2026-11-04-issue-53-live-diagnostics.md)
and [#50 Export acceptance](2026-11-04-export-shared-timeline-integration.md) remain
scoped to their recorded builds. Per the owner's instruction, these passing
results are retained where behavior is unchanged; they are not labelled as a
fresh FL Studio run of this integrated binary. No owner edge-case repetition is
requested. Physical OS-DPI and screen-reader acceptance remain untested.

## Integrated validation

The existing warm Windows MSVC tree is reused with the build wrapper's lock.
No passing pre-merge suite substitutes for these required gates.

| Command | Build log ID | Result |
| --- | --- | --- |
| `scripts/build.ps1 -Configuration Release -Test` | `20261104-224210-adbb546d` | 51/51 CTest, 99.07 s |
| `scripts/build.ps1 -Configuration Debug -Test` | In progress | Pending |
| `scripts/check-ui-standards.ps1` | Integrated tree | Passed: 43 files, 61 unchanged bounded occurrences |
| `openspec validate --all --strict` | Integrated tree | 19/19 passed |
| `git diff --check` | Integrated tree | Passed |

Release module SHA256:
`4BDB522277933524A20077239CD0A452CACB4DF17EE6F4D95D4BCADC0856DEB3`.
Bundle: `out/build/windows-msvc/preverb_plugin_artefacts/Release/VST3/ReverbArise.vst3`.
The wrapper identifies `3ca116e` with evidence/task-only dirty files; no production
code changed after the gates started. Final Debug result and main-reference
check follow once complete. No installed bundle or DAW project was overwritten.

## Main advanced during validation

The upstream check then found `423a9fd`, merging approved Perform PR #48 into
main. The owner-selected **Latest main only** now includes #48 through main;
no unmerged branch is imported. The `3912ace` integration above becomes
preliminary evidence. Final merge preparation must reconcile #48's optional
navigation/audition preferences, accepted production layout and bounded
navigation with #38's first-generation handoff, then repeat both full gates on
that latest-main integration. Its final results supersede the preliminary table.
