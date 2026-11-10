# Shared UI control contracts — issue #107

**Owner-approved contracts, updated 7 November 2026; production standards revision 14.**
Reconciled with merged #80 at `717c890` (revisions 8–10). This document records
existing mechanisms and accepted contracts for separate family changes; it does
not certify legacy consumers or approve unreviewed production appearance, input
changes, tokens or variants. Independent #108 Level and #8 recovery/engine work remain
outside this delivery. The
[inventory](../design/ui-control-inventory.md) identifies exact consumers/debt,
and [audit evidence](../validation/2026-11-06-issue-107-control-audit.md) distinguishes
source observations from native, OS-DPI and host results.

## Ownership and status

The [production guide](../design/ReverbArise_UI_Layout_and_Style_Guide.md) owns
approved numerical roles; [UI development standards](ui-development-standards.md)
own existing reuse/review policy. Use those authorities, not sampled mockup values.
Library families own appearance, common input lifecycle, focus/accessibility and
feedback presentation. Screens own composition and contextual prose. Bindings
own ranges, parsing, units, precision, validation, accepted state, commands and
object/session/pass/async identity. No library family may access the audio engine,
write assets or change transport policy as part of presentation.

**Observed** means read in source, not necessarily accepted. **Approved** requires
a scoped existing evidence reference. **Proposed** is pending owner acceptance and
the child family checkpoint. **Legacy** is a listed consumer awaiting disposition.
Unchanged extractions retain prior scoped acceptance when equivalence is proved.
Future changes update inventory rows, contract exceptions and evidence together.

## Common state matrix and precedence — accepted contract

### Focus visibility policy

**Owner decision, 2026-11-06:** distinguish focus ownership from its decoration.
Show focus decoration for keyboard navigation and accessibility-driven focus.
Pointer clicks hide the decoration while retaining actual focus, keyboard
activation and the existing hover/held feedback. Repeated clicks on an already
focused control must also hide it. Keyboard navigation/activation or an explicit
accessibility focus request makes it visible again; mouse movement alone does
not clear it. Programmatic restoration retains the preceding input provenance.

Implemented now in the shared C1 command family using `Controls/FocusVisibility.h`:
the observer never consumes keys, invokes commands or stores keystrokes. Other
families adopt this policy when their own design-system jobs begin, with their
normal component/binding checks; do not retrofit them in unrelated work. This
includes the existing native-dialog boundary and specialized FileDragTile focus
presentation. Carets, text-editing baselines, selected/latched state and menu
selection are separate feedback and must remain understandable after pointer use.

The common state matrix below states the family contract, not a claim that every
legacy control already implements this visibility policy.

| State / combination | Required resolution for applicable families |
| --- | --- |
| Idle | Approved neutral surface/type and discoverable enabled affordance |
| Hover / pressed | Use owning family's reviewed feedback; matching command semantics share the same surface response; do not infer semantics from names or labels |
| Focus | Use the focus visibility policy as each family is migrated; retain contained geometry independent of hover, including the approved knob corner motif |
| Selected / latched | Display accepted state; do not confuse inspected/focused option with committed selection |
| Disabled + hover/pressed | Disabled eligibility suppresses activation; resolve disabled ink/surface before transient emphasis; audit existing mismatches before changing them |
| Disabled + focus/selected | Preserve understandable state and a documented focus rule; no blanket removal of selection or unreachable explanation; do not invent a global alpha/focus geometry |
| Read-only | Preserve readable value, selection/copy and explanation where supported; no edits; distinguish from disabled |
| Editing + refresh | Preserve active text/gesture within the same valid target; target/session supersession follows documented cancellation/reconciliation |
| Invalid/rejected | Display binding-supplied outcome, retain accepted state and applicable editable draft policy; never silently publish a rejected value |
| Busy/unavailable | Represent actual binding state/reason without fabricated progress or disabled-only explanation |

Not every row applies to every family. Labels have no activation/pressed state;
native surfaces use native policies; specialized graphs have domain-specific drag
states. Each child records applicability and combined-state tests. Matrices do
not authorize a new visual treatment before review.

## C1 — Commands, icon actions and latches

