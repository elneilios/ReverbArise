# Combined offline repair and timing diagnostics test build

## Authorization and scope

On 5 November 2026 the owner explicitly requested a local FL Studio test build
combining the completed #73 diagnostics with the existing offline export fix,
plus exact timing-investigation capture instructions. This supersedes the
earlier diagnostic phase's no-integration/no-install boundary for this delivery
only. Work remains local; no push, PR, release publication, sync or archive.

Branch `codex/73-live-grid-clock-investigation`, based on diagnostic closeout
`c66d00e`, integrates existing implementation `1532f37` from the preserved local
`codex/fix-offline-export-clock` branch. Its production change is reused without
altering the strict shared mapping helper or live-run policy. The old offline
proposal/design/spec/tasks and original validation are retained as historical
context, not presented as new results. Its prior owner acceptance is recorded
at `a5b36d2`; this delivery does not request that successful bounce again absent
a new failure.

Conflicts were limited to the documentation index/collection guide and additions
at the same two test insertion points. Both diagnostic and offline cases remain:
stable 16/-31-sample bounces, real within-bounce drift rejection, positioned live
versus offline output, per-instance state, accepted IR handover and causal history.
The original Copy diagnostics ActionButton is reused in its existing row; it
copies the expanded grid/live causal text as well. Native clipboard assertions
check these sections, keyboard invocation, containment and unchanged saved state.
No output-protection spike, tempo-map implementation or source-edge change is
copied from the historical test branch.

## Validation and installation

The combined source will be identified by its local implementation commit in
Settings > About. The ordinary build identifier remains `v0.1.0-local`; no
release tag/candidate is created. Full combined Release and Debug wrapper/CTest
results, guide currency and installation inventory are recorded below when
completed. The installer builds Release plugin-only after those gates and uses
the supported system VST3 destination with a retained complete previous bundle.
Installation alone does not establish new FL Studio playback acceptance.

Combined implementation `4c39b06` was built with the existing warm MSVC/JUCE/
WavPack trees, without clearing caches. Full native Release passed **71/71**
in **227.11 s** (`out/build-logs/Release/20261105-220910-5eae01b4`).
The first combined Debug run failed 5/71 under heavy competing compilation
(CPU 97–100%, up to six `cl.exe` processes). Its log is
`out/build-logs/Debug/20261105-221500-e38547f8`; this is a failed gate, not a pass.
A quieter full Debug rerun passed **70/71** in **545.44 s**
(`out/build-logs/Debug/20261105-223512-51725706`): streaming, composition preview,
engine and processor recovered, but the source-handle assertion in `swell_ui_test`
repeated. Failure-only logging was added to record the actual source end, phase,
neighbour phase and interaction status, without changing production behavior or
the assertion. With that instrumentation, two direct Debug runs, one focused
CTest run (**29.99 s**) and five consecutive CTest repeats (**156.45 s**) passed.
The original intermittent assertion has not been explained by a captured failing
value; instrumentation is retained for the next failure. No tolerance, timeout,
expected phase or production behavior was relaxed. Final full Debug validation
passed **71/71** in **538.16 s**, on clean source `02a3a17`
(`out/build-logs/Debug/20261105-225030-3a48c846`). The source-handle test also
passed in that full run. Production code is byte-for-byte unchanged from the
combined Release gate at `4c39b06`; only test failure logging and this evidence
were added. These are native MSVC/CTest results, not FL Studio acceptance.

UI standards passed (51 files / 61 existing bounded occurrences), user-guide
currency passed after the recorded semantic review, and both active changes
passed strict OpenSpec validation. No shared timing tolerance, UI debt inventory
or generated workflow file changed. Processor regressions verify zero host
latency; no output-protection experiment was included.

### Installed identity and retained previous bundle

The supported installer rebuilt the Release VST3 target at clean source
`02a3a1739765c346251af956eaea91336cfb439d`, identifier **v0.1.0-local**.
The installer's plugin-only build did not rerun CTest; the full gates above
preceded installation. The first normal attempt built successfully but Windows
denied opening the system install lock, before replacement. The same script was
rerun with Windows UAC elevation and returned **0**. Its Release wrapper log is
`out/build-logs/Release/20261105-230105-a655847b`; transcript and verification are
retained under ignored `out/issue-73/`.

- Installed: `C:\Program Files\Common Files\VST3\ReverbArise.vst3`
- Previous bundle: `out/vst3-install-backups/20261105-230112-d9cfe774d7eb42a6bff912bfa2372b9c/ReverbArise.vst3`
- Installed x64 binary SHA256: `dc53c456aa3cb2f82f2ff2a962c59d9ee8fa6357808bd64eb81ea3f5c36e06af`
- Independent inventory comparison: all **six installed files** match the built
  Release bundle; all **six backup files** match the saved pre-install inventory.
  FL Studio was closed; no host was stopped and no other installation was changed.

The retained backup is inside this worktree's ignored `out` directory: preserve
it before removing the worktree or its output. Installation does not resolve the
live timing investigation or imply a new host/DPI acceptance result. No repeat
of the previously accepted offline bounce is requested absent a fresh failure.

## Exact capture procedure

After installation, reopen FL Studio. In Options > Manage plugins enable
**Verify plugins** and **Rescan previously verified plugins**, then Find installed
plugins. Confirm ReverbArise resolves to
`C:\Program Files\Common Files\VST3\ReverbArise.vst3`.

Use a copy of the affected constant-tempo project and the same affected plugin
instance. Keep its existing source, swells, routing and Snap preference; do not
recapture, regenerate or clear anything. Select one previously affected ready
swell and keep that selection for the whole check. Use Song playback, starting
before that swell's intended wet interval.

Retain five labelled text blocks (paste each somewhere before copying the next):

1. **BUILD** — Settings > About > Copy build information. Confirm its Git commit
   matches the installed implementation recorded below.
2. **ASSETS** — Settings > Recovery > Open Diagnostics > Copy asset info. This
   identifies the working project/instance and selected IR/render paths.
3. **BEFORE PLAY** — while stopped, use Copy diagnostics.
4. **DURING PLAY** — start once and let playback reach the affected interval.
   While the warning/silence is present, use Copy diagnostics **before Stop or
   seek**. If the symptom does not occur, copy while wet should be audible and
   say whether it was heard. Do not seek, restart, toggle Snap/output or switch
   modes during this run.
5. **AFTER STOP** — stop once and immediately use Copy diagnostics again, before
   another Play or project reload. This preserves the completed run's first
   rejection and predecessor alongside the stopped grid.

Use FL Studio's transport buttons for Play and Stop; Space while a plugin button
has focus can activate that button. After copying DURING PLAY, click FL Studio's
Stop button, then paste that clipboard text under DURING PLAY. Now copy and paste
AFTER STOP. This avoids switching applications during the live run and preserves
both snapshots. Paste every block before the next copy replaces the clipboard.

Return those five blocks plus the expected swell/bar and two observations:
**Was wet audible? Was the Snap warning present?** No screenshot, video, new
recording or repeated DAW bounce is required for this check. Full text includes
the exact fixed-grid inputs, run/reference/host deltas, first and preceding
callback identities, current/historical gates, host loop context and worker
coverage; do not trim it to the summary line. If a separately attempted bounce
fails, retain its copied diagnostics after failure as an additional report.
