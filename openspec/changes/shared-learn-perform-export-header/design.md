# Design

## Context

See proposal.md for motivation and scope. Issue #10 and the knob fix are merged.
`PluginEditor` currently constructs a `workflowBox` dropdown and renders mode
status separately in the globals band. `WorkspaceLayout` reserves a 70-pixel
header; the supported default/minimum is 1100x850. `WorkspaceTheme` supplies the
flat palette, typography and name-based Settings glyph. `WorkflowState` owns
guard predicates and `modeName()` is used by state serialization; changing that
function would break the durable-name boundary. The existing native UI fixture
expects a ComboBox and must move to behavior assertions for the new navigation.

Visual reference: issue #3's approved Capture mockup at
https://github.com/elneilios/ReverbArise/issues/3#issuecomment-6577837991 shows
brand/tagline on the left, three joined mode segments on the right and Settings
at the far right. Its old labels are superseded by #16; its textured rendering
does not supersede the approved flat style guide. Follow
`docs/development/ui-development-standards.md` throughout.

## Goals / Non-Goals

**Goals:** one reusable header and guarded selector; clear status at existing
sizes; consistent visible names; explicit component look/feel approval; unchanged
source, audio, saved state and asset behavior.

**Non-Goals:** moving other controls into a new workspace, changing mode admission,
implementing WAV delivery or host transport controls, consolidating all knob/list/
menu/window families now, or treating this extraction as whole-library completion.

## Decisions

### 1. Library primitives, header composition, application binding

Add a focused `SegmentedSelector` and semantic `IconActionButton` under
`Source/UI/Controls/`. They own reusable focus, state painting, input and accessible
feedback; their API accepts labels, selected ID, availability/reasons and action
callbacks, without depending on the processor or engine. Use JUCE event/accessibility
mechanisms rather than a hand-drawn mouse-only control. A shared `WorkflowHeader`
composes the existing Wordmark, tagline, selector, italic mode description and
Settings action. The editor supplies presentation snapshots, binds requests/settings
and hosts the important-message bar in a reserved row beneath the header branding.
The same instance/composition remains in place for every mode and future workspace.

Do not embed processor references in primitives or create three header variants.
A theme-only change would leave interaction configuration duplicated, while
wrapping every unrelated widget now would exceed the agreed incremental scope.

Affected inventory:

| Current control | Ownership after this change |
| --- | --- |
| Temporary workflow ComboBox | Shared SegmentedSelector, old dropdown removed |
| Header Settings gear | Shared semantic IconActionButton, existing window action |
| Wordmark/tagline/mode description | Shared header composition, existing brand resource |
| Important workflow/error messages | Shared NoticeBar beneath branding, reserved space and conditional visibility |
| Global/inspector knobs and numeric fields | Existing components, including merged wheel fix; later workspace migration |
| Recapture, lists, canvas, menus, Settings/Diagnostics content | Existing controls/guards, terminology changes only where mode-related |

Icon role, not component name, chooses the new Settings glyph. Preserve useful
component names as test/accessibility identifiers. Do not rewrite unrelated
name-based theme branches in this issue; record those remaining consumers.

### 2. Model acceptance and one authority for eligibility

The selector emits an intent; `requestWorkflowMode` remains the only transition
entry. Refresh updates selection without callbacks. Rejected/stale requests
refresh from the authoritative snapshot, never leave an optimistic active segment,
and expose the current reason. Active-segment activation and wheel motion are
no-ops. Recording/settlement locks activation, while current selection stays visible.

Reuse `entryError`/capabilities. If presentation needs structured reasons, factor
the existing predicate chain into a typed reason once and have both rejection and
presentation consume it; do not implement a second readiness predicate in the UI.
No changes to the underlying source/frozen/render qualifications are permitted.

Use accessible selected/unavailable state and visible focus. Tab enters/leaves the
group; arrows navigate its labelled segments; Enter/Space activate only eligible
choices. Unavailable reasons remain inspectable through group/item descriptions
and tooltips or equivalent focusable help, rather than depending on a disabled
JUCE button receiving focus. Keyboard navigation does not itself select a mode.
Settings supports the same shared action-button keyboard activation and safe lifetime.

### 3. Presentation names are not persistence keys

Keep `Mode` values, `modeName()` serialization, `workflowMode` XML property,
state versions and restore parsing unchanged. Add one presentation mapping for
Learn/Perform/Export, concise subtitles and mode-related guidance. Review emitted
user-visible errors in processor/editor/Settings as well as the header. Do not
perform global Capture/Live replacement: source capture and live wet are legitimate
functional terms. If typed reasons are introduced, render their human wording
through the display mapping without exposing durable labels to users.

The long-term Export role may be described in documentation, but current status
must say that DAW wet is bypassed and only the existing explicit preview is
available. It must not advertise a working Save WAV or drag tile before #6.

### 4. Responsive composition within existing bands