Implemented owners: [ActionButton](../../Source/UI/Controls/ActionButton.h),
[IconActionButton](../../Source/UI/Controls/IconActionButton.h),
[LatchingActionButton](../../Source/UI/Controls/LatchingActionButton.h) and the
neutral fallback in [WorkspaceTheme](../../Source/UI/WorkspaceTheme.h). All scoped
commands share one resolver/painter; names no longer select appearance. The owner
approved native family revision 4 on 6 November 2026; the integrated commands
and IR alignment correction are also owner accepted. See
[button evidence](../validation/2026-11-06-issue-107-buttons.md).

Focus uses four aqua corner brackets with a 2-pixel stroke and a 3-pixel gap
outside the body border, contained within the component. Neutral hover uses the
muted aqua outline; held press uses separator fill and a stronger aqua outline.
Destructive hover strengthens the red outline; held press fills red. Neutral
icons retain off-white ink. Primary/on-latch surfaces retain dark content on
aqua fill. Disabled body ink is dimmed and suppresses mouse emphasis; any focus
brackets remain aqua. Knob focus and FileDragTile's specialized painter are unchanged.

Compositions reserve the action frame around the intended visible body size.
The shared IR field aligns the import button's visible body with the dropdown's
top, bottom and height, with its focus margin outside that body. Feedback retains
the existing tight gap below the dropdown/body; empty fields retain enough room
for the complete focus frame. The folder glyph keeps its original geometry.

Contract semantic variants: neutral command, primary command, destructive command
and effective-state latch. Glyph-only and text actions are content forms of these
roles, not separate hover policies. File drag tile remains a named C8 exception.
Use explicit typed/family ownership; identifiers remain available for discovery.
Relabeling may alter text layout/accessibility but must not select a visual role.

Pointer activation and Return/Space issue one eligible command; Tab focus is
visible and full stroke stays inside bounds. Capture invocation ownership before
the request, including keyboard and accessible activation; model guards remain
authoritative after queued delivery. Disabled widgets must not issue commands.
Whether an existing pre-invocation hook runs while disabled is distinct from model
mutation; preserve compatibility and test rather than silently changing it.
Each glyph action needs a useful accessible name/help independent of visible text.

Neutral command hover/press uses the shared approved surface; primary and latched
fills remain intentionally distinct. Destructive classification is explicit and
owner reviewed: do not color every command named Delete/Reset by text matching.
Display latch refresh silently from accepted/effective state; preserve requested
versus temporarily overridden Snap/listening state. Long labels retain approved
font and full accessible text; review fit instead of shrinking text opportunistically.

Evidence required: state and combined-state render checks, rename/relabel role
invariance, pointer/Return/Space/accessibility requests, disabled and queued
lifetime cases, invocation guards, silent latch refresh and representative bounds.
Use ControlLibrary/ExportControl/AuditionStrip cases plus real owner bindings.

## C2 — Dropdown selectors and menus

Observed owners: [DeliberateSelector](../../Source/UI/Controls/DeliberateSelector.h),
[IrSelector](../../Source/UI/Controls/IrSelector.h), InvocationComboBox, plus plain
Settings ComboBoxes and dynamically created menus. The shared selector disables
wheel selection; legacy boxes do not establish that library-owned policy.

Contract: selection is deliberate; wheel remains surrounding scroll
input unless a named reviewed exception says otherwise. Keyboard inspection,
popup opening, commit, dismissal and focus restoration are explicit. Capture the
target/catalogue/session before human invocation; asynchronous completion must
revalidate it. Silent display must neither invoke a command nor accept a stale
queued edit. Accepted values/catalogue keys, not index coincidence, own selection.

Document I/F/D/B and popup H/P/F/S/D behavior. Unavailable choices retain truthful
reasons; no enabled-looking selection may bypass binding rejection. Labels and
selected text use semantic roles and retain complete accessible text on overflow.
Menus preserve native popup navigation/dismissal and screen-space anchoring.
Context menu and reset closures retain captured identities and lifetime guards.
No generic popup abstraction may discard parameter-specific reset eligibility.

Evidence: no-wheel routing, keyboard/popup/cancel/focus, disabled choices and
accessible reason, catalogue reorder, stale target/pass/deletion, silent refresh
and long-value geometry. Reuse UiStandards/ControlLibrary/PerformKnobReset and
the #109 IR binding tests on the merged baseline. Plain Settings selectors require a
documented intended policy before migration; they are not certified by inheritance.

