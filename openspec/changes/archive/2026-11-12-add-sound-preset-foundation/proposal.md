# Proposal

## Why

Learn and Perform currently assign immutable impulse responses directly. A versioned Sound assignment gives later presets a common state and render contract while preserving the exact sound of existing projects.

## What Changes

- Deliver [GitHub issue #190](https://github.com/elneilios/ReverbArise/issues/190) under the approved [Sound Preset System Epic #189](https://github.com/elneilios/ReverbArise/issues/189). Use the six existing bundled responses and managed Custom IR as the first catalogue entries.
- Persist stable preset and definition identities, typed settings and exact resolved IR references for future defaults and each swell. Introduce a neutral Learn Level default for future creation. Keep existing per-swell Level as the effective gain authority.
- Migrate supported projects without reassigning audio or changing existing swell gain, source, arrival, wet timing intent, frozen renders or activity. Keep host identity and automation contracts compatible.
- Commit Sound assignment, fitted shape and matching replacement render through the existing guarded worker and accepted handover from #183. Provide a single accepted-edit boundary for future #185 history.
- This slice excludes the shared picker and other UI work in #191, creative DSP, waveform previews and a history stack.

## Capabilities

### New Capabilities

- `sound-presets`: Versioned Sound catalogue and assignment semantics, future defaults and accepted replacement authority.

### Modified Capabilities

- `swell-session`: Persist the accepted Sound assignment alongside the fitted wet shape and migrate legacy IR-only projects.

## Impact

Creative session codec, managed IR catalogue and asset dependencies, Learn creation snapshots, and the existing selected-swell replacement worker. Compatibility verification covers portable restore, saved projects, render reuse, Audition, Song playback and offline/Export snapshots. The existing JUCE editor and VST3 parameter inventory are outside the change.
