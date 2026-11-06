# Proposal

## Why

ReverbArise's 1100x850 minimum consumes too much of the owner's 1920x1080 display, and an alpha tester missed Reset beneath the scrolling Perform inspector knobs. Issue [#58](https://github.com/elneilios/ReverbArise/issues/58) needs a complete, comfortable composition at 900x600 and 100% UI scale, with responsive expansion rather than shrinking the current oversized composition.

## What Changes

- Make 900x600 logical pixels the minimum and fresh opening size at 100% UI scale in Learn, Perform and Export. Retain the existing major regions, functionality, visual language and musical workflow.
- Allocate compact columns, bands and navigation deliberately; larger windows add useful waveform and inspector space. Implement owner-approved Option A revision 4: responsive typography, optical logo alignment, a full-width reserved notice row, grouped toolbar wrapping and measured comfort padding. Preserve usable interaction targets.
- Keep the existing selected-swell Reset and reset-options actions permanently visible outside inspector scrolling. Use persistent Timing / Envelope / Sound inspector sections and Capture & Generate / Sound tabs in Learn, with Generate/readiness/manual guidance pinned below both Learn tabs.
- Make all supported controls visible or accessible through obvious labelled entry points, including scroll/focus affordances. Coordinate space and ownership with #80's shared Learn/Perform sound module without implementing new DSP or placeholder controls.
- Separate window sizing from optional proportional UI scale; provide size presets, validated scale choices, persistent presentation preferences and screen-fit recovery. Evaluate 90%, 100% and 125%; do not ship 75% without separate readability/interaction acceptance.
- Establish automated geometry/interaction/persistence coverage, scoped native owner approval and a brief final FL Studio check on the owner's 1920x1080 display. Distinguish actual Windows DPI results from software-scale evidence.

## Capabilities

### New Capabilities

- `editor-presentation`: Minimum/default sizing, responsive expansion, discoverable overflow, independent UI scaling, presentation preference persistence and screen-fit recovery across all workspaces.

### Modified Capabilities

- `swell-workspace`: Replace the 1100x850 opening/minimum contract, allow responsive navigation/header geometry, guarantee persistent Reset access and validate compact musical inspection.
- `ui-design-system`: Extend density governance and native validation to the compact baseline and responsive expansion with the approved responsive text roles while preserving semantic and interaction language.

## Impact

Owner sequencing decision (5 November 2026): this change lands after the other
active UI changes. Fold in accepted/merged #51 Export controls and notice policy,
#68 transport readout and #93 selection/Fit behavior, plus relevant #73 transport
integration, before finalising. Interim layout reviews may continue on the
isolated branch, but final acceptance requires a new integrated native preview
at 900x600. Do not merge other agents' unapproved work or treat interim approval
as acceptance of the final combined implementation.

Planning reference: checkout `84894ee`; owner decisions recorded in this chat on 5 November 2026 supersede #58's earlier wording that 900x600 was only a candidate. #53 is unrelated and is not this change's issue.

Expected implementation touchpoints: `PluginEditor`, `PluginSettings`, `WorkspaceLayout`, `PerformLayout`, `UiGeometry`, `WorkflowHeader`, `LearnWorkspace`, `ExportWorkspace`, existing shared controls, waveform cache scale invalidation, focused native UI fixtures and a small UI-only preference store. No new rendering framework or dependency is proposed. Document approved metrics and review scope in the production guide/standards, update affected living workspace guidance and user-guide currency, and retain dated evidence.

No DSP, capture/render/asset migration, host automation parameter, VST3 identity change, major region rearrangement, separate compact workflow or whole-library migration. The owner approved Option A revision 4 in this chat and subsequently requested implementation. Static mockup approval does not waive native look/feel or final host review. A proposal-only PR must use `Refs #58`, not close the issue.

## Approved review baseline

See `docs/design/compact-ui-900x600/README.md`, revision 4, and its v4 PNGs/render checks. At 900x600: headings 16, navigation 14, body 12, captions/values 11 logical pixels; at 1100x850: 18/16/13/12; at 1400x1000: 21/18/14/13. Named comfort clearances are at least 8 pixels with 12-pixel outer side margins. These replace the initial fixed-type assumptions. The future DSP study reserves capacity only; no Character/Tone/filter/Stretch placeholders ship. #51 / draft PR #92 owns the independent Export mixer; #58 adapts delivered Export controls without absorbing that unmerged audio change.
