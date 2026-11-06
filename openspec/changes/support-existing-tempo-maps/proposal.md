# Proposal

## Why

GitHub issue: [#71](https://github.com/elneilios/ReverbArise/issues/71).
Existing tempo automation currently trips fixed-tempo checks throughout capture,
rendering and playback. After the separately accepted #73 repairs, songs with
pre-existing tempo steps and ramps need sample-aligned audio without fabricated
musical precision or compulsory recapture at every BPM change.

## What Changes

- Capture bounded host timing observations alongside absolute source samples;
  accept existing tempo changes with constant sample rate and meter, retaining
  seeks, loops, silence and sparse coverage truthfully.
- Introduce immutable, versioned timing evidence with coverage, provenance,
  precision and conflict state. Commit it atomically with source and preserve
  old fixed-tempo projects and prior asset references.
- Separate audio/sample compatibility from musical conversion. Keep accepted
  source and verified wet at their stored samples through ordinary tempo
  changes; use the same positioned contributions in Perform and DAW bounce.
- For new source windows without a verified beat endpoint, use arrival-local
  observed tempo to choose a disclosed fixed sample duration, as approved by
  the owner during planning; do not substitute a distant playhead's BPM.
- Make shared rulers, bar/range entry and Snap consume verified local evidence.
  Unsupported ramp interiors and gaps show unavailable musical conversion;
  sample/time edits and safely aligned audio remain usable.
- Preserve fresh constant-tempo capture's existing affine Snap, ruler and musical
  entry behavior, including #73 stable-run offsets. Evidence collection is not
  itself a reason to demote a working fixed-tempo grid.
- Detect only demonstrable later-map conflicts, disclose unverified coverage,
  and retain original audio/renders. No silent retiming or automatic recapture.
- Compare different callback partitions only through supported local authority;
  calibrate stable run offsets on affine evidence without shifting PCM. Persist
  scoped conflict fences and require explicit recovery, including after seeks.
- Preserve both authorities in mixed legacy/new source; require compatible
  boundary evidence for a swell spanning them. Reject evidence-capacity overflow
  atomically and disclose the worst-case recording duration for its limits.
- Gate production implementation on a bounded JUCE/VST3/FL Studio precision
  experiment and owner review of the resulting map-revision/admission policy.

Out of scope: later-map time stretching/repositioning, a tempo editor,
time-signature automation, rhythmic bias (#70), Song/Pattern detection (#1),
UI redesign, new control families and changes to Export's gain law. No
implementation, installation, publication or new host-acceptance claim is made
by this planning change.

Review follow-up: the owner authorized only the bounded diagnostic harness and
host investigation in tasks 1.1–1.2. Those findings are presented in the
[checkpoint packet](../../../docs/validation/2026-11-06-issue-71-checkpoint.md).
Production timing changes and tasks 2 onward remain gated by task 1.3 approval;
the draft PR is not approved for merge or issue closure.

## Capabilities

### New Capabilities

- `song-tempo-mapping`: accepted observation coverage, conversion precision,
  transactional persistence, bounded publication and later-map conflict policy.

### Modified Capabilities

- `transport-swell-playback`: tempo-map-aware live/offline admission, retained
  asset/freeze validity and truthful completed-tail handling.
- `automatic-swell-processing`: stable map/source generation authority and
  explicit source-window conversion across tempo changes.
- `swell-session`: restore verified renders against accepted timing evidence
  rather than the host's current instantaneous BPM.
- `workflow-modes`: enter Perform/Export from compatible sample-aligned material
  even when the current position has a different BPM or incomplete grid.
- `swell-workspace`: one local conversion authority for Snap, inspectors and
  shared timelines, with precise unavailable-state feedback.

## Impact

Affected paths include `SongTime`/`TransportTime`/`HostGrid`, processor capture
admission, capture queue/worker/manifest transactions, candidate/Generate
snapshots, `SwellEngine` render validity, live/offline clocks and publications,
session serializers and shared timeline bindings. Reuse sparse source storage,
immutable assets, runtime session/run ownership and bounded mixing. A
versioned metadata extension is required; plugin IDs, parameters, swell IDs,
PCM coordinates and old assets remain compatible. No JUCE fork or full-map host
API is assumed. Native audio/persistence/component checks and one consolidated
final FL Studio smoke test will distinguish measured support from limitations.