### C2 action menus — issue #107

`ChoicePopup` accepts `PopupChoice` descriptors with `PopupRowRole::choice`,
`action`, or `destructiveAction`. The choice role owns selection marks and
selectable accessibility metadata. Neutral actions use the quiet selector
hover/held fills and aqua edge. The destructive role uses `danger` text and
edge ink, with popup-only hover and held blends. Labels are display text.

Supply the complete label, eligibility and visible/accessibility explanation
from the action provider. Unavailable rows remain inspectable. The binding
captures the original action and target before opening and checks eligibility
again on one-shot delivery. Outside dismissal preserves the focus recipient.

The owner approved the native fixture on 7 November 2026. In production, the
Swells menu opens via right-click or the row's three-dot icon. The list owns
the popup across row recycling, and Rename/Delete use the clicked item's
captured ID and epoch. Per-knob Reset uses the same neutral presentation at
the right-click anchor. It displays the provider's label and eligibility,
and invokes the captured `ResetAction`. Pointer return to the dial keeps
keyboard access with its focus decoration suppressed. Keyboard or accessibility
activation can restore the existing brackets. The
[control inventory](../design/ui-control-inventory.md) records the exact scope.

### C2 approved implementation — 2026-11-07

`Controls/ChoiceSelector.h` extends the existing deliberate selector as an explicit
adoption seam. `ChoicePopup` owns a one-level anchored popup; `PopupInteraction`
separates pointer inspection, keyboard navigation, held state and the accepted
checkmark. A pointer departure clears only hover; any valid keyboard inspection
remains. Pointer focus never creates a sticky row background. Choice completion
is one-shot, with the original binding still deciding acceptance.

When keyboard focus leaves the popup, cancel immediately and close after the
native focus transfer. Outside dismissal leaves focus on its new recipient,
including a native host control in the same process.

`setRowBounds` reserves the outside focus frame around an existing visible body
row. `canCommit` is an optional read-only predicate captured after
`beforeInvocation`; Settings bindings use it to reject a changed session/pass or
selected swell before the original command runs. The control owns no engine.

The popup's root owns real keyboard focus while unavailable child rows remain
disabled and accessible. Keyboard/accessibility inspection can reach such a row,
but neither its press action nor Return/Space grants permission to select it.
This avoids treating an inspectable row as an enabled command. Silent refresh,
catalogue changes and deferred completion retain captured identity/lifetime guards.
`setChoiceReason` associates provider-supplied reasons with stable item IDs;
disabled Export Custom remains an accepted display value, never a preset command.

`SelectorPresentation` owns **native-approved popup-only metrics**: width 220–420,
outer padding 4, row padding 8 and check column 22 logical units. Popup fonts and
composition follow the source's user transform; OS DPI remains JUCE's concern.
Existing command bracket stroke/gap and body geometry are reused. Raised/held
surfaces derive from existing background/outline colours; no raw palette or
global popup colour override is introduced. Scoped native family approval was
recorded on 2026-11-07; standards revision 14 governs explicit consumer adoption.

Owner feedback approved the **red message plus red body border** field treatment
on 2026-11-07. `FieldPresentation` owns the initial small local feedback policy:
12-height Inter Regular, 17-height line box and 4-unit gap from the visible body,
left-aligned with that body. `ChoiceSelector::setValidationMessage` accepts a
binding-supplied error without parsing input or changing selection. Error border
ink stays red during hover/held feedback; outside focus brackets remain aqua.
The preview replaces routine guidance with the local error, keeps disabled
explanations readable and restores normal help after a valid simulated choice.
These roles were approved in the updated native fixture; production adoption is scoped to the C2 change.

Preview: `preverb_selector_popup_test --review-selectors`; focused checks use the
same executable without arguments. The fixture offers size/scale and normal,
dense, empty and unavailable/error scenes. Adopted consumers are Learn/Perform IR,
Perform grid, Export Character, Appearance size/scale, Playback routing/gain policy
and Swell fade-in/out curve. All use deliberate no-wheel selection and silent
accepted refresh. Control values retain the approved minimum height 13 above
12-height feedback. The approved browser design and native owner acceptance are
recorded in `unify-selector-states`. #108 Level, #8 recovery and
storage/working-folder controls, C1 buttons and action-menu adoption are separate.

