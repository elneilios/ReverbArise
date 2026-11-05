# Proposal

## Why

The supplied four-overlapping-swells export reproduces sharp cut-edge artifacts through an echo-rich IR without output clipping. A 1 ms source-edge taper on wet-render inputs removes the excitation while retaining the swell's musical dynamics.

## What Changes

- Condition a private wet-generation source copy with linked 1 ms raised-cosine edge ramps before convolution, with bounded shorter ramps for tiny inputs.
- Preserve capture PCM, dry references, coordinates, complete convolution length, existing gains and zero additional latency.
- Identify newly conditioned renders separately; retain verified saved and frozen renders on restore. Explicit regeneration upgrades eligible older renders without recapture.
- Add native DSP and engine/persistence regression coverage, run Debug/Release gates, and install one local test build with a retained previous bundle.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `reverse-convolution`: wet-generation source conditioning before exact linear convolution.
- `swell-regeneration`: explicit upgrade of saved older processing dependencies while preserving unchanged restored/frozen assets.

## Impact

Worker-side DSP preparation, SwellEngine render dependency matching and explicit generation, focused native tests and living DSP guidance. No callback, UI, capture codec, asset codec, host latency, limiter or tempo-map changes. Initial host testing used a local branch with the accepted export-clock fix. At the owner's subsequent request, the PR isolates the source-edge change on current main, excluding that export fix, issue #71 and the output-protection experiment.
