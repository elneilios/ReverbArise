# Design

## Context

Production standards revision 24 is the baseline. The owner approved the P3
sample-data JUCE preview, including its final softer aqua footer. That preview
exercises six surface choices, five text scenes, two logical sizes and
100%/125% scale. It supplies family design approval and layout evidence.
Production consumer and binding checks remain implementation work.

The existing `TextRole` policy supplies fonts, faces and line metrics.
`EllipsisLabel` keeps a single line at its font size. Field feedback measures
wrapped lines through `TextLayout`. `NoticeBar` has an explicitly approved
bounded responsive font policy and silent unchanged refresh.
`TransportReadout` has fixed field geometry. Settings still constructs some
labels with individual font setters, and several multiline labels independently
measure their content.

## Goals / Non-Goals

Complete the text family across visible plugin consumers with one documented
disposition per consumer. Keep ordinary prose wrapping, single-line truncation,
local feedback and footer help distinct. Retain the existing field-family
anatomy and domain-supplied messages. Window chrome, chooser interaction, DSP
and storage operations belong to their existing owners.

## Decisions

| Decision | Rationale and alternatives |
| --- | --- |
| Extend the existing role and label mechanisms with one measured wrapped-text control or policy. | A shared measurement and paint layout prevents clipped final lines and repeated local calculations. Each screen still supplies its own composition. A new general-purpose form framework would expand this family beyond its text responsibilities. |
| Use `EllipsisLabel` for bounded single-line names, paths, counts and values, with full accessible descriptions and existing copy/context access. | Automatically wrapping every row would disturb fixed readouts and compact control geometry. Multiline detail can remain in its existing scroll region. |
| Name the footer's information ink in the shared theme, reusing `hover` at #00C2CC. | The owner approved this exact preview colour. Keeping the semantic alias separate makes its informational purpose clear despite the existing token name. Existing error red, text roles and notice severity retain their own meanings. |
| Preserve the editor's nearest `contextHelp`/tooltip lookup, nested-control fallback and stale-source handling. | The existing route already covers owned auxiliary windows. Apply shared footer presentation to that route and verify it against actual children. A popup help host would change the approved interaction. |
| Reuse field feedback, notices and transport as established specialized controls. | Their existing local-error, readiness, speech and fixed-geometry contracts are already approved. Verify their text integration while completing the remaining labels. |

## Consumer coverage

| Surface | Concrete owners and disposition |
| --- | --- |
| Shared header/footer | `WorkflowHeader`, `HeaderInformation`, `hoverHelp`, `NoticeBar`, `TransportReadout`. Adopt aqua footer presentation and verify existing header, notice and transport policies. |
| Learn | `LearnWorkspace` captions, scope prose, generation reason, heading, status and empty-state text, plus the shared IR message. Migrate remaining screen setup and verify existing field feedback. |
| Perform | `PluginEditor` inspector/list headings, counts, empty states, scope/status labels, readout, and `SwellTimeline` lane/help text. Assign shared roles and overflow policy while retaining guarded gesture help. |
| Export | `ExportWorkspace` heading, mix caption, legend, readiness/status and preview readout, plus shared level and file-tile text. Preserve accepted-mix and pending-message authority. |
| Settings | `PluginSettings` caption/help arrays, regeneration feedback, About build values and Appearance explanations. Replace duplicate setup with roles and explicit wrapping or ellipsis. |
| Diagnostics | `PluginEditor` capture/playback/policy/live details and read-only field captions. Preserve precision, full copy text and throttled updates in existing scroll containers. |
| Storage/Recovery | `StorageInventoryPanel`, `WorkingFolderPanel`, `PortableStoragePanel` headings, metrics, paths, help and statuses. Verify existing shared presentation or migrate remaining local layout. Record the native asset-recovery alert as a retained platform presentation. |
| Hidden compatibility seams | Record the existing D6 and hidden diagnostic labels as retained hidden consumers. Their declarations do not authorize exposing them. |

This roster extends the #107 inventory to the Storage/Recovery surfaces delivered
by closed #8. A family is complete only when every visible text consumer has a
recorded migration, verified shared implementation or named native exception.

## Risks / Trade-offs

- Wrapping can increase page height. Measure the same layout used to paint,
  reflow following content and retain focus reveal and pinned actions.
- Role consolidation can unintentionally change special readouts. Keep the
  existing transport and notice exceptions explicit and run their native cases.
- Repeated text refresh can expose stale descriptions or noisy events. Compare
  accepted text, update visible and accessible content together, and keep
  unchanged refresh silent.
- The Loop/peak-guard work can change message content. Reconcile the current
  implementation base during apply while retaining message authority and
  current guide changes.

## Migration Plan

Record the existing native approval, implement shared text support and migrate
the full roster in the same change. Verify actual mode and auxiliary-window
layouts after adoption. Update standards to revision 25, contracts, inventory,
guide and dated evidence together. A source revert provides rollback without
project-data conversion.
