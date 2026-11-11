# Proposal

## Why

Issue #128 completes the Loop workflow with a WAV that can be placed predictably on a musical grid. Accepted Loop audio already supports capture and playback, but Export is explicitly blocked for Loop sources and the Song exporter intentionally clips pre-origin audio, which would discard a Loop's initial lead-in.

## What Changes

- Add finite Loop export with repeat count and a verified musical lead-in grid, preserving the complete included wet lead-in, internal wrap overlaps and final tails.
- Reuse Mix Character and independent Dry/Wet faders for both Preview and WAV, defaulting to Wet Layer (Dry 0%, Wet 100%). As confirmed by the owner during native review, a nonzero Dry level includes captured dry audio once per scheduled pass; this supersedes the original wet-only scope.
- Use accepted capture-local timing and exact Loop extent independently of the current host clock, Phrase signal or Cue availability.
- Reuse existing Export preview, waveform navigation, Drag WAV, Save WAV and atomic file delivery; share wrap composition with Loop playback rather than creating a second overlap algorithm.
- Identify WAV start and Loop Bar 1 and show placement guidance beside Drag WAV.
- Require owner approval of the native Loop Export preview before rollout, followed by sample-level oracles, the full Release suite and a focused FL Studio import/alignment check.
- Preserve Song export behavior, immutable files, captured audio, render caches and the existing offline transparent gain policy. Live output protection is not baked into exported PCM.

## Capabilities

### New Capabilities

- `loop-export`: Finite, grid-aligned Loop composition with independent Dry/Wet levels and consistent preview/WAV delivery, including negative wet lead-in, repeat boundaries and tails.

### Modified Capabilities

- `workflow-modes`: Admit Loop Export using accepted capture-local material without requiring live playback synchronization.

## Impact

- Export planning/mixing and delivery: `SongExport`, `SongMix`, `ExportSource`, `ExportFiles`, `SwellExport`, `ExportVoice`; Loop overlap authority: `LoopWetPlan` and scheduler cycle coordinates.
- Workflow readiness and editor binding: `PluginProcessor`, `PluginEditor`, `ExportWorkspace` and existing shared controls.
- Export preferences/revision descriptors may need additive metadata for repeat/grid settings and the independent Dry/Wet pair; preserve legacy Song state/history. Broader Loop recovery remains #129.
- Extend export, workflow, Loop compositor, asynchronous ownership and native component tests; update living export guidance and host appendices with dated validation evidence. #130 retains integrated host/DPI qualification.
- Work on `codex/128-loop-wav-export` from latest `feature/loop-mode`; PR targets `feature/loop-mode` and remains open for review. No dependency upgrade or main-branch change.
