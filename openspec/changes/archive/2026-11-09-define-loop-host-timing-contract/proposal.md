# Proposal

## Why

[Issue #123](https://github.com/elneilios/ReverbArise/issues/123) is the first delivery gate for [Loop Mode #116](https://github.com/elneilios/ReverbArise/issues/116). The Rack spike measured sample-positioned controls and fractional phrase lengths. Capture and playback need a common contract for which observations establish authority, what invalidates it and how each unavailable state is explained.

## What Changes

- Extend the existing VCV Rack/Host-FX/CLKD probe evidence with the intended wiring, audio-to-marker alignment, long-run timing, Stop, reset, disconnect and reload cases. Publish a reproducible matrix with exact host, module, sample rate and actual callback sizes.
- Define and exercise a per-instance admission contract for the Song timeline, Loop phrase phase, capture edges, musical grid and external performance commands. Give each an independent state and actionable reason.
- Document the verified sample-offset input map and rules for stable fractional periods, discontinuities, re-lock and ambiguous gate release. Obtain an explicit owner go/no-go for each proposed supported configuration.
- Preserve existing absolute Song admission. FL Studio Loop trigger and automation feasibility is tracked in [#127](https://github.com/elneilios/ReverbArise/issues/127).

## Capabilities

### New Capabilities

- `loop-host-admission` defines the evidence, observations and lifecycle required to grant and revoke host timing and command authority for Loop consumers.

### Modified Capabilities

None. The Song timing and transport specifications remain the authority for Song.

## Impact

The deliverable is a measured contract with focused probe/reference checks, a technical guide and dated validation evidence. Production capture, live wet scheduling and editor bindings belong to [#124](https://github.com/elneilios/ReverbArise/issues/124), [#125](https://github.com/elneilios/ReverbArise/issues/125) and [#126](https://github.com/elneilios/ReverbArise/issues/126). Existing plugin identity, saved projects, assets and Song processing are compatibility constraints.
