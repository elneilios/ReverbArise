# Design

## Context

See [proposal.md](proposal.md) for motivation and [the delta](specs/swell-workspace/spec.md) for the observable contract. Current main is `ae47594`. `workspaceNotice()` in `Source/UI/WorkspaceNotice.h` is a pure presentation selector: errors/actionable notices precede phase/recapture/Export notices, then it returns optional debounced IR preparation. Idle Perform currently yields no message. `PluginEditor::refresh()` displays that result and separately retains ExportWorkspace's own notice composition. Existing cases explicitly assert message-free Perform and therefore need updating.

The original clean-idle scenario conflicts with the requested permanent Perform warning. The delta changes that scenario while retaining routine-status suppression, reserved geometry, focus safety, resolution rules and accessibility deduplication. This is a visible UI behaviour change only; audio behaviour stays unchanged.

## Goals / Non-Goals

**Goals:** Reuse the existing selector and approved warning bar; make precedence and restoration deterministic; explain the known risk without implying active detection.

**Non-Goals:** New audio/capture decisions, an activity gate/connector, host state queries, parameter/state changes, notification-queue redesign or a new control/style variant. No implementation in this planning PR.

## Decisions

1. **Lowest-priority mode fallback.** Preserve existing notice selection order. Return applicable IR preparation before the new Perform fallback, even though its tone is information. Use mode (`live` internally / Perform visibly) only; do not condition this general safety explanation on input level, playing state, host brand, enabled swells or `liveEnabled`. Mode-independent error paths remain untouched. Reject severity-only sorting because it would let a permanent warning obscure live progress. Reject changing ordinary `editor.message` into a new notice channel.
2. **Copy and help.** Main text: **“Caution: In Perform, swells can remain audible during DAW playback, even if the source is silent or muted.”** Detail: **“Perform plays prepared swells at saved Song positions, including before notes and through silence. Pattern playback or muting a Playlist track or Channel Rack instrument may not stop them. Bypass the effect or mute its mixer track to stop these swells.”** This follows the owner's plainer wording while using “can remain audible” instead of “always audible”: disabled swells, scheduled gaps and unavailable renders remain silent. It does not claim detected host state. Mixer mute affects the whole path; bypass can restore dry. Do not describe either as a wet-only scheduling control. No uncertain menu path is introduced in the copy.
3. **Existing shared control and roles.** Continue using `Controls/NoticeBar` in its existing header appearance, `NoticeTone::warning`, named warning ink and body typography. Keep `WorkspaceLayout`/WorkflowHeader dimensions, mode description, divider and all controls fixed. Full primary/detail text stays in existing accessible/title/help metadata and context-help routing. Repeated identical refresh uses existing equality/announcement suppression. No dismissal state. No token, exception or library-family changes, so no new-family approval gate is introduced; review the actual integrated message at minimum width before delivery.
4. **Focused verification.** Extend `WorkspaceNoticeCases` to cover all Perform states, every existing override, information-level IR preparation, restoration, mode return/reopen and unchanged Learn/Export notices. Retain `NoticeBarCases` dedup/accessibility/focus checks and existing editor binding/geometry coverage. Use the real header message width at supported logical sizes, not a full-editor-width surrogate. Preserve full accessible detail if overlong text is elided; prefer this short copy fitting without elision. If a wider/second-line design is needed, stop for scoped review rather than shrinking text or inventing tokens.
5. **Document the limitation.** During Apply update workflow/Perform guidance and run the user-guide maintenance workflow for the new user-facing message. Keep issue #1 open and use `Refs #1`; this communicates existing behaviour, not a transport defect fix. Do not fold source prototypes into this PR or replace installed plugins as part of a warning change.

## Risks / Trade-offs

- [Permanent warning competes with actionable information] → Place it below all currently selected notices; test replacement and return explicitly.
- [Overclaiming mute detection or wet-only control] → Use conditional general wording and accurately name explicit host effect/mixer actions.
- [Short header column truncates the explanation] → Validate at actual minimum/larger header widths with embedded Inter; retain full detail via existing help/accessibility roles without geometry changes.
- [Existing tests equate no error with no Perform notice] → Update those expectations narrowly; keep real failure precedence and historical-error filtering assertions.
- [Repeated announcements make persistence intrusive] → Keep copy stable and use existing NoticeBar equality path; no new timer or notification state.

## Migration Plan

No state or asset migration. Apply only after a new explicit owner request, validate the focused native checks/UI standards and provide a prepared native message preview for scoped copy/layout review. Respect the owner's preference to perform computer actions themselves. Record actual host/DPI coverage honestly; avoid repeating the host transport experiments. Rollback removes the fallback and associated documentation/tests without touching any musical data.
