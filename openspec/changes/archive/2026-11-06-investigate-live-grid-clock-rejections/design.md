# Design

## Context

See proposal.md for motivation and issue #73 scope. Planning baseline is main
commit `84894ee`, on local branch `codex/73-live-grid-clock-investigation`.

Current paths inspected:

- `Source/Core/HostGrid.h`: `make` checks capture rate and
  `transportMappingMatches(accepted, now)`, then validates grid conversion. It
  returns an empty optional for all failures. `SwellTimeline::gridTiming` uses
  accepted session timing except for the existing empty-session bootstrap.
- `Source/UI/SwellTimeline.h/.cpp`: `gridAvailable` and guarded drag setup use
  that authority. The visible message only says fixed timing is required.
  `Source/Plugin/PluginInspector.cpp` also consumes the same grid factory.
- `Source/Playback/LiveTransportClock.h`: the callback pins the first valid
  playing observation, retains it through seeks/loops and rejects subsequent
  origin differences exceeding one sample. A confirmed Stop ends that run.
- `Source/Playback/LiveDiagnostics.h` and `Source/UI/LiveDiagnosticsText.h`:
  bounded coherent current/latest-rejection observations already include the
  run reference. The printed numeric delta is publication-to-host, not
  run-reference-to-host. The last rejection alone cannot reconstruct the first
  transition or its preceding admitted block.
- `Source/DSP/SwellEngine.cpp` and `Source/DSP/SwellTransport.cpp`: worker/live
  and offline preparation have distinct admission paths. Their results cannot
  be inferred from Snap or one another.

The owner supplied two different comparisons at 48000 Hz, 122 BPM, 4/4:

| Context | Reference | Observed | What it establishes |
| --- | --- | --- | --- |
| Failed offline entry | sample 3399344, PPQ 144 | same sample, PPQ 143.999322222222219 | Source-to-export origin delta of +16 samples |
| Historical live rejection | run sample 3399344, PPQ 143.999322222222219 | sample 3795967, PPQ 160.801390972220531 | Approximately -16 samples from the live run reference, despite near-zero publication-to-host delta |

The local `fix-offline-export-clock` change, implementation `1532f37`, pins an
independent per-bounce clock and uses it in outer preparation and the inner
mixer. Installation `09b5696` and owner-result commit `a5b36d2` record the
successful bounce, 7414 offline blocks and completed-wet tail reset. Retrieve
its artifacts/evidence using `git show codex/fix-offline-export-clock:<path>`;
the relevant paths are `openspec/changes/fix-offline-export-clock/` and
`docs/validation/2026-11-05-offline-export-clock.md`, with installed inventory in
`docs/validation/2026-11-05-offline-export-clock-installation.json`. They are
absent from this main-based tree. Canonical live and grid requirements
still intentionally reject the recorded incompatible mappings. A warning alone
does not prove sample audio is misaligned or that this is a regression in #53.

## Goals / Non-Goals

**Goals:** expose the exact rejecting predicate and observation sequence; prove
the distinction between grid and audio admission; preserve the existing offline
result as a separately versioned baseline; produce evidence sufficient to choose
the smallest supported repair or identify the remaining host question.

**Non-Goals:** changing the authoritative mapping, run-start/recovery policy,
one-sample guard, sample coordinates, gain, render validity or saved state. This
change does not port the offline repair, introduce tempo-map support, detect
Song/Pattern, or implement an unreviewed live/Snap fix.

## Decisions

### 1. Explain the same grid decision that gates edits

Extend the existing grid authority with a nonallocating result/reason containing
the accepted/observed inputs, capture rate and failure dimension; keep the
existing optional factory interface as a delegating convenience if needed. The
editor supplies actual sample-position availability and reports it explicitly.
Do not create a parallel approximate predicate in UI code. Successful grid
rounding and every existing rejection must remain equivalent under the old API.
Include invalid division/conversion as well as invalid timing, rate, tempo,
meter and origin; avoid mislabelling an unavailable prerequisite as drift.

Compute/format grid evidence on the UI/control side from the same inputs used by
that grid evaluation. Label it as a UI snapshot; do not claim it is simultaneous
with a callback or worker snapshot. Preserve the empty-session bootstrap and
canvas/inspector stale-gesture and exact-entry policies. Replacing accepted grid
timing with live timing was considered and deferred: it changes musical edit
semantics without establishing which mapping is trustworthy.

### 2. Retain a fixed amount of causal live evidence

Extend callback-owned observation state with a fixed-size run-start observation,
last admitted observation and a latched pair containing the first timing
rejection and its preceding admitted observation. Include block/range, run,
session/reset, valid timing, Playing and available loop context. Preserve the
existing current and latest historical rejection views. First-rejection evidence
does not advance on subsequent failures or later recovery within the same run.
Use explicit availability for a run with no admitted callback.