Reserve room for all three segments and Settings first, then lay out the brand
and italic tagline without distorting the SVG or reducing labels below approved
type roles. Keep the opening size; reserve an 82-pixel header including notice space. Mode labels use
an 18-pixel Semibold role and the description uses a 16-pixel italic role. Resolve
all text through explicit approved Segoe UI faces, with brighter off-white
secondary copy. Logo, tagline, toggles and cog share one primary-row center;
the description sits below the toggles independently, centered vertically with the notice row. Remove the
routine Perform status line; important notices belong beneath the branding.
Reserve a 32-pixel notice row beneath the logo/tagline, left of the mode
description. The header is 82 pixels high in all states; one message does not
move global settings or the workspace. Always draw the full divider at the
header/global-settings boundary below the reserved row, clear of notice paint.
This supersedes the earlier conditional 32-pixel insertion after owner review.
The existing single-highest-priority policy remains; if multiple simultaneous
messages are introduced later, extra rows may grow the header vertically rather
than overlap controls. No message queue or multiple-message feature is added here.
Geometry invariants are keyed by size, regardless of notice presence.
At 1100x850, adjust spacing/brand proportions within the approved composition;
review the real result with the owner instead of inventing another navigation.

Review empty, useful-source-only, ready, recording, delayed validation, failed,
incompatible and frozen-recovery states. Logical geometry tests and scaled native
renders support layout checks but do not establish in-host Windows DPI acceptance.

### 5. Incremental component and integrated approval gates

Checkpoint A presents the selector and Settings action in a small native control
fixture with normal/active/hover/focused/disabled states and an operable example;
it does not require opening FL Studio.
Ask the owner for both look and feel, including keyboard, wheel no-op and unavailable
reason discovery. Resolve requested adjustments before integrating the family.

Checkpoint B presents the integrated header in all modes at minimum and larger
sizes. Ask about brand balance, hierarchy, status, focus and switching feel before
final host acceptance. Record appearance-only approvals separately from untested
interactions. Any material change to accepted component behavior reopens approval.
An untested behavior cannot be checked off merely because its screenshot passed.
After checkpoint-B feedback, the new NoticeBar and typography refinement receive
a representative native component review before production bar binding/wider
rollout. The final integrated review can be consolidated with the brief host smoke
test to keep owner testing small.

### 6. Conditional important-message bar

Add a library-owned NoticeBar with information, warning and error roles. It accepts
text/detail/severity, supplies consistent painting and accessible state, and owns
no workflow, rendering or host logic. It is nonmodal, never steals focus, and has
no arbitrary auto-dismiss timer or notification queue. Repeated refresh of the same
notice is silent; changing a relevant notice updates its accessible description.

An application presentation adapter selects one highest-priority notice from
explicit workflow and snapshot error fields. Source failure/unavailability, failed
actions/IR preparation and existing offline-export warnings take priority over
recording/validation information. Recapture arming and Export's DAW-wet bypass
guidance remain important context. Ordinary idle Perform/clean Learn, progress
counts and routine ready messages do not create a bar. Existing contextual row/
inspector details remain, and no generic parsing of arbitrary status prose or
cross-plugin message bus is introduced. The bar disappears when its underlying
condition resolves; it does not dismiss an unresolved error merely to look clean.

## Risks / Trade-offs

- Durable/display name confusion → explicit XML round-trip assertions and unchanged
  identity checks; no broad replacements or archived-record edits.
- Guard drift or async readiness changes → one predicate authority, request-time
  revalidation and tests for failure/stale availability and retained active state.
- Disabled controls conceal reasons → test accessible descriptions/help separately
  from pointer activation and disabled state.
- Minimum-width crowding → preserve band budget, test geometry and ask at checkpoint B.
- Library scope expands into a rewrite → migrate the inventory above only; record
  remaining families honestly for #11 and subsequent workspace work.

## Migration Plan

Automated component, state, processor and persistence tests provide the primary
evidence, including failure/race and audio invariants. Early owner checkpoints
review native examples; do not turn each component into a new FL Studio checklist.
Consolidate host UAT into one brief final smoke test. Request additional host
checks only for observed failures or specific unresolved host behavior, not to
repeat passing automated edge cases or prior acceptance.

Add presentation/primitive tests and the checkpoint-A fixture first. After owner
approval, integrate the shared header, remove dropdown-specific paths/tests, update
visible terminology and living product/design/workflow guidance, then run focused
state/UI/processor checks. Obtain checkpoint B and one short FL Studio smoke run:
control feel/focus, a Learn → Perform → Export/return journey, and save/reopen.
Automate detailed guard, routing, protection and compatibility cases rather than
asking the owner to repeat them. Host DPI checks are requested only where a
layout change creates a concrete unresolved risk; otherwise record their actual
coverage without inventing a pass. Rebuild the ReverbArise VST3 for the smoke test.

No saved-data migration is needed. A rollback restores the prior header while
reading the same mode/source/assets. Keep #11's already committed proposal on its
own branch; reconcile its header dependency when resuming rather than duplicating
the selector. Preserve dated validation history; append new evidence for this change.
