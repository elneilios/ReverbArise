# Issue #38 — latest-main reconciliation

## Final scope

The owner requested PR #56 against latest `main`, preserving approved Perform,
Export and transport behavior. Main advanced during the first validation pass:
PR #48 merged as `423a9fd`. The owner's **Latest main only** choice therefore
includes #48 through main. Merge `6f41af9` integrates that head; final production
integration fixes are in `56c5a5a`. No unmerged dependency branch was imported.
Main already includes Export PR #50 and issue #53's fix through PR #57.

The approved compact Perform layout, inspector, Snap/knob policies, looping
listening controls, footer help and standards revision 5 are retained. ExportVoice,
ExportWorkspace and shared TimelineNavigation match main exactly, as do the
per-run live clock and transport playback files. Existing main-authored archives
are retained; no #38 sync/archive, issue closure or PR merge is performed.

## Conservative resolution and regression checks

- The first merge (`3ca116e`, main `3912ace`) conflicted only in the engine test.
  Both #38 provenance protection and #53 live-clock assertions were retained.
- The #48 merge had six conflicted files. Codec resolution keeps strict v11
  Auto-generate/fingerprint fields plus optional `performUi` and audition
  preferences. A focused migration check verifies an accepted v10 UI/Export
  session restores with legacy Stop ON while preserving navigation and music.
- Engine snapshot resolution keeps coherent workflow/model publication and
  main's `audioAllowed` audition observation. No clock or callback policy changes.
- Editor/timeline resolution retains main's formatting, list count, bounds and
  saved-navigation authority. Restore preferences before the initial reveal;
  publish that reveal through existing navigation persistence and suppress later
  fallback Fit. Pending source/IR timing keeps the lead-in navigable without
  inventing a wet waveform. Checks include a pending protected manual swell,
  saved initial view, both Generate triggers, empty visits and delayed renders.
- The list fixture preserves the real Recapture arm, per-pass analysis advance,
  exact counts, Ready, frozen protection, ordering and persistence assertions.
  Its actual recapture exposed a production bug: explicit Perform Regenerate did
  not clear Learn's dependency deferral. The corrected command clears deferral
  and calibration protection only for work it explicitly queues. The test first
  asserts the protected manual render is stale and retained, then uses the real
  Perform action and verifies Ready with changed audio but unchanged identity,
  name, IR, shape and provenance. Frozen neighbouring work remains identical.

No assertion or timeout was relaxed. Native diagnostics were added to the failing
wait. The existing #38 SongExport exclusion for nonaudible `generationSource`
metadata remains; audible export composition and immutable WAV handling are unchanged.

## Retained host acceptance

The owner confirmed **FL Studio 26.1.7.5653, 48000 Hz, 512 samples** for the passing
#38 tests. The overall working workflow report, Ready-state regeneration and
successful low/high/low retry while automatic renders remained pending are in
[the canonical #38 evidence](2026-11-04-issue-38-generation-workflow.md). The
final accepted #38 implementation was `a235fa9` with its recorded Release hash.
Native Learn and initial selection/framing approval remains valid.

The approved [#48 presentation/host result](2026-11-04-issue-46-presentation-adoption.md),
[#53 Song playback result](2026-11-04-issue-53-live-diagnostics.md) and
[#50 Export result](2026-11-04-export-shared-timeline-integration.md) retain their
recorded build scope. Per the owner's explicit instruction, passing evidence is
reused where behavior is preserved; these results are not relabelled as a fresh
host run of the combined binary. Task 6.3's missing environment record is complete.
Physical OS-DPI and screen-reader acceptance remain unverified. Desktop inspection
was stopped with Escape; the metadata came from the owner's reply. No installed
bundle, audio settings or saved DAW project was changed by the agent.

## Validation history and final gates

The existing warm Windows MSVC tree and wrapper lock are reused, without cache
clearing. No pre-merge result substitutes for the final integrated gates.

- Preliminary main `3912ace`: full Release `20261104-224210-adbb546d` passed
  51/51 in 99.07 s; Debug `20261104-224455-e430abef` passed 51/51 in 369.07 s.
- Focused codec/navigation build `20261104-225506-7bf2e74b` passed, followed by
  passing session and initial-generation editor executables.
- First full #48 integration Release `20261104-225947-5ea2ea7e`: 62/63 passed,
  263.87 s. The stale protected recapture case above failed the unmodified Ready
  wait. Diagnostic build `20261104-230450-9395f09c` reproduced its exact state.
- Corrected focused build `20261104-230721-04aa76f8` and the full native UI
  executable passed, retaining every original assertion and adding stronger ones.

| Final command | Build log ID | Result |
| --- | --- | --- |
| `scripts/build.ps1 -Configuration Release -Test` | `20261104-230901-9da8a408` | 63/63 CTest, 251.24 s |
| `scripts/build.ps1 -Configuration Debug -Test` | `20261104-231323-d5f1b1e9` | 63/63 CTest, 541.56 s |
| `scripts/check-ui-standards.ps1` | Integrated tree | Passed: 49 files, 61 unchanged bounded occurrences |
| `openspec validate --all --strict` | Integrated tree | 20/20 passed |
| `git diff --check` | Integrated tree | Passed |

Final source is `56c5a5a`. Release module SHA256:
`E947BB81B0C13629EED73AFECE5B53F875BF544E939CABA53195F9A4415CDEB6`.
Bundle: `out/build/windows-msvc/preverb_plugin_artefacts/Release/VST3/ReverbArise.vst3`.

The last upstream check found `939f656` (PR #61), changing only contribution /
issue-reference guidance. Merge `f6a624d` includes it. `Source/`, tests, scripts
and CMake are identical to the validated production integration; no full native
suite is repeated for that documentation-only merge. Existing main-owned skill
changes were imported unchanged, not customized by this task.

A final private native editor fixture passed again to capture the integrated
[Learn](2026-11-04-issue-38-integrated-learn.png) and
[pending initial Perform](2026-11-04-issue-38-integrated-perform.png) rasters.
These are supporting automated evidence, not a new owner host/DPI approval.
No production edits follow the final gates. Later evidence/task/PR-only edits do
not require native repetition. The PR stays unmerged; owner review controls merge.