Publish the bundle through the existing bounded coherent atomic mechanism. UI
read retries remain bounded; a failed read is unavailable. Keep ownership on the
callback and reset the causal bundle only under existing new-run/session/audio
reset lifecycle, never because the editor opens or a worker publishes a page.
Compute textual origins/deltas outside the callback, using finite/checked
arithmetic. No file writes, logging, dynamic queue, strings or new locks on the
real-time path. A rolling trace was considered but a fixed causal bundle answers
the immediate question with a smaller, explicit work/memory bound; it cannot
prove what happened between unretained observations.

### 3. Separate evidence from a repair decision

Use `tests/host_grid_test.cpp` and `tests/transport_playback_test.cpp` for exact
recorded tuples, stable zero/positive/negative offsets and a within-run change.
Replay both plausible sequences: a shifted first callback followed by a stable
source-origin clock, and a stable shifted run followed by a later change. Tests
must identify their injected assumptions; neither establishes FL Studio's cause.
Preserve the reference after rejection, including through seek/loop and invalid
callbacks, then verify the existing confirmed Stop/new Play behavior.

Use resident synthetic asymmetric wet with known absolute intervals in transport
and processor fixtures. Assert nonzero admitted output, exact positioned sample
content and declared transition/gain behavior against an independent oracle;
compare real-time and offline at the same song positions for supported clocks.
For rejected timing, assert the existing gate and bounded suppression instead of
inventing parity. Reuse the offline branch's exact offset/equality regressions
and documented accepted result as separate evidence; do not cherry-pick it or
claim its old test counts validate current main. A future integration of that
repair remains governed by its own change and owner request.

### 4. Reuse the existing diagnostic presentation

Affected inventory: existing Diagnostics text/scroll content, existing timing
formatters and Snap availability/accessibility binding. Keep the main timeline
hint concise. Reuse shared production text roles, embedded Inter fonts and
existing auxiliary-window/scroll policies. No new controls, style tokens,
density variants, popup or workspace layout change is needed.

The offline branch owns its Copy diagnostics ActionButton. Reuse it if that
change has independently been integrated by Apply time; otherwise do not port or
create a duplicate action merely for this investigation. Evidence must remain
inspectable in the existing window either way. Component/binding tests must
check full accessible text, contained/scrollable content and unchanged saved
state/assets/jobs. If implementation reveals a need for a new/revised family or
visual exception, stop for a scoped plan/owner review, launch the prepared native
preview directly and preserve its window for review before wider rollout.

### 5. Minimize host work and preserve attribution

Create a dated #73 evidence record linking existing offline evidence by branch,
commit and path. Record tested revision/dirty state, running build identity where
available, exact source/session timing, source/render fingerprints, positions,
gates, publication coverage and provenance for every comparison. Distinguish
supplied host observations, injected reproduction and untested hypotheses.

If native evidence leaves the host transition unresolved, prepare one concise
FL Studio collection procedure on a preserved project copy: identify the loaded
build and observe stopped -> Play through one known wet interval -> Stop. Collect
Snap and causal live evidence together. Do not repeat the accepted offline bounce
or passing automated cases absent a concrete regression. Installation or host
operation requires separate authorization; this scope remains local. Honest
unavailable host/build/DPI coverage is a valid investigation finding, not proof
that #73 is resolved.

## Risks / Trade-offs

- Added atomic payload increases bounded callback work -> record payload size,
  fixed publication work and native stress results; reuse existing ownership and
  memory-lifetime checks, without claiming a DAW deadline guarantee.
- A later UI snapshot can disagree with historical callback timing -> label
  provenance, preserve complete individual tuples and never join mismatched
  observations into one asserted host event.
- A deterministic rejection can appear to prove a host defect -> distinguish
  predicate reproduction from causal host reproduction and retain the unknowns.
- Changing a warning could hide a safety failure -> leave admission and snapping
  results unchanged and assert equivalence plus actual audio/state invariance.
- Existing offline fix is absent from main -> keep its branch/evidence intact;
  report integration status rather than silently expanding this investigation.

## Migration Plan

No persistent migration is needed; evidence remains ephemeral. After a fresh
Apply request, implement diagnostics/tests locally, run focused native checks,
then full Debug and Release wrapper/CTest gates plus UI standards. Review the
user guide with `maintain-user-guide` for delivered diagnostic/spec changes and
run its read-only currency gate. Record commands, environment, warm versus fresh
build status, results and limitations in one dated evidence record.

Finish with a causal assessment and a reviewable recommended repair scope, or an
explicit unresolved-host finding. Do not automatically implement that repair:
update planning and obtain owner approval first. No push, PR, install, canonical
spec sync, archive or issue closure is authorized. Local diagnostic code can be
reverted without touching project assets; preserve source/render evidence and
the existing offline branch throughout.

## Open Questions

- At which actual FL Studio callback does the run origin first change, and is
  that associated with Play entry, a seek/loop, a host mode transition or none of
  those? The investigation captures evidence; it assumes no answer.
- Does the owner's current Snap warning use the same tuple as the historical
  live rejection? Existing text from different snapshots cannot establish this.
- What exact binary/project produced the original Snap-only report? Record it
  if recoverable; otherwise label that attribution unavailable rather than infer
  it from the later offline installation.

These are investigation outputs, not deferred choices about an approved fix.
