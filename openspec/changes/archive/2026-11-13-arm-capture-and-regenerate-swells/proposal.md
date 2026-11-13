# Proposal

## Why

Learn can start Song capture when an instance receives an already-playing transport. Returning to Learn after reviewing swells can also leave the musician unable to rebuild from the accepted recording because existing edits and previous deletions influence explicit generation.

## What Changes

- Put one **Arm** control above the Song and Loop settings in Learn. Both modes start disarmed and require an explicit request for each take, including during FL Studio Pattern playback. Keep mode-specific timing and separate Recapture Audio permission.
- Use **Generate Swells** for the first generation and **Regenerate Swells** when revising an arrangement or trying again after deletion. An explicit successful rebuild uses the current markers and sound defaults, keeps only frozen swells from the prior set, and ignores previous deletion suppression.
- Give the Learn action a taller shared primary button with a centred swell icon and full label. Keep one action in the same place on both Learn tabs, with one helper line below it and the rebuild explanation in footer hover help. Review the presentation in a native preview before adoption. [Issue #178](https://github.com/elneilios/ReverbArise/issues/178)
- Align the visible Learn Settings tabs and Sound controls with the capture sidebar gutter using the existing focus margins. [Issue #177](https://github.com/elneilios/ReverbArise/issues/177)

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `workflow-modes` defines shared capture authorization and the distinct explicit and Stop generation intents.
- `song-timeline-audio` requires Arm for additive and replacement Song takes.
- `loop-phrase-capture` binds the existing Loop take contract to the shared Learn control.
- `automatic-swell-processing` defines frozen-only explicit rebuilding and preserves the conservative automatic Stop policy.
- `swell-session` scopes provenance-based protection to automatic reconciliation and specifies frozen retention in explicit rebuilds.
- `swell-workspace` defines shared Arm placement and the Generate/Regenerate action and its feedback.

## Impact

Affected paths include the processor and Learn capture bindings, candidate generation and render publication, native Learn layout, shared command-button presentation, focused regression fixtures and living guidance. Existing project and plug-in identities remain compatible. Song users who previously relied on DAW Play to begin capture must arm first.