## C6 — Switches and toggles

Owners include [BooleanToggle](../../Source/UI/Controls/BooleanToggle.h) and
[Switch](../../Source/UI/Controls/Switch.h). Learn and Settings use labeled
tracks, while Enabled, Solo and Freeze use inline checkboxes. C1 owns action
latches. Hidden compatibility controls retain their existing owners.

Contract: define explicit boolean-setting versus accepted-effective-latch
semantics. Boolean controls expose accessible name/state, pointer/Space activation
and visible keyboard focus. Return behavior must be recorded per family rather
than assumed from command buttons. State is supplied from accepted model refresh;
rejected requests reconcile without a second write. Disabled/read-only policies
include a discoverable reason, preserve displayed state and block mutation.

I/H/P/F/D/S combinations must be reviewed within the existing family geometry.
Do not standardize the .35/.4 disabled alphas or checkbox/track focus differences
without explicit approval. Screens own adjacent captions/help; associations belong
to the shared composition. Capture existing pass/session hooks and retain silent
programmatic refresh. Evidence: pointer/key/accessibility toggling, D+F/S cases,
stale pass rejection, no extra display callbacks, hit/focus bounds and caption fit.
Reuse ControlLibrary/UiStandards and Learn pass policy plus actual editor bindings.

P6 uses two labeled forms of the same boolean control.
The track and checkbox share quiet hover, held feedback and independent outer
aqua brackets. Pointer clicks hide the brackets and keep actual focus. The
label and indicator share a single Tab stop and guarded activation. Space,
existing Return behavior and accessibility use the same invocation path.
The disabled form keeps the accepted state readable. The owner approved the
native JUCE preview on 10 November 2026.

## C7 — Navigation, lists, tabs and scroll containers

Observed owners: [SegmentedSelector](../../Source/UI/Controls/SegmentedSelector.h),
[SwellList](../../Source/UI/Controls/SwellList.h), TimelineZoomControls,
[FocusViewport](../../Source/UI/Controls/FocusViewport.h), plus Settings tabs.
Segmented selection already distinguishes accepted selection from keyboard
inspection: arrows/Home/End inspect; Return/Space requests; wheel does not choose.
Unavailable choices expose reasons and are not committed. Keep joined-edge paint
ordering and accessible selected state. Ordinary tab policy can remain distinct
from segmented workflow navigation when named and reviewed.

Contract: focus, inspected option, accepted selection, scroll and reveal
are separate state. Refresh must preserve stable row identity/primary selection
and scroll intent; explicit list navigation may reveal the existing contextual
Fit, as approved for #93. Row Enabled/menu hit regions need keyboard/accessibility
coverage as well as pointer geometry; an inaccessible RowSurface is not proof of
accessible row commands. Preserve parent ListBox policy and audit actual gaps.

Focused controls must be reachable inside scroll content and full focus strokes
contained. FocusViewport reveal is an explicit usage policy; the type alone does
not prove it is enabled. Disabled navigation cannot mutate the model. Overflow
may use bounded scrolling and full accessible text; keep compact composition and
comfortable auxiliary typography distinct. Display/refresh is silent.

Evidence: keyboard order and unavailable inspection, selected/hovered adjacent
edges, accessible selected state and row action access, rejected requests, silent
refresh, row identity/reorder/delete/scroll preservation, explicit versus timer
reveal, tabs, focused children beyond viewport, minimum/larger/scaled geometry.
Reuse ControlLibrary/SwellListBinding/PerformNavigationState/ResponsiveEditor/
ResponsiveScale cases and their existing native preview paths.

## C3 — Labels, context help, notices and field feedback

Observed owners: semantic TextRole policies, plain labels,
[EllipsisLabel](../../Source/UI/Controls/EllipsisLabel.h), HeaderInformation,
[NoticeBar](../../Source/UI/Controls/NoticeBar.h), TransportReadout and the editor
context-help footer. Settings Swell source range and the shared IR field have
dedicated feedback composition. Other label/help layouts remain screen-built.
Noninteractive text has no activation or pressed state.

