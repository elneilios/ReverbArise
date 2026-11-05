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

Return those five blocks plus the expected swell/bar and two observations:
**Was wet audible? Was the Snap warning present?** No screenshot, video, new
recording or repeated DAW bounce is required for this check. Full text includes
the exact fixed-grid inputs, run/reference/host deltas, first and preceding
callback identities, current/historical gates, host loop context and worker
coverage; do not trim it to the summary line. If a separately attempted bounce
fails, retain its copied diagnostics after failure as an additional report.
