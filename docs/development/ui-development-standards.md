# UI development standards

## Context help delivery — issue #46 review

The editor routes existing `TooltipClient` or explicit `contextHelp` metadata
from hovered controls and graph regions into one bottom `EllipsisLabel`, using
the established helper text role and feedback height. Nested children inherit
the nearest available message; owned Settings/Diagnostics windows participate.
Keep help metadata on the control, with workspace-specific explanatory prose;
do not introduce popup tooltip hosts or draw hover text over waveform content.
Interactive title/name is only a fallback. Full text is also the footer's
accessible description. Hover must not publish state, rebuild layouts or start
jobs. The owner approved the actual native presentation on 4 November 2026.

Use these standards when implementing or refactoring ReverbArise UI. Consolidate
controls as the planned work touches them; a separate whole-plugin migration is
not required before the Learn / Perform / Export work can proceed. Existing UI
is not considered compliant merely because it inherits the shared LookAndFeel.

## Shared controls and ownership

Production standards revision 20 uses labeled switches in Learn and Settings
and inline checkboxes for Enabled, Solo and Freeze. Use `Switch` and the
checkbox form of `BooleanToggle` for their shared pointer, focus and activation
policy. Give the indicator and label one clickable area and preserve the
binding's accepted value and eligibility.

Production standards revision 19 adopts the approved P5 dial and track states.
Use the shared dial or track surface and the same corner-bracket geometry as
command controls. Configure wheel steps in the control's own units. Ctrl+wheel
uses the fine step, and neighboring numeric boxes retain the P4 stepper policy.
The optional home detent applies to raw dial drag candidates. Level is the
first consumer, with its home at 0 dB and a ±0.3 dB band. See the
[C5 contract](ui-control-contracts.md#c5--knobs-and-sliders) and
[P5 evidence](../validation/2026-11-09-issue-107-knobs-sliders.md).

Production standards revision 18 adopts the owner-approved P4 field family.
Use `FieldTextEditor` for editable and read-only text and `FieldNumericInput`
for numeric value boxes. Keep each binding's commit, cancellation, range and
precision rules. Route local help and validation through `FieldFeedbackLabel`,
with error text and border independent of keyboard or accessibility focus.
Captions belong above the editable field or group. When a value box has
steppers, wheel input over the box or arrows makes one configured step. Use the
[C4 contract](ui-control-contracts.md#c4--numeric-and-text-fields) and
[P4 review evidence](../validation/2026-11-09-issue-107-numeric-text-fields.md)
for the approved variants and subtype behavior.

Issue #108, production standards revision 15. Level uses the standard `KnobValue`
appearance and drag response with one -12 to +12 dB edit range. The normal
field shows an older value below -12 dB. A primary press on the dial brings
that value to -12 dB and starts any subsequent drag at that limit. The
accessibility slider reports the bounded position and edit limits, with the
actual saved gain in its text readback and help. The Level binding uses
`onDialPress` for first touch and `onAbsolute` for numeric accessibility writes.
`--test-level` checks native input, neutral paint, layout and project reopen.
The owner approved this interaction on 2026-11-07.

Production standards revision 13 adopts the owner's
[focus visibility policy](ui-control-contracts.md#focus-visibility-policy).
Shared command buttons show their aqua brackets for keyboard/accessibility focus,
and hide them after pointer clicks without discarding actual focus or keyboard
activation. Hover and held feedback are unchanged. Other families adopt this
policy only in their own design-system jobs, preserving their separate editing,
caret and selection cues. Reuse the shared input observer where appropriate;
never disable focus or keyboard operation as a way to hide its decoration.

Production standards revision 11 adopts the owner-approved
[control inventory and family roadmap](../design/ui-control-inventory.md) and
[shared control contracts](ui-control-contracts.md), reconciled with merged #80.
Use them to identify actual ownership, applicable state/interaction matrices,
field help/validation composition and bounded child delivery. This is contract
governance, not certification of legacy widgets or approval of new native variants.
[Audit evidence](../validation/2026-11-06-issue-107-control-audit.md) records scope
and validation; #108 Level and #8 recovery/engine work remain independent.

Production standards revision 12 records the owner-approved #107 command family
revision 4. `ActionButton::paintSurface`, icon actions, latches and the generic
theme fallback delegate to one semantic resolver/painter. Focus uses four aqua
corner brackets, 2-pixel stroke, with a 3-pixel gap outside the body border;
component bounds contain all ink. Body border/background own hover and held-press
feedback. Destructive hover/press stays red; neutral icons remain off-white.
Primary/on-latch fills keep contrasting dark content. Disabled body state wins
over mouse emphasis; focus remains independent. Reuse explicit variants and
`CommandButton` invocation hooks, never names or labels to select appearance.
The 20 scoped legacy commands use ActionButton. Revision 18 also adopts neutral
ActionButtons for Rename OK and Cancel. FileDragTile and knobs retain their painters.
[Button evidence](../validation/2026-11-06-issue-107-buttons.md) records the family
approval, 85/85 native checks and owner acceptance of the integrated IR alignment correction.

Issue #80's owner-approved `Controls/ConvolutionIrEditor` family composes
`IrSelector`, `IconActionButton` and `EllipsisLabel` using existing responsive
geometry and text roles. It owns catalogue presentation, captured menu choices,
deliberate selection, notification-free accepted-state refresh, focus, accessible
field status and footer-help metadata. It has no scope-message property or child.
Hosting screens own context explanations such as future-default versus selected-
swell scope; these belong with their Sound section, outside the shared field.
Overflow retains full help/description;
requested feedback never becomes an optimistic accepted assignment.
`ConvolutionIrBinding` outside the control library owns two explicit target types:
future default (session/pass) and selected swell (session/ID/input revision).
One dispatch path calls the existing engine/importer validation authority. Its
file chooser uses the same captured weak-lifetime completion tested without an
OS dialog; a newer choice/import invalidates an older unsubmitted file result.
Engine pass/mode/freeze/revision guards still decide publication. These files
are consumed by Learn Sound and Perform Sound through the same command path;
the Diagnostics default-import entry also uses that binding. The owner approved
the revised family and integrated composition, including corrected hover and
collapsed empty feedback spacing. Host acceptance remains separate.
Existing visual tokens are reused (production standards revision 10).
The focused test/review entry points are `--test-ir-editor` and
`--review-ir-editor` on `preverb_swell_ui_test`.

The #80 owner requested a closer field/feedback relationship and red failed
states. The revised candidate treats label, input/action row and supporting text
as one field: `spacing::tight` below the input, at most two ordinary helper-font
message lines. Idle accepted/ready messages that repeat the selected value are
omitted. Empty feedback occupies no space; hosts use the width-dependent preferred
height to reflow adjacent fields when messages appear, wrap or clear. Perform Level
follows the field with the existing content padding; Reset stays fixed outside the
scrolling fields. Hosting-screen scope notes are separate from field feedback. Error text
uses the existing `danger` semantic ink, includes a recovery action, and retains
full accessible diagnostic detail. Labels/messages stay readable when the input
is disabled; colour supplements the message. Bottom hover help remains available.
This is a scoped owner-approved IR-family policy, informed by
[Fluent Field guidance](https://fluent2.microsoft.design/components/web/react/core/field/usage).
It does not require a general field framework or a whole-plugin migration.

Shared NoticeBar updates visible severity, accessible title/description/help and
standard accessibility change events without explicitly synthesizing speech.
Do not call postAnnouncement for notice refreshes: on Windows JUCE can invoke
SAPI whenever an accessibility client (including automation) is active. Keep
assistive access and OS accessibility settings intact. This interaction-policy
correction is owner-approved in the issue #51 follow-up; it adds no visual variant.

Issue #51's `Controls/ExportLevelsControl.h` review candidate composes the existing
`DeliberateSelector`, `SliderControl` and `NumericInputControl` policies. It emits
one pair edit for a preset, preserves the untouched level for fader edits, derives
Custom from the exact canonical pair and refreshes without notifications. Its
binding owns session/pass validation and accepted-state rejection/reconciliation.
The owner approved minimum and larger native appearance/interaction and accepted
the generated preset comparison for integration on 2026-11-05, reserving final
listening acceptance for FL Studio. Production Export now consumes this composition
with session/pass-pinned pair edits. No new styling token or variant is introduced.
The dedicated native
fixture is `--review-export-levels` on `preverb_swell_ui_test`.

The [production guide](../design/ReverbArise_UI_Layout_and_Style_Guide.md) is the
numerical authority (production standards revision 13; scoped knob focus review below). Its responsive composition section
takes precedence over the retained historical mockup suggestions. Current code
and scoped native approval establish measurements; do not sample a generated
mockup to invent a production token. The [#37 evidence](../validation/2026-11-04-issue-37-ui-standards.md)
lists the integration baseline, component ownership and remaining legacy consumers.

- Use a repository-owned control library for all new or refactored application
  controls: knobs, sliders, numeric/text entries, buttons and icon buttons,
  toggles, dropdowns, menus, lists, tabs, scroll containers and auxiliary windows.
  Settings and Diagnostics follow the same rules as the main editor.
- Build on JUCE and the existing `WorkspaceTheme`, `KnobValue`, viewport and
  chooser-invocation mechanisms. Put reusable primitives/composites under
  `Source/UI/Controls/` as they are extracted. Avoid duplicate implementations,
  aliases with no shared behavior, and a universal control with many mode flags.
- The library owns styling, reusable interaction defaults, focus/accessibility,
  and edit-lifecycle plumbing. Screen code owns layout and action/value bindings.
  Parameter bindings own musical units, ranges, conversion, precision, validation
  and model/session identity. The library must not depend on the audio processor
  or perform capture, rendering, asset I/O or host transport actions.
- Waveform handles, the timeline, Overview and row interactions are specialized
  reusable controls, not exceptions that can accumulate screen-local behavior.
  Keep their timing/hit geometry and domain validation in their existing focused
  components; reuse library primitives and policies where applicable rather than
  rebuilding the waveform editor as generic sliders.
- Native OS file choosers and window chrome may remain native. Their invocation,
  ownership and safe completion paths must still be shared and tested.

## Consistent interaction

- Define common defaults once for each control family: wheel direction/travel,
  normal and fine drag, keyboard operation, focus, enabled/read-only states,
  hover/pressed feedback, tooltips and accessible names. Global and selected-swell
  knobs must use the same interaction implementation, with parameter-specific
  ranges and legitimate numerical steps supplied by bindings.
- Document intentional exceptions as named policies, with a reason and a test.
  For example, an IR dropdown may disable wheel selection to prevent accidental
  reassignment. Do not scatter `setScrollWheelEnabled`, drag-sensitivity or
  fine-modifier choices throughout screen constructors.
- Make knob position, value text, canvas and accepted model state agree during
  gestures. Preserve active typed text during refresh. Define Return, Escape,
  focus-loss and invalid-input behavior explicitly; display rounding must not
  reduce saved precision. Wheel input outside a knob must retain normal scrolling.
- Pin edits and asynchronous actions to the intended object/session. Cancel or
  reject stale selection, deletion, timing, workflow or restore completions. Model
  guards remain authoritative beneath disabled widgets. Programmatic refresh
  must not generate edits, dirty notifications, audio or background jobs.
- Use semantic roles for icons, emphasis and destructive actions. Appearance must
  not depend on incidental component names or visible button text. Keep stable
  component names where they support accessibility and existing tests.
- Resolve fonts through the shared theme policy, including custom Graphics
  painting: `fontFor(component, height, flags)` uses embedded Inter Regular,
  Bold and Italic; `navigationFont()` selects the actual Inter SemiBold face.
  `interTypeface(InterFace)` owns cached explicit face pointers. Label refresh
  preserves named Medium/SemiBold rather than reducing them to a binary bold flag.
  Learn's reviewed control-label refinement uses the pinned static Inter Medium
  (500) face; header/navigation faces stay unchanged. Invocation sliders can set
  explicit value typography through `setValueFont`, retained on child recreation.
  Do not use installed-family lookup, mix in system bold/italic, rely on an
  implicit default font or change JUCE's process-wide default LookAndFeel.
  Text-entry composites must refresh their font when their inherited LookAndFeel
  changes. Verify resolved typeface names/styles and embedded resource coverage.

## Incremental migration

`Controls/TransportReadout` owns the owner-approved #68 read-only Perform footer:
independent value slots, separators, typography, bounded overflow and accessible
help. Editor code binds existing diagnostics and musical timing without host
actions or persistence changes. Preserve `hostInfo` as its component name. The
native family approval covers spacing, readability and stability as values change;
its [sizing policy](transport-readout.md) and component/editor tests travel together.
The retired editor font setup reduces its existing font-construction debt count
by one; the inventory is manually narrowed for that exact removal, not regenerated.

Settings About reuses the existing tab container, sectionHeading/controlLabel/helper
text roles, EllipsisLabel, ActionButton and comfortable geometry. It adds no style
token or control-family variant and leaves the five existing pages in place.
Build values come from generated metadata; the copy action runs on the UI thread
without session edits, capture or rendering. Native tests verify full copied
identity, accessibility/bounds and session/capture/render invariance.
Copy uses the existing filled ActionButton variant for clear enabled/hover feedback;
the existing helper role confirms clipboard completion without disabling the action.

Issue #46's native revision-4 compact composition has owner approval. Its
`PerformLayout::performGeometry` profile is scoped to Perform and tested by the
native composition matrix; standards revision 3 records the accepted values.
The production editor consumes this profile following #50's merge. The
`LatchingActionButton` reuses ActionButton's unlatched drawing, invocation,
keyboard and focus mechanism and adds accepted pressed-state ink. Its native
look/feel checkpoint received owner approval on 4 November 2026. Perform owns
requested/effective Snap, transient Ctrl bypass and its guarded grid gestures.
`DeliberateSelector` owns the accepted no-wheel invocation policy shared by IR
and grid selectors. `KnobValue` retains exact-entry/gesture ownership while using
the approved 14-Medium/24 caption and 28-high value roles. Long captions ellipsize
with full tooltips; they do not shrink the font. Unaffected Settings/Diagnostics
consumers retain the same shared lifecycle and domain-specific steps.

The #46 knob audit corrections separate accepted dial painting from JUCE's raw
pointer accumulator, add explicit rotary keyboard/focus and domain accessibility,
and keep fixed wheel/arrow increments in the binding. They reuse existing focus
ink/geometry and text roles; no new density/profile token is introduced. The
changed knob interaction received focused native family acceptance on 4 November
2026; prior compact layout and Snap-button acceptance remain valid. `EllipsisLabel` preserves a
single line's approved font metrics for overflowing inspector status/IR text.
See [value/unit policy and guards](perform-navigation.md) and
[audit/remediation evidence](../validation/2026-11-04-issue-46-knob-audit.md).

KnobValue may compose compact readout text with an optional exact-edit string.
The binding owns formatting; the cell keeps a distinct typing baseline so focus
and unchanged Return do not publish a rounded value. Existing consumers omit the
optional string and retain their current behavior. The grouped Perform compact
readout usage is staged in the native clarity checkpoint, before wider rollout.

The owner requested the revision-4 KnobValue focus variant: four open corner
brackets, using existing aqua and 2-pixel stroke, with legs at 20% of the contained
focus box side. It changes only the dial focus painter; text fields and other
library families retain their own focus treatment. The owner approved the actual
native variant and Level wheel/arrow steps of 1 dB normally and 0.1 dB with Shift.
The subsequent double-click/context-menu reset interaction received focused native
owner approval on 4 November 2026. KnobValue owns menu lifetime and invocation tickets; bindings own
default/initial semantics, canonical validation and identity guards. See the
[reset policy](perform-navigation.md#knob-reset).

`Controls/TimelineZoomControls.h` composes the shared `ActionButton` and
`InvocationSlider` mechanisms for horizontal Zoom/Fit intents. The workspace owns
accepted bounds, logarithmic range, Fit and stale-session guards. The independent
`MusicalTimelineRuler` painter reuses the existing timing/coordinate authorities.
See [interface and review scope](shared-timeline-controls.md); this library slice
contains no workspace-specific Fit, gesture, transport or serialization behavior.

`Controls/SliderControl.h` is the reusable wheel-enabled slider/value family.
It retains `InvocationSlider` ownership guards and JUCE range, interval,
formatting and notification behavior. Its `NumericInputControl` is a reusable
editable Label with named, keyboard-accessible increase/decrease buttons and
binding callbacks `onBegin`/`onStep`; a binding owns validation and formatting.
Use the same model path for wheel, buttons and typed values. Do not duplicate
parameter mutation in the visual component. Learn is the first integration;
the numeric up/down component received owner look/feel approval on 2026-11-03.
Preserve that accepted component; revised families still require approval before
wider rollout.

For each design-system family, inventory its consumers across the whole plugin,
including Learn, Perform, Export, Settings, Diagnostics and dialogs. Use the
approved shared control or variant for every applicable consumer and remove the
superseded setup. Record existing shared consumers and any owner-approved
specialized variants in the inventory. A family may land in reviewable slices
and stays open until the inventory accounts for every consumer.

Local field feedback is part of the owning family's rollout wherever its
controls have a position for help or validation. Use the shared C3 presentation
and preserve the binding's existing edit and validation behavior. The family
inventory records which fields have a feedback position and what belongs there.

Keep extraction and intentional behavior changes reviewable separately. Preserve
approved layout, audio behavior, source protection, saved values and plugin/state/
asset identities. Do not add host automation parameters, normalize every domain
to the same numerical step, or redesign unrelated workspaces as a side effect.
If extraction reveals work outside the agreed issue, surface it before expanding
scope. OpenSpec plans for the upcoming UI work should include the affected
inventory, reuse/migration tasks and acceptance gates.

## Verification and review

Automated verification is the default and primary evidence. Keep owner testing
small: use lightweight native previews for component feedback as work progresses,
then consolidate FL Studio UAT into one brief final smoke test. Do not require
opening the DAW for each component checkpoint. The final smoke test should target
host-specific feel/integration and a representative user journey; automate the
detailed state, audio, persistence and failure cases wherever practical. Do not
repeat passing automated edge cases or previous owner acceptance without a
specific reason. Ask for additional host checks only after a failure or when
concrete host behavior remains unresolved. Record untested host/DPI coverage
explicitly rather than expanding the checklist by default or inventing a pass.

When asking for component/UI approval, open the prepared native preview directly
for the owner if available. Do not require them to locate an executable, paste a
command or run a launcher. Reuse an already open review window rather than
spawning duplicates; leave it open while they review. Screenshots remain useful
supplementary evidence. If a review executable is open, preserve that process and
defer relinking its binary until the owner has finished.

A native harness borrowing the production editor's components must have one
refresh owner. After construction, call `stopAutomaticRefreshForReview()` on that
hidden editor and drive its public `refresh()` from the harness; normal plugin
construction retains automatic polling. Apply specimen overrides before yielding
to the event loop and retain callback ownership when rebinding borrowed controls.
Keep layout/font setup in actual resize/layout changes rather than idle polling.
Verify live timer/event-loop stability as well as static rasters; the
[issue #46 follow-up](../validation/2026-11-04-issue-46-perform-composition.md)
records the owner-approved review seam and a competing-timer regression test.

- Add focused tests at the shared component boundary using actual wheel, drag,
  keyboard, text-entry and focus events where practical. Exercise limits,
  invalid/disabled states, cancellation and refresh; avoid tests that only call a
  value callback or restate implementation details.
- Add binding/integration checks for model/value synchronization, stale edits,
  unrelated-swell/source protection and persistence where affected. Test menu
  anchors, scroll routing and window/callback lifetime when changing those areas.
- Maintain a small native control gallery or equivalent reusable test fixture
  for the families being migrated, including normal, focused, disabled and error
  states. Reuse it across controls rather than creating unrelated demo apps.
- Review actual native screenshots at the 1100x850 minimum and relevant larger
  sizes. Ask the owner at early representative and integrated visual checkpoints
  before continuing dependent visual work. Follow the approved style guide and
  issue mockups; shared components are not permission to change the design.
- Obtain explicit owner acceptance for each new or materially revised library
  control family as work progresses, before rolling it out to further consumers.
  Present a representative native/host example and ask for both **look** (size,
  contrast, spacing, labels and state feedback) and **feel** (wheel/drag/fine
  adjustment, typing, keyboard/focus, scrolling and disabled behavior). Keep these
  reviews small and timely; do not defer all component approval to the final
  plugin review. Record approved, adjust or untested outcomes separately. A
  screenshot approves appearance only, not untried interaction. Revisit approval
  when a later change materially alters an accepted component's behavior.
- Run checks appropriate to the affected layers and report their scope. Record
  FL Studio wheel/drag/focus feel, host scaling and acceptance separately from
  native/offscreen tests. Do not claim host or DPI acceptance from a raster.

PRs touching UI should state the component families reused/extracted, deliberate
policy exceptions, remaining migration and validation gaps, and visual approval
status. Reviewers should flag duplicated interaction setup as a maintenance issue,
not defer it automatically because the screen already looks correct.

## Production text roles and shared defaults

Shared namespace functions in `m3::ui` extend the existing `WorkspaceTheme.h`
authority. `textStyle(TextRole)`
returns height, embedded face, semantic ink and line-box height;
`textFont(TextRole)` resolves the cached face. `applyTextRole(Label&, role,
justification)` and `applyTextRole(Graphics&, role)` keep label/custom paint
consistent. Roles cover section headings, control labels, helpers, rulers, empty
heading/body, tagline, header body and control values. Screen layout still owns
actual rectangles; applying a role must not resize, focus or bind a component.
Default Label justification is centred-left; helpers explicitly use top-left.
Do not reinterpret line-box height as a font size or change numerical precision.

`controls::IrSelector` builds on the existing invocation-aware combo box and
owns the no-wheel IR selection default. Learn and selected-swell IR use it;
managed identities, asynchronous requests and pass/session validation remain in
their bindings. `SliderControl` already owns horizontal style, wheel availability,
value typography and numeric bounds; Learn supplies only range/interval, suffix,
precision and model callbacks. Unrelated settings selectors and legacy knob/slider
families retain their current semantics until planned migration. See the evidence
inventory rather than claiming all consumers have moved.

## Styling guard and exceptions

Run `powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-ui-standards.ps1`
from the repository root. Command fixtures are
`powershell -NoProfile -ExecutionPolicy Bypass -File tests/ui_standards_lint_test.ps1`
(also supported with `pwsh -NoProfile -File`). Both are registered with CTest.
`-ListFindings` is a read-only diagnostic report, not a passing enforcement run.

The targeted scan covers C++ headers/sources under `Source/UI` and `Source/Plugin`.
Rules reject raw colours/factories, direct/default/system font construction,
literal typography and styling setters, duplicated interaction defaults, and
changed/new numeric token definitions in the existing theme/geometry authorities.
It is not a complete C++ parser: ordinary placement/domain arithmetic is allowed,
and custom glyph/path geometry still needs review. Comments are masked with line
positions preserved; escaped quoted strings are retained. C++ raw string parsing,
macro expansion and semantic alias analysis are outside this lightweight guard.
Native/component tests and review remain necessary.

`scripts/ui-standards-baseline.json` bounds approved definitions and retained debt
by exact path, rule, normalised occurrence and count. Every entry includes a
stable ID, scope, rationale and review reference. A duplicate new occurrence,
changed token or stale exception fails. There is no file-wide legacy exemption
or automatic baseline-update mode. Remove exceptions when their debt is removed;
do not regenerate the file to make a failure disappear.

For example, if a future Perform control needs a new named spacing role:

1. Its ordinary OpenSpec design cites the current standards revision, the shared
   control and why comfortable spacing cannot satisfy the actual dense content.
2. Prepare the named role/profile proposal and representative native fixture;
   present owner review of bounds, readability, focus and feel where affected.
3. Record approval before rollout. Update the token, guide revision/change record,
   affected consumers and component tests together. If the guard flags the approved
   definition, add the exact scoped entry referencing that approval.
4. Run the guard, command fixtures and affected native tests. Record Windows/host
   gaps separately and remove obsolete exceptions. A screenshot alone cannot
   approve interaction or actual Windows scaling.

New tokens, visual variants and one-off exceptions follow this same review path.
Existing owner-approved families retain acceptance when extraction demonstrably
preserves look and behaviour. Do not reopen passing owner checks without a concrete
change. Standards revisions are independent of product and saved-state versions.
Future UI OpenSpec artifacts must identify standards revision, affected control
inventory/reuse, justified exceptions and review checkpoints; generated OpenSpec
skills and configuration remain unchanged.

## Reusable native visual checklist

Use the existing `preverb_swell_ui_test` fixtures: `--review-controls` for family
states, `--review-messages` for all modes/severities and long notices,
`--review-learn-empty` for real empty composition, `--review-learn` for mapped
source and `--review-workspace` for integrated navigation/Settings. Agents launch
the prepared fixture directly when owner review is needed and preserve the window
through review. Set `PREVERB_UI_M3_OUTPUT` during normal native UI tests to retain
the same fixture renders; these are JUCE software renders, not Windows DPI passes.

At 1100x850, 1400x900, 1400x1000 and 1600x1000, plus width-breakpoint boundaries:

- Check full headings/labels and helper lines, numeric value/unit/stepper bounds,
  long IR names, empty prompt, reserved notice row and unclipped dividers.
- Exercise generation on/off, all header modes and severities, selected/hovered
  adjacent borders, focused/disabled/error states and keyboard order/activation.
- Confirm complete focus strokes remain inside bounds, text fits its line boxes,
  controls remain reachable and waveform space remains dominant.
- Preserve active typed text on refresh, validate Return/Escape/focus loss, and
  verify shared wheel/stepper input plus silent programmatic display. Bindings
  must reject stale intents without changing another object/session.

Automated geometric comparisons use logical bounds, actual embedded font metrics
and small floating-point tolerances (0.01 logical pixels for metrics); renders
exercise 1/1.25/1.5/2 software scales without whole-image golden gates. Real Windows
100/125/150/200% DPI and monitor transitions are recorded separately as passed,
failed or untested, with actual display/host context. Do not infer them from a
scaled image. Retain prior owner acceptance for unchanged families; consolidate
any remaining concrete host integration/feel checks into one brief FL Studio
smoke rather than repeating passing automated edge cases.

## Initial control-library API (issue #16)

The first families live under `Source/UI/Controls/`; they were approved in a
native review fixture and are composed into the shared production WorkflowHeader:

- `SegmentedSelector::display(choices, acceptedId)` silently supplies labels,
  help, availability and unavailable reasons. `onRequest(id)` emits an intent;
  the binding must revalidate it against the model and call `display` with the
  accepted state. Pointer/Enter/Space activation never toggles optimistically.
  Arrow/Home/End keys inspect choices without selecting them; Tab enters/leaves
  the group. The group exposes all reasons through accessibility descriptions,
  including disabled choices. Wheel and repeated active selection are no-ops.
  Segments share square internal joins with one border; only the group's outer
  left/right ends are rounded, following owner checkpoint-A feedback.
  Navigation text uses the shared 18-pixel Semibold role. Issue #35 adds an optional
  semantic `ModeIcon` to each choice, defaulting to no icon. Canonical 16-pixel SVG
  icons precede labels by 8 pixels; the combined group stays centred inside the
  existing segment geometry. `ModeIconCache` prepares immutable source and separate
  foreground variants before paint. Borders still paint above all child fills,
  preserving the Issue #11 shared-edge and focus regressions.
- `HeaderInformation` owns the regular 14-pixel Inter mode description in
  `#C7CDD4`, centred in the 32-pixel information row. Its 2×16-pixel aqua accent
  has an 8-pixel gap to text and an 8-pixel trailing inset. The screen supplies
  the reserved right-column bounds; typography and global theme tokens are shared
  through the existing font policy.
- `IconActionButton(ActionIcon::settings)` supplies a semantic Settings glyph,
  focus/hover/disabled feedback and native pointer/accessibility/Enter/Space
  activation. Bind `onClick` to the screen's Settings action. Component names
  remain identifiers, not icon/appearance switches.
- `NoticeBar::display(optional<NoticeMessage>)` displays one information/warning/
  error notice or hides on no message. It returns whether content changed, never
  takes focus and owns no timers, queue, workflow or model logic. Bindings decide
  priority and relevance from explicit state fields. Identical refresh is silent;
  detail-only changes do not reannounce the headline. Full detail remains in the
  tooltip/accessibility description. The preferred inline height is 32 pixels.
  `setHeaderAppearance(true)` opts only the workspace header notice into the
  matching `#C7CDD4` body ink. Severity icons/stripes and non-header defaults stay
  intact. The description and notice use the same regular font and vertical
  alignment. The Issue #35 native look/interaction checkpoint was owner-approved
  on 4 November 2026; actual Windows DPI and FL Studio coverage remain separate.

`tests/ControlLibraryCases.h` exercises native events, accessible state, silent
refresh, rejected/stale intents and callback lifetime. The UI test executable's
`--review-controls` mode opens the same interactive fixture without FL Studio;
normal test execution also renders its state gallery when
`PREVERB_UI_M3_OUTPUT` is set. Its message-loop helper is enabled only in the test
target, not the plugin. The initial families' owner approval is recorded in
`docs/validation/shared-mode-header-evidence.md`; material changes reopen that gate.
`tests/NoticeBarCases.h` and `--review-messages` provide the corresponding native
message-bar/polished-header fixture. Its component acceptance precedes production
message binding; prototype severity examples do not imply simultaneous notices.

`WorkspaceNotice.h` selects a single current notice from explicit workflow,
capture, action/IR error and existing offline-result fields. Failures precede
recording/settlement and contextual Recapture/Export guidance; routine ready/idle
status is quiet. The editor reserves one notice row beneath the branding, keeping geometry fixed
across notice visibility changes without clearing focus or committing an active
value edit. The divider above global settings remains clear of notice paint. No notification queue,
expiry timer or generic parsing of engine progress messages is involved.
`--review-workspace` opens the real processor/editor test fixture for integration
review without a DAW project; it supports the actual mode and Settings controls.

This stage does not claim a full library migration. KnobValue/global sliders,
general buttons/toggles/dropdowns, row menus, scroll containers and window content
remain on their existing implementations until their planned workspace refactors.
Timeline/Overview remain specialized controls with their current geometry and
model guards. Retained name-based theme branches outside the new Settings action
are follow-up consumers, not a precedent for new controls.


## Export control family (issue #6, approved 2026-11-04)

`Controls/ActionButton.h` owns shared command-button keyboard activation,
invocation hooks, semantic play/stop/save/return glyphs and in-bounds focus ink.
`Controls/FileDragTile.h` owns a once-per-pointer-gesture external-file intent
and click/Enter/Space Save fallback, with no path storage or filesystem I/O.
Bindings own current-file verification and session/revision authority. The tile
never claims the host accepted a drop. Disabled actions do not invoke bindings;
keyboard activation remains usable after dragging. Native cases exercise these
contracts before wider rollout. Existing Settings/Diagnostics and Perform text
buttons are untouched consumers, not claimed migrated. The owner approved these families in native component checkpoint A on
2026-11-04; the existing numeric stepper approval is preserved. Integrated
Export workspace acceptance remains a separate checkpoint.

The owner approved the integrated Export presentation and read-only navigation
feel on 2026-11-04. `TimelineNavigation` is the current gesture adapter over
`SwellViewport`; do not promote it into a competing toolbar. Issue #46 owns the
shared Zoom − / slider / Zoom + / Fit family and musical ruler integration.
Export retains its own complete-audible-composition Fit policy and does not adopt
Perform's Snap/grid or editing handles. See the [Export handover](../design/export-workspace.md#deferred-integration-handover-to-issue-46--agent-2).
## Swells list family — issue #4

`Controls/SwellList.h` is the processor-independent, virtualized Swells list
family. It retains the existing Perform row appearance and `swellList` identity;
no new typography, colour, geometry or density value is established. It owns
numeric arrival/ordinal persistent-ID ordering, row hit handling, multi-selection,
keyboard anchoring and scrolling. Bindings supply immutable row values and
selected/primary IDs and own model guards and asynchronous menu lifetime.
Callbacks return IDs or row-value copies, never model row indices.

Snapshot replacement suppresses selection callbacks and restores the primary
keyboard anchor explicitly. Structural changes preserve the first surviving
visible row and its pixel offset; missing anchors use nearby visible identities
and then a clamped prior position. An explicitly selected primary or a moved
visible primary is revealed with minimum scrolling. Routine/cosmetic refresh
does not pull back to an offscreen primary. A changed session epoch resets
ephemeral scroll ownership. No refresh edits state or initiates processing.

`preverb_swell_list_test --review` runs the same control in a lightweight native
fixture; normal execution tests real pointer/keyboard row interactions, snapshot
replacement, scrolling and virtualization. The pure ordering contract is covered
by `preverb_swell_list_order_test`. The owner approved the standalone family's
look and interaction feel on 4 November 2026 before editor integration. The
Perform editor now uses this family; delayed row commands carry a coherent
snapshot session epoch checked under the engine lock, including atomic targeted
deletion. `SwellListBindingCases.h` and `SwellListCaptureCases.h` cover production
bindings, sparse captures, loops, recapture and restore. Integrated/host acceptance
remains separately recorded in the [issue #4 evidence](../validation/2026-11-04-issue-4-swell-list.md). Settings and
Diagnostics have no duplicate Swells list consumer. Other families and general
menus/scroll containers remain outside this extraction.