Contract composition: a reusable field presents a caption, existing input,
optional helper and optional binding-supplied validation message. Associate the
caption/description/error accessibly with the actual input and maintain its
accessible role/value. Screen-specific scope prose stays with the screen, as
explicitly required by #80. Use the #109 grouped feedback pattern as the existing
reviewed reference as delivered; its acceptance does not automatically approve
every field's new geometry or behavior.

Feedback precedence: actionable validation/rejection occupies the local feedback
slot before routine field guidance; routine success/Ready does not displace an
error. Workspace notices retain their existing severity/readiness authority.
Bottom context help describes the hovered control/region and retains the full
message in its accessible description; it must not become the sole place to find
a persistent error. Keep input enabled-state separate from feedback readability.
No popup tooltip host, waveform overlay or forced spoken announcement is added.
Standard accessibility events remain available; unchanged refresh must not spam
events or expose stale cleared text.

Use approved text roles, face and line metrics. Overflow is handled through the
family's named wrap/ellipsis/full-description policy; never arbitrary text shrink.
The approved responsive NoticeBar bounded text rule and fixed transport readout
are explicit existing exceptions to generic labels. Their policies are not new
permission to resize other captions. Empty feedback collapses only in an approved
composition; preserve reserved workspace notice geometry and screen stability.

Evidence: actual font metrics/line boxes, long names/help/errors, empty/cleared
feedback, accessible associations/full descriptions, disabled input with readable
error, context-help inheritance into nested/auxiliary controls, notice precedence
and no-forced-speech source/interaction checks. Reuse UiStandards, WorkspaceNotice,
WorkspaceFeedback, WorkflowHeader and responsive cases, plus #109 field cases.

### Approved field anatomy — 2026-11-07

Use the owner-approved small helper text directly beneath the input body, aligned
with its left edge. A binding-supplied error replaces routine guidance with red
text and a red body border; keyboard/accessibility focus decoration stays aqua
and separate. Helper/error text wraps without shrinking and stays readable when
the input is disabled. Disabled state alone is not a validation error. Do not
introduce a new validation, parsing or commit rule through this presentation.

`FieldPresentation` provides the approved 12-height Inter, 17-unit line box
and 4-unit gap. The first C3 adopter is Settings → Swell → Source range (bars).
Each editor has its own caption above and exposes the shared, full description.
The visible message wraps within the two input bodies and their gap, and the
controls below follow its measured height. The command owns rejection and
affected-field metadata. Correction, selection change and session replacement
clear the local error. Return and the explicit button submit the pair.

The source-range group established the shared presentation. Adopt local help
and validation as part of each field family's plugin-wide rollout wherever the
control has a feedback position. Document whether that position carries help,
validation or both. The owning binding continues to decide how an edit commits
or fails. The approved IR field is another example of local feedback.

## C4 — Numeric and text fields

Observed mechanisms differ intentionally:

| Existing subtype | Commit/cancel/refresh authority to preserve |
| --- | --- |
| KnobValue Entry | Return and typing focus-loss commit; Escape cancels; same-identity display preserves active text, identity change cancels; restore accepted canonical display after commit/rejection; exact text is separate from rounded display |
| SliderControl FieldNumericInput | Shared numeric label and temporary FieldTextEditor retain JUCE's editor lifecycle, slider range and formatter. Where present, step buttons and wheel over the value or arrows use one configured step and capture invocation ownership. |
| Settings bar range FieldTextEditor pair | Digits-only input. Focus loss checks the draft against its bound swell and session. Return or Set source from bars submits to that same context. A selection or session change clears the pair. |
| Read-only diagnostic fields / placement | Copy/inspect accepted values, no edit publication; hidden Name seam remains hidden |
| Rename dialog FieldTextEditor | Focus loss validates. OK/Return submits a nonempty name to the captured ID and epoch. Cancel/Escape discards the draft. |

P4 uses the shared field surface for the inspector, embedded entries, Rename
and read-only Diagnostics. Keyboard and accessibility focus show the aqua
brackets while pointer editing keeps its caret and selection. Numeric editors
filter letters on insertion, including paste. The timing entries still use the
song-position parser. Standalone and embedded steppers share one-step wheel
behavior. Slider tracks follow C5 for Ctrl+wheel and retain ordinary drag with
either Ctrl state.

