# Proposal

## Why

The owner's 3 November header brief and original screenshot request clearer mode
recognition and a cohesive lower information row while retaining the approved
composition. The current header uses text-only mode segments and an italic mode
description; it already has reusable navigation, notices and native review fixtures.

## What Changes

- Add semantic JUCE vector icons before Learn, Perform and Export: vertical
  waveform bars, a rising reverse-swell curve with three outlined handles, and an
  audio document/outward-arrow motif.
- Prepare the canonical SVG resources in [assets/icons](assets/icons/README.md)
  now, with no production embedding, C++ helper or shared-component edits.
- Keep existing segment rectangles, border/focus/clipping behaviour, model-owned
  selection, wordmark, tagline, Settings and workspace geometry.
- Give the mode description a 2 px aqua vertical accent, an 8–10 px text gap and
  regular Inter text matching the contextual-message body size and #C7CDD4 ink.
- Preserve contextual severity markers/icons and reserved left-column space;
  scope the typography/ink treatment to header consumers rather than changing
  global theme tokens.
- Stage independent icon preparation before overlapping integration. Preserve the
  Learn agent's focus/border clipping corrections and obtain native component
  approval before production rollout.

Copy: retain Learn's current description; use “Shape and place reverse swells with
precision” for Perform; retain “Preview your prepared swells” for Export while WAV
delivery remains unimplemented. The suggested “Render your swells to audio” is
reference copy, not authorization to expose future functionality.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `swell-workspace`: revise the shared header's italic description requirement and
  add semantic mode icons and unified, non-colliding information-row presentation.

## Impact

Future implementation affects shared UI controls, `WorkflowHeader`, presentation
copy, focused component/binding tests, the existing native review fixture and
living UI guidance. No DSP, workflow authority, persistent identities, new audio
capability, third-party asset/library or main-workspace redesign is proposed.

Planning is isolated on `codex/header-mode-icons`, based on main `09c566b`.
`bound-waveform-paint-work` remains untouched with all seven tasks unchecked.
The concurrent Learn agent's files are not edited by this proposal.

## Preparation status

The three original SVGs share a 16-unit canvas, 1.6-unit rounded strokes and
explicit monochrome ink. Browser asset inspection covers 14/16 px plus scaled
examples; the static review sheet does not establish native JUCE, interaction or
Windows DPI acceptance. Exact visual specifications, file-level integration
dependencies and the validation checklist are maintained in [design.md](design.md).
Implementation remains blocked on the owner's later instruction and Agent 1's
published navigation/focus-clipping baseline. No merge or cherry-pick is performed
as part of preparation.
