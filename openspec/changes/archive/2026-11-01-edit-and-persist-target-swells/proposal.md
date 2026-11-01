# Proposal

## Archive reconciliation (2026-11-01)

M3's implementation and 42 acceptance tasks are complete. The successor `automatic-lean-swell-workspace` was synced and archived first. Its accepted automatic processing, contextual/overlapping audition, neutral absent-trim default, wet placement and transition support remain authoritative. M3's delta is reconciled to preserve those contracts while adding its editing, regeneration and persistence requirements and removing the obsolete M2 session-local restriction. Historical M3 defaults and architecture below describe that milestone's original implementation, not a request to restore them. The transport change remains separate and requires its own final host acceptance.

## Why

M2 proves reverse convolution and safe manual audition, but offers only one temporary preview with no musical editing. M3 should make captured songs useful to edit across sessions: choose arrivals, shape several swells, regenerate only affected work, and reopen without losing those choices.

## What Changes

- Add suggested onset markers and explicit confirmation, manual add/move/delete, stable swell identities, names, enable/mute/solo, and per-swell source intervals and IR references.
- Add a shared overview/detail timeline with zoom, page navigation, sample-accurate markers, full wet previews, and nondestructive audible start/end, fade lengths/curves, and gain controls. Editing the envelope does not reconvolve.
- Add Regenerate Selected, Regenerate All, range-armed Recapture & Regenerate, and optional auto-regeneration after committed recapture (off by default). Preserve unrelated swells, cancel obsolete jobs, and show stale/error/progress states per swell.
- Extend protected manual audition to selected swells with their edits. Preserve dry passthrough, capture isolation, native WAV/WavPack import, and the disclosed audition attenuation/level control.
- Persist swell definitions, source dependencies, imported IR assets, immutable full render references, edits, selection, fixed timing, and audition settings. Restore validated renders or rebuild missing renders in the background; never start audition or mapping on reopen. Report missing/mismatched inputs without silently accepting replacements.
- Replace M2's deliberately session-local preview contract with versioned M3 state, retaining legacy v1/v2 capture and tone restoration.

Out of scope: transport-synchronized swell playback and overlap mixing (M4); export, Pack Project, a general relink workflow and cross-PC portability/recovery polish (M5); automatic host transport control, tempo maps, bundled IRs, IR tone/tail shaping, and production UI branding. M3 still uses manual audition and manual FL Studio playback for recapture.

## Capabilities

### New Capabilities

- `swell-editing`: Candidate arrivals, confirmed targets, multiple swell definitions, timeline navigation, nondestructive shape edits, and edited selected-swell audition.
- `swell-regeneration`: Per-swell render requests/dependencies, selected/all regeneration, scoped manual recapture, optional automatic refresh, and independent job/error handling.
- `swell-session`: Compact versioned state, managed IR/render assets, safe same-PC reopening, validation/rebuild, legacy migration and non-destructive asset retention.

### Modified Capabilities

- `reverse-convolution`: Replace transient preview restoration, persist audition level, support per-swell sample-accurate source intervals alongside bar entry, and apply selected-swell envelopes only to audition/visualization while retaining the untrimmed convolution contract.

## Impact

- Extend `Source/Core` with editing/session records and serialization; add `Source/Analysis` and managed IR/render storage alongside the existing capture assets.
- Generalize `PreviewEngine`'s single-result worker into a bounded multi-swell coordinator, reusing `SourceRangeReader`, dependency intersections, WAV/WavPack loading, resampling, FFT rendering, and pinned audition ownership.
- Extend `Source/UI`, plugin controls/state, scoped capture admission, waveform summaries and `AuditionVoice` for edited selection; keep capture before generated output.
- Add headless model/analysis/envelope/coordinator/persistence tests, processor migrations/allocation checks, offscreen interaction/layout coverage, and a separate FL Studio M3 validation guide.
- Reuse the pinned JUCE/libwavpack dependencies; no new external dependency is proposed. Main specs and implementation code remain unchanged during this proposal.