Contract: reusable field composition extends these working mechanisms;
it does not introduce another parser or universal commit policy. Each field must
declare its subtype, units/range/precision, Return/Escape/focus-loss behavior,
invalid-draft policy and active-edit refresh policy in its binding packet. Capture
ownership before all typed, stepped, dragged and accessible edit paths. A stale
session/selection/catalogue change cannot redirect an edit to a different object.

Bindings return an accepted/rejected outcome and explanation where applicable;
presentation renders it using C3. Preserve current clamp-versus-reject semantics
and accepted exact precision. Rejection restores accepted presentation or retains
an explicitly documented editable draft as the subtype requires; never publish
the draft or overwrite another active field. Do not turn label refresh into an
edit notification. Read-only and disabled values need separate accessible state.

Text editors retain selection, caret and keyboard copy where permitted. Steppers
have named increment/decrement actions, complete focus/hit bounds, range limits
and one gesture lifecycle per request; their glyph-hover feedback is a subcontrol
policy, not an ordinary command variant. Invalid state is not conveyed by color
alone; make the binding's explanation visible and accessible near the input.

Evidence: exact typing beyond display precision, units/range boundaries,
Return/Escape/focus-loss by subtype, active refresh/identity supersession, invalid
and stale rejection, pointer/keyboard/accessibility increments, repeat behavior,
disabled/read-only interaction, silent display and caption/input/feedback fit.
Reuse ControlLibrary/PerformKnob/PerformKnobReset/ExportLevels and real inspector,
rename and bar-range bindings. Child changes must add missing subtype lifecycle
coverage rather than infer it from the parent LookAndFeel.

## C5 — Knobs and sliders

Owners include [KnobValue](../../Source/UI/KnobValue.h),
[TrackSlider](../../Source/UI/Controls/TrackSlider.h), and the companion
numeric field in SliderControl. Zoom and Height use display-only bindings.

Contract: each family owns coarse/fine drag, keyboard increments, wheel
travel/direction and reset defaults once; bindings supply musical units, range,
step, conversion and exact canonical values. Keep visible dial/fader/value text
and accepted model state coherent. Return/Escape/typed editing follows C4.
Ctrl Snap bypass, Shift fine adjustment and reset intent remain explicit parameter
policies where already implemented; no generic normalization changes their meaning.

An input gesture captures target/session and preserves its anchor through queued
changes; active modifier changes rebase through the existing mechanism. Selection,
deletion, timing, render/freeze or session invalidation must reject/cancel the
same cases as current bindings. Silent display is not another input gesture.
Pointer, wheel, key, stepper and accessible value changes follow the same binding
authority. Wheel outside the value control retains normal surrounding scroll.

P5 uses quiet hover and held feedback along the dial arc or on the track body, aqua value ink, an off-white
position marker and independent keyboard/accessibility focus brackets. Horizontal
zoom keeps its text-box-free track. Ctrl+wheel fine-adjusts dials and tracks
where their domain allows a smaller step. Timing Ctrl+wheel uses the existing
unsnapped step, and Shift retains its established finer behavior. A configured
home attracts raw pointer drag candidates within its band. Wheel, exact entry,
accessibility and Reset follow their independent paths. The Level detent is
centered at 0 dB with a ±0.3 dB band. Companion dial fields use the shared
value width when it fits. Inline feedback aligns beneath the field and wraps
within the value column. Target's hover-help footer and accessible field
description explain the active entry units. Its validation stays beneath the
field.

Evidence: coarse/fine drag/wheel/key/reset, explicit/implicit edit cancellation,
actual unit steps, exact value/display agreement, captured stale intent rejection,
disabled/accessibility paths, silent refresh and minimum/scaled focus bounds.
Reuse PerformKnob/PerformKnobReset, ControlLibrary, ExportLevels,
PerformNavigationState and ResponsiveEditor/Scale tests. Model/audio invariance
checks remain binding-specific whenever a child touches those paths.

## C8 — Specialized canvases, native surfaces and auxiliary shells

