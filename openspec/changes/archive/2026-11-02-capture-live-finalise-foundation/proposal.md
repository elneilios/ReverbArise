# Proposal

## Why

The current plugin captures and schedules wet audio through independent switches, so routine playback can revisit mapped source and there is no authoritative Capture/Live/Finalise lifecycle. Issue [#10](https://github.com/elneilios/ReverbArise/issues/10), the foundation of epic #3, needs protected source capture and predictable transport/mode contracts before the dedicated workspaces and export workflow are built.

## What Changes

- Introduce one saved Capture/Live/Finalise mode with guarded transitions. Fresh instances and legacy projects without a saved mode enter Capture; Recapture is off. Preserve existing creative state and assets.
- **BREAKING behavioral change:** ordinary Capture fills unmapped absolute positions only; mapped audio and mapped silence cannot be replaced by playback, including FL Studio Pattern playback. Replace tolerance-based implicit recapture with explicit, independently armed Recapture existing audio.
- Stage capture passes in immutable temporary assets, publish a validated source revision atomically on DAW Stop, and retain prior snapshots. A failed/aborted pass cannot partially replace committed source.
- Disable manual mode switching during active Capture and Stop commit/validation. A useful successfully committed pass automatically enters Live without waiting for wet renders; empty, failed or incompatible passes remain Capture with truthful status.
- Gate host audio by mode: Capture and Finalise retain dry passthrough without scheduled wet; Live schedules accepted wet under existing routing and never captures. Capture disallows creative editing/internal audition; Finalise locks creative editing while retaining a separate explicit-preview allowance for follow-up work.
- Persist mode and recover safely across loops, seeks, missing timing, save/reopen, duplication and stale/duplicate completion events. Recapture arming and in-flight operations never resume on restore.
- Preserve existing frozen-wet recovery: a verified compatible frozen contribution can qualify for Live if dry source is missing/cleared, without making unavailable source or unfrozen wet valid.
- Add minimal mode selection/status and enforce guards in the model as well as UI. Preserve the existing Live editor and independent automation/IR/gain settings.
- User-requested delivery addition: name the build and displayed product ReverbArise (`ReverbArise.vst3`) while preserving VST3 identifiers and project-state compatibility; do not remove an installed old bundle automatically.

## Capabilities

### New Capabilities

- `workflow-modes`: Authoritative mode state, prerequisites, capture phases, guarded transitions and mode-specific capabilities.

### Modified Capabilities

- `song-timeline-audio`: Additive default capture, explicit pass-atomic recapture, mapped-silence protection and failure-safe Stop publication.
- `automatic-swell-processing`: Capture-mode source ownership, independent Automatic Swells/Recapture policies and generation tied to committed revisions.
- `transport-swell-playback`: Mode-gated wet routing and noncapture behavior, preserving existing absolute-time, offline and retained-IR guarantees.
- `swell-session`: Versioned workflow restore, legacy-to-Capture migration, disarmed recapture and committed-only recovery.
- `swell-workspace`: Minimal mode control/status, disabled-state explanations and model-enforced editing/audition guards without workspace redesign.

## Impact

Expected areas: processor host observation and saved state; CaptureWorker/Session/AssetStore and queue metadata; scoped-recapture compatibility; automatic coordinator and SwellEngine command/playback admission; editor/Settings bindings; capture, state, transport and processor tests. Coordinate source integrity with #8 while retaining the current storage root and managed asset formats wherever possible.

Not included: #11 adaptive Capture layout, #5 full-timeline audition, #6 export/Finalise workspace or mixdown, #4 list sorting, #9 bulk operations, parent #3 viewer-toolbar/snap redesign, Character DSP, portable storage/cleanup, or new host automation parameters. This foundation must not fabricate export controls or change approved IR assets, gain calibration or existing wet-envelope behavior.
