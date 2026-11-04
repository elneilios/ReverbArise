# UI development standards

Use these standards when implementing or refactoring ReverbArise UI. Consolidate
controls as the planned work touches them; a separate whole-plugin migration is
not required before the Learn / Perform / Export work can proceed. Existing UI
is not considered compliant merely because it inherits the shared LookAndFeel.

## Shared controls and ownership

The [production guide](../design/ReverbArise_UI_Layout_and_Style_Guide.md) is the
numerical authority (production standards revision 1). Its Learn/header table
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

For each UI issue, inventory the affected controls and identify which library
components or specialized controls own them. Extract a family once, migrate its
affected consumers, and remove their duplicate setup. Include corresponding
Settings/Diagnostics consumers when they share that affected interaction.
Record untouched consumers and follow-up work honestly in the issue/change;
do not label a family fully migrated while alternate implementations remain.

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

1. Its ordinary OpenSpec design cites standards revision 1, the current shared
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