Observed owners: SwellTimeline/SwellOverview and waveform handles, LearnWorkspace
inspection canvas, Export TimelineNavigation, FileDragTile/ExternalFileDrag,
PresentationSurface, WorkspaceWindow and native choosers/chrome.

Contract: specialized controls own domain hit geometry, zoom/pan/pin,
selection and keyboard equivalents. Extend their focused mechanisms with shared
roles/help/focus where applicable; never rebuild the waveform as generic sliders.
Do not change captured-grid/live offsets, retained source/render identity,
envelope math, timing guards or viewport selection policy in a presentation slice.
Document unsupported keyboard/accessibility actions as gaps for scoped child work,
not as behavior proven by a painted highlight.

Keep Learn inspection pin and transient hover distinct; Escape clears only in its
existing focus context. Keep Perform drag anchor/frozen/session policy and display
height separate from gain. Export navigation remains left-position/right-pan,
wheel zoom/Shift-wheel pan and Home Fit. FileDragTile's dashed/solid focus border,
drag threshold and suppression of click after drag are an explicit #6 variant.
Its binding retains file/revision/session eligibility; the control owns no path.

Native file choosers and window chrome remain native. Application launch and
completion must retain lifetime, target, cancellation and session/pass checks.
Auxiliary windows use one magnification transform, comfortable typography,
work-area bounds and explicit focus reveal. Main 900x600 logical composition at
100% is independent of user magnification and physical OS DPI. Do not infer
host resize/work-area support from software images. Disabled editing does not
erase source data, and unavailable audio must not be drawn as captured silence.

Evidence: specialized pointer/key/hit/drag bounds and accessibility applicability,
stale and frozen gesture rejection, display-only zoom invariance, row/popup screen
anchors, file drag-versus-save/cancel/stale completion, focus reveal and auxiliary
work-area constraints. Reuse LearnTimeline, PerformNavigationState, ExportControl,
ResponsiveEditor/Scale and relevant timeline/reset/persistence oracles. Child
native review must use the actual prepared fixture and retain one review window.

## Named exceptions and acceptance register

| Policy | Current authority / disposition |
| --- | --- |
| Neutral/primary/destructive/latching command state | Shared resolver/roles and IR geometry approved at revision 12; keyboard/accessibility-only decoration approved at revision 13; shared C1 implementation, other families retain their staged jobs |
| No-wheel selector | ChoiceSelector explicit C2 consumers approved at revision 14; Recovery candidateBox and action menus remain separate debt |
| Knob corner focus, Level step/fine interaction | Production guide revision 4 / #46 evidence; retained, no global focus redesign |
| Segmented inspected versus accepted selection and joined-edge painting | Existing header/control policy and scoped #35/#37/#58 acceptance; not generic tab behavior |
| Stable fixed transport typography and bounded notice wrapping | Production guide revisions 6/7 and no-forced-speech spec; not generic text shrink permission |
| File-drag tile dashed affordance | Existing #6 Export family; retain separate native-file semantics |
| Grouped local actionable error/collapsed empty IR feedback | #109 scoped field approval; merged and present here; C3/C4 extension candidates need their own family checkpoint |
| Comfortable auxiliary typography, main responsive roles, single transform | Production guide revision 7; physical DPI/host coverage remains separately recorded |

No new token, appearance exception or revised interaction is approved by this
register. State precedence and shared feedback composition are accepted contracts. Legacy
family migrations still require their own scoped owner/native review before rollout.

## Family delivery acceptance

For each P1–P8 packet, name actual consumers, control reuse, binding guards and
exceptions before proposing its child OpenSpec change. Add focused component and
binding tests, run the supported build wrapper and UI standards guard, and open
the native preview directly for any new/revised family before broader adoption.
Update code, standards revision, living inventory and dated evidence together
when its scope is approved. Remove exact obsolete debt entries only for delivered
consumers; do not regenerate the standards debt inventory to pass a check.

Use the common review matrix in the inventory. Keep test outcomes, owner native
approval, Windows DPI and FL Studio results distinct. An audited family can retain
its accepted implementation without migration when evidence proves its contract;
shared LookAndFeel ancestry alone is insufficient. #107 tracks the full roadmap
after this umbrella audit is accepted and archived through the normal workflow.
