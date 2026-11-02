# UI development standards

Use these standards when implementing or refactoring ReverbArise UI. Consolidate
controls as the planned work touches them; a separate whole-plugin migration is
not required before the Learn / Perform / Export work can proceed. Existing UI
is not considered compliant merely because it inherits the shared LookAndFeel.

## Shared controls and ownership

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

## Incremental migration

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

## Initial control-library API (issue #16)

The first families live under `Source/UI/Controls/`; they are introduced in a
native review fixture before integration into the production header:

- `SegmentedSelector::display(choices, acceptedId)` silently supplies labels,
  help, availability and unavailable reasons. `onRequest(id)` emits an intent;
  the binding must revalidate it against the model and call `display` with the
  accepted state. Pointer/Enter/Space activation never toggles optimistically.
  Arrow/Home/End keys inspect choices without selecting them; Tab enters/leaves
  the group. The group exposes all reasons through accessibility descriptions,
  including disabled choices. Wheel and repeated active selection are no-ops.
  Segments share square internal joins with one border; only the group's outer
  left/right ends are rounded, following owner checkpoint-A feedback.
- `IconActionButton(ActionIcon::settings)` supplies a semantic Settings glyph,
  focus/hover/disabled feedback and native pointer/accessibility/Enter/Space
  activation. Bind `onClick` to the screen's Settings action. Component names
  remain identifiers, not icon/appearance switches.

`tests/ControlLibraryCases.h` exercises native events, accessible state, silent
refresh, rejected/stale intents and callback lifetime. The UI test executable's
`--review-controls` mode opens the same interactive fixture without FL Studio;
normal test execution also renders its state gallery when
`PREVERB_UI_M3_OUTPUT` is set. Its message-loop helper is enabled only in the test
target, not the plugin. Component approval remains pending until recorded.

This stage does not claim a full library migration. KnobValue/global sliders,
general buttons/toggles/dropdowns, row menus, scroll containers and window content
remain on their existing implementations until their planned workspace refactors.
Timeline/Overview remain specialized controls with their current geometry and
model guards. Retained name-based theme branches outside the new Settings action
are follow-up consumers, not a precedent for new controls.
