# First main → Loop integration

## Scope and identity

- Worktree: `C:/Users/neila/.codex/worktrees/loop-main-integration/k7ma-preverb`.
- Branch: `codex/loop-main-integration`; PR target: `feature/loop-mode`.
- Loop parent: `c2a9758727439380d023e122fab57805dfc0a006` (includes PR #147 guide diagrams).
- Initial main parent: `a9302bb437b2b0d9525f97d29979528dee027956`.
- Final main checkpoint: `88bd5d0` (PR #148, revision-25 text/help/notices), synchronized in merge `d9c17b4`. The Loop target remained `c2a9758`.
- Integration began on 10 November; follow-up validation continued on 11 November.
- Initial resolved merge: `942bca3523e280635245a1bed7a94361e26eb7c8`; a real two-parent merge retains synchronization ancestry.
- This integrates existing approved contracts. No new OpenSpec change, feature scope, control family, visual variant, state schema or host identifier is introduced.

## Resolution decisions

- Preserve Loop capture, playback, source identities, phrase-local coordinates and stale-edit revision guards.
- Retain main's local inspector rejection feedback while also passing the captured Loop revision to the engine edit guard.
- Keep Song-only source-bar validation in the extracted preflight path and retain FL trace ownership.
- Use shared labeled switches inside the existing Loop policy viewport, the approved numeric input for Loop duration and main's shared waveform navigation. Preserve Loop Arm's approved destructive latch.
- Reset Learn inspection/navigation when accepted source coordinates or Loop identity change. Timer refresh preserves the current inspection and bounded viewport.
- Reconcile workspace requirements for both Song and Loop with main's waveform and list contracts; preserve all requirement headings from both branches.
- Preserve the Loop guide timing diagrams, merge main's UI instructions and clarify existing Song-only source bars, Generate on Stop and WAV Export. Regenerate the guide review record only after semantic review.

## Validation

Debug used `scripts/build.ps1 -Configuration Debug -Test` followed by focused
native reruns after fixes. The initial full run passed 99/103 in 1382.07 s;
its four failures were investigated rather than waived. The final supported
Debug build passed in `out/build-logs/Debug/20261111-001949-8230eeee/`.

The final-main targeted run passed Learn UI (26.05 s), the broad editor including
P3 text cases (106.21 s), Storage Settings (3.75 s), Portable Storage (1.61 s)
and working-folder controls (1.26 s). The Loop test then passed separately after
correcting its current-pass boundary; the isolated native readability case also
passed. Logs are `out/integration-review/debug-final-targeted.log`,
`loop-final-fixed.log` and `readability-debug-final.log`. Final Loop-fixture
compilation passed in `out/build-logs/Debug/20261111-002842-80c9835e/`.
These are full-run plus focused native results, not a claim that a second full
Debug run was executed on the final checkpoint.

Release validation is the next gate and will be recorded in the completion update.

- `scripts/check-ui-standards.ps1`: pass, 85 files / 48 bounded definition/debt occurrences after merging main revision 25. No debt inventory regeneration.
- `scripts/check-user-guide.ps1`: pass after explicit semantic review.
- `openspec validate --all --no-interactive`: 24 passed, 0 failed.
- Strict OpenSpec validation: 15 passed, 9 failed due to existing requirement-length warnings. Exact temporary baseline copies under `out/integration-spec-baselines/` confirm Loop already fails the same nine specs; main fails seven. Existing warnings are retained rather than changing unrelated specifications.
- Focused static checks: both parents' workspace requirement headings retained without duplication; guide IDs unique, fragment links resolve, timing-figure count unchanged from Loop, no scripts or remote image/frame resources.

## Findings resolved during validation

- Slow Debug Diagnostics text layout made its timer immediately overdue. Removed
  the duplicate measurement and measured its quiet interval from completion.
  Hidden idle Storage/working-folder polling also skips repeated session
  serialization, retaining active jobs and unconsumed results. The broad native
  editor then completed its source edits, 500-swell layout and remaining checks.
  Stack samples are retained under `out/integration-debug/`. The additional
  whole-editor timer rearm considered during diagnosis was not applied.
- The synchronous Loop UI fixture paused callbacks during expensive painting;
  one measured interval was 461 ms plus surrounding work. The real callback-gap
  guard correctly reported lost reference and fading. Native captures now keep
  actual processor callbacks running on a joined background thread. Stop checks
  use the current observed phrase boundary because audio can advance during a
  screenshot. Production gap protection was not weakened.
- A completed reload's epoch-only marker could suppress the inventory refresh
  after a same-epoch folder association. Diagnostics showed a correctly accepted
  new root but a displayed old-root report. The one-shot marker now matches
  epoch **and** instance root and clears when superseded. Component coverage
  verifies wrong-root rejection and one-shot matching consumption. Storage
  Settings and both storage component tests passed, followed by two additional
  consecutive Settings passes. The fixture waits for published UI readiness,
  rather than only worker completion.
- The initial readability failure was the native foreground gate recorded on
  mainline. Its isolated native rerun passed precision/ruler/grouping/mode/footer
  assertions. No assertion or focus prerequisite was bypassed.

## Native review evidence

Real disposable-fixture captures, 900×600 composition with 100%/125% user scale:

- [Loop Learn, 100%](2026-11-10-loop-main-integration/learn-loop-100.png)
- [Loop policy scrolling, 125%](2026-11-10-loop-main-integration/learn-loop-options-125.png)
- [Song/Loop Perform, 100%](2026-11-10-loop-main-integration/song-loop-perform-100.png)
- [Song/Loop Perform, 125%](2026-11-10-loop-main-integration/song-loop-perform-125.png)

The completion update will replace the initial captures with the final Release
checkpoint images and record the retained owner-preview output.

## Environment and limitations

Windows x64, Visual Studio 2026 / MSVC 14.51.36231, bundled CMake/CTest, pinned JUCE and WavPack. The new worktree began with a cold build/dependency tree. The initial serial Debug build was deliberately stopped after producing the plugin; its verified CMake/MSBuild/compiler descendants were stopped together. The wrapper then resumed the partial tree with `CMAKE_BUILD_PARALLEL_LEVEL=4`; no cache was cleared or copied from another worktree. Debug and Release use the wrapper's shared-directory lock sequentially.

No FL Studio installation, host smoke test, hardware-audio listening acceptance or physical Windows DPI coverage is claimed. Automated user-scale coverage is separate from host/DPI acceptance.
