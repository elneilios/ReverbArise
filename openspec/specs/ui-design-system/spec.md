# ui-design-system Specification

## Purpose

Keep ReverbArise's approved production UI reproducible through traceable standards,
reusable controls, controlled density changes and practical automated/native review.

## Requirements

### Requirement: Production standards identify their approval baseline
The production style guide SHALL distinguish validated production standards, historical design intent and provisional suggestions. Each validated role SHALL identify its code authority, scope and approval evidence. Metrics SHALL derive from the approved Learn/header implementation and native review, with untested environments disclosed.

#### Scenario: Extract a production role
- **WHEN** a contributor looks up typography, colour, spacing, dimensions, borders, icons, focus or responsive layout
- **THEN** the guide identifies the implemented role, precise units and consumer scope without treating a mockup suggestion or unrelated legacy value as validated

#### Scenario: Conflicting historical guidance
- **WHEN** historical mockups or provisional tables differ from the approved Learn/header implementation
- **THEN** the production section identifies the approved baseline and historical material remains clearly labelled as intent rather than a competing numerical authority

### Requirement: Reusable patterns own appearance and interaction
New or refactored UI SHALL reuse repository-owned control families and named presentation policies. Screens SHALL own composition and bindings SHALL retain domain validation and identity. Library changes SHALL preserve accepted appearance, input behaviour and silent refresh; remaining legacy consumers SHALL be recorded rather than silently treated as migrated.

#### Scenario: Add or revise a control
- **WHEN** a change uses headings, labels/helper text, switches, selectors, numeric up/down entry, sliders, buttons, empty states, notices or focus treatment
- **THEN** its plan identifies existing ownership and reuse, and any missing abstraction extends a working mechanism with focused component and binding validation

#### Scenario: Extract without changing accepted views
- **WHEN** a shared presentation policy replaces duplicate Learn/header setup
- **THEN** geometry, font faces, focus visibility, keyboard/typed edits and accepted model state remain equivalent, including rejected or stale intents

### Requirement: Density preserves invariant visual and interaction roles
Semantic colour, legibility, focus and control interaction language SHALL remain consistent across density profiles. Typography SHALL use the owner-approved responsive compact/expanded roles, rather than fixed heights across all sizes. The approved Learn spacing SHALL be documented as comfortable. Compact geometry SHALL require scoped native approval across its affected Learn, Perform, Export and header consumers before production rollout; it SHALL NOT be achieved by arbitrary text reduction or unreviewed token values. The 900x600 baseline SHALL work at 100% UI scale, independently of optional reviewed proportional scaling.

#### Scenario: Prepare Perform work
- **WHEN** a contributor plans a denser three-column Perform workspace
- **THEN** the plan reuses invariant roles and proposes spacing changes through a named profile review, without treating current legacy spacing or unvalidated compact numbers as approved

#### Scenario: Adopt a compact profile
- **WHEN** a compact proposal is assessed at supported sizes with representative dense content
- **THEN** approval covers readability, overflow, waveform room, hit targets, keyboard and focus before its values enter production, and changes to comfortable consumers are limited to their explicitly reviewed responsive composition using the explicitly approved responsive text roles while preserving focus and interaction policies

#### Scenario: Review the compact baseline across modes
- **WHEN** 900x600 composition and optional proportional scales are presented for acceptance
- **THEN** 100% UI scale independently passes readability, useful waveform space, hit-target, keyboard and overflow review in all three modes
- **AND** any compact header treatment, inspector section family or new metric receives named scoped approval before production adoption

### Requirement: Styling drift has actionable automated checks
A lightweight repository check SHALL reject unapproved raw styling and duplicated interaction policy in governed UI files, with actionable file/rule diagnostics and nonzero failure. It SHALL distinguish styling from legitimate layout/domain calculations and allow only explicit scoped exceptions. Existing debt SHALL be bounded so enforcement does not require unrelated migration.

#### Scenario: Introduce an unapproved style
- **WHEN** a governed UI file adds a raw colour, font construction, styling setter value or duplicated interaction default outside an approved definition or exception
- **THEN** the check fails with a location and remedy, and fixtures prove the rejection

#### Scenario: Legitimate geometry and existing debt
- **WHEN** layout uses named roles and domain calculations, or a declared unchanged legacy occurrence remains
- **THEN** it passes without requiring review for ordinary placement arithmetic, while a new occurrence cannot hide behind a file-wide legacy exemption

### Requirement: Component and native evidence establish practical UI confidence
Automated checks SHALL cover resolved typography, clipping, focus containment, layout/overflow and keyboard/edit interaction for affected patterns. Reusable native fixtures SHALL support visual review at the 900x600 baseline at 100% UI scale, intermediate sizes, responsive breakpoints, 1100x850 and documented larger sizes. Evidence SHALL separate automated proof, native owner approval, actual Windows DPI and FL Studio results, recording untested coverage honestly.

#### Scenario: Review supported geometry and scale
- **WHEN** Learn/header and shared patterns are validated
- **THEN** results include normal/focused/disabled/error and long-content cases across supported sizes and software scales, while Windows 100/125/150/200% checks are separately passed, failed or untested

#### Scenario: Approve a material family revision
- **WHEN** a new or materially revised control family is ready for rollout
- **THEN** its prepared native preview is opened for owner look and interaction review before further consumers adopt it; passing automated cases and prior approvals are not repeated without a concrete reason

#### Scenario: Primary 1080p acceptance and complete control access
- **WHEN** compact implementation is validated on the owner's 1920x1080 display
- **THEN** actual Windows scaling and FL Studio work-area fit are recorded, Reset remains visible without scrolling and every supported control has demonstrated mouse/keyboard access
- **AND** separate results cover Windows 100/125/150% where available, with 200% and any non-fitting combinations explicitly passed, failed, unsupported or untested rather than inferred from software screenshots

### Requirement: Standards evolve through traceable review
Agent guidance SHALL direct UI work to the production guide and shared controls. New tokens, variants, density profiles and one-off exceptions SHALL require explicit owner review, named scope, rationale, affected consumers and validation. Approved changes SHALL update versioned code/documentation together; future UI plans SHALL identify reuse, justified exceptions and review gates without modifying generated OpenSpec workflows.

#### Scenario: Propose an exception
- **WHEN** an agent needs a visual or interaction rule outside the accepted system
- **THEN** it presents a scoped proposal and native evidence where applicable, records approval before rollout and adds the corresponding documentation and regression coverage

#### Scenario: Hand over to Perform
- **WHEN** the standards implementation is declared ready for Perform visual work
- **THEN** approved baseline references, control ownership, passing checks, remaining migration and host/DPI gaps are discoverable, with no claim that compact spacing has already been approved

### Requirement: Shared notices do not force speech
Shared notices SHALL expose visible severity and accessible title, description and help text, with standard accessibility change events. They SHALL NOT explicitly invoke text-to-speech or post forced spoken announcements for notice changes, regardless of whether an automation or accessibility client is active. This SHALL NOT disable accessibility or change operating-system settings.

#### Scenario: Export readiness returns to routine guidance
- **WHEN** Export changes between preparation, warning and the DAW wet-bypass information notice
- **THEN** the visible and accessible notice updates without the plugin synthesizing speech
- **AND** assistive software can still obtain the notice through standard accessibility interfaces

#### Scenario: Unchanged and hidden notice refresh
- **WHEN** the same notice is refreshed, hidden or shown again
- **THEN** no forced speech occurs and unchanged refresh does not create redundant notice-change events
- **AND** cleared notices expose no stale title or description

### Requirement: Control consumers have a traceable family inventory
The design system SHALL provide a maintained inventory of actual controls across all workspaces and owned auxiliary windows. Each consumer SHALL identify its family or explicit legacy owner, semantic role, applicable states, interaction policy, approval evidence and migration disposition. An omitted or unreviewed consumer SHALL NOT be represented as compliant.

#### Scenario: Audit a control in an auxiliary window
- **WHEN** a contributor looks up an interactive Settings or Diagnostics consumer
- **THEN** the inventory identifies its current owner, applicable state and interaction differences, approved variants and remaining debt with source references
- **AND** compact workspace composition, user magnification and auxiliary comfortable typography remain distinguishable

#### Scenario: Specialized interaction has no generic equivalent
- **WHEN** an audit includes a timeline handle, list row, navigation surface or native chooser
- **THEN** the inventory records the specialized or native policy, reusable parts and review status without requiring replacement by a generic widget

### Requirement: Families expose explicit semantic state and interaction contracts
Each control family SHALL document applicable visual states, keyboard and pointer behavior, focus, accessibility and disabled or read-only behavior using approved presentation roles. Matching semantic actions SHALL share a contract; intentional differences SHALL be named and traceable. Migrated appearance SHALL NOT depend on incidental component names or visible labels.

#### Scenario: Text and glyph command actions are compared
- **WHEN** command actions with matching semantics are idle, hovered, pressed, focused or disabled
- **THEN** their contract specifies consistent surface feedback and activation behavior, with separate explicit primary, destructive or latched variants where needed
- **AND** renaming or relabeling a migrated action does not select a different visual variant

#### Scenario: Interaction differences are intentional
- **WHEN** a selector rejects wheel selection while a value control permits wheel adjustment
- **THEN** the contracts identify the intended policies and their rationale rather than treating different control semantics as inconsistent

### Requirement: Reusable fields define help and validation presentation
Field contracts SHALL define composition of captions, inputs, help and validation feedback, including accessible associations and overflow. They SHALL specify Return, Escape, focus-loss, invalid-input and refresh behavior for applicable field types. Bindings SHALL retain domain validation and accepted-state authority; presentation SHALL NOT silently invent new rejection or commit behavior.

#### Scenario: Invalid or stale input is rejected
- **WHEN** a binding rejects a field edit as invalid or stale
- **THEN** the field follows its documented reconciliation policy and exposes the binding-supplied explanation through the applicable visible and accessible feedback
- **AND** the rejected edit does not mutate accepted model state or erase unrelated active input

#### Scenario: Help and validation coexist in a constrained view
- **WHEN** a field with a long caption, help and validation feedback is shown at a supported size or user scale
- **THEN** its contract preserves readable input, discoverable full feedback and keyboard access
- **AND** shared contextual help and local validation have explicit precedence without introducing forced speech

### Requirement: Family migrations have independently reviewable delivery boundaries
Design-system delivery SHALL separate umbrella audit and contract completion from family migration completion. Each migration SHALL name consumers, reuse, variants, compatibility guards, automated checks and scoped native approval. Remaining consumers SHALL stay explicit debt until delivered; family delivery SHALL preserve commands, invocation/session guards, persistent state and audio behavior.

#### Scenario: Umbrella audit is delivered before all migrations
- **WHEN** the inventory, contracts and family roadmap are accepted
- **THEN** umbrella audit completion is recorded separately from pending family migrations
- **AND** the umbrella issue remains open while accepted implementation slices are outstanding

#### Scenario: Revised family becomes ready for adoption
- **WHEN** a child change materially revises a control family
- **THEN** focused component and binding checks establish its state, keyboard, accessibility, disabled and rejection behavior before broader adoption
- **AND** its prepared native preview is opened for owner look and feel approval, with standards revision and evidence updated upon approval
- **AND** minimum, larger-size and user-scale results are recorded separately from actual Windows DPI and host coverage

#### Scenario: Partial migration preserves unrelated behavior
- **WHEN** an approved child change migrates named consumers
- **THEN** only that reviewed scope is marked migrated and its obsolete debt entries are removed
- **AND** protected commands, session identities, saved assignments, captures and renders retain their existing behavior

### Requirement: Migrated command states follow explicit semantics
Migrated text and glyph command actions SHALL share reviewed, distinguishable idle, hover, pressed, focus and disabled feedback for equivalent semantic roles. Primary, destructive and latched variants SHALL remain explicit. Names and labels SHALL NOT select appearance. Neutral glyphs SHALL retain steady ink while border/background provide mouse feedback; semantic filled variants SHALL retain readable contrast. Specialized file-drag presentation SHALL retain its scoped behavior.

#### Scenario: Command metadata changes
- **WHEN** a migrated action is renamed or relabeled without changing its explicit semantic role
- **THEN** its state surface and content role remain unchanged apart from the intended text and accessible-name update
- **AND** an incidental name matching an old play, stop, reset or import identifier does not change its glyph or variant

#### Scenario: Focus and disabled combinations are rendered
- **WHEN** a migrated command is hovered, pressed, keyboard-focused or disabled, including applicable combined states
- **THEN** its reviewed state matrix provides contained focus and consistent semantic content/surface feedback
- **AND** disabled state does not make the action appear eligible or permit invocation
- **AND** focus uses aqua outer corner brackets with a small gap from the button border and does not mask hover or held-press feedback, including on destructive actions

#### Scenario: Shared IR import button aligns with its selector
- **WHEN** the shared IR field lays out its selector and import action
- **THEN** the import button's visible body matches the selector's top, bottom and height
- **AND** its original folder glyph fits within that body, the outside focus frame remains contained, and field feedback retains its existing gap below the inputs

#### Scenario: Input method controls focus visibility
- **WHEN** a shared command receives keyboard navigation or explicit accessibility focus
- **THEN** its aqua brackets identify the focused command, including when focus was already present
- **AND** Return/Space and accessibility activation retain one-shot guarded delivery
- **WHEN** the user clicks with the pointer, including clicking an already focused command
- **THEN** the brackets are hidden while actual focus and keyboard eligibility are retained
- **AND** hover and mouse-down body feedback is unchanged, mouse movement alone does not clear keyboard focus visibility, and pointer-driven programmatic focus restoration does not reintroduce brackets

### Requirement: Command activation preserves binding authority
Migrated commands SHALL support pointer, Return, Space and applicable accessibility activation through the same eligible command path. One activation SHALL issue at most one command. Disabled, stale or destroyed targets SHALL NOT gain permission through a new input path. Existing command effects, target/session guards, persistent data and audio behavior SHALL remain unchanged.

#### Scenario: Keyboard or accessibility invokes a selected action
- **WHEN** an eligible migrated action is activated through Return, Space or its accessibility press action
- **THEN** it issues the same guarded command as pointer activation exactly once
- **AND** the existing target/session and asynchronous completion rules still determine acceptance

#### Scenario: Activation is no longer eligible before delivery
- **WHEN** a migrated action is disabled, destroyed or its captured context becomes invalid before queued delivery
- **THEN** no stale command mutates another target or session
- **AND** existing model rejection and reconciliation remain authoritative

### Requirement: Latching commands preserve accepted effective state
Latching command presentation SHALL use the shared command state language while distinguishing accepted effective on/off state. Programmatic refresh SHALL be silent and SHALL NOT rewrite preferences or issue another command. Requested state and temporarily overridden effective state SHALL remain distinct where the current binding distinguishes them.

#### Scenario: Effective latch state refreshes during interaction
- **WHEN** Snap or audition listening state refreshes from its accepted binding state
- **THEN** the latch displays the accepted effective state without another invocation
- **AND** hover, press, focus and disabled rendering follow its reviewed on/off matrix without changing requested preference semantics

### Requirement: Selector fields present local help and validation consistently
Migrated selector fields SHALL place smaller readable helper text directly beneath and aligned with the visible input body. Binding-supplied errors SHALL replace routine help with red validation text and a red body border, retained through hover and held states. Keyboard/accessibility brackets SHALL remain aqua and independent. Disabled inputs SHALL retain readable feedback. Presentation SHALL NOT introduce new validation or commit rules.

#### Scenario: Show and clear a field error
- **WHEN** a selector's binding supplies an error
- **THEN** the local message and body border turn red while any keyboard/accessibility brackets remain aqua
- **AND** the explanation wraps without shrinking and is exposed with the input's accessible description
- **WHEN** accepted binding refresh clears the error
- **THEN** normal helper text and body feedback return without an extra selection command

#### Scenario: Explain an unavailable field
- **WHEN** a selector is disabled because its prerequisite is absent
- **THEN** its small local explanation stays readable directly below the input, separately from the dimmed control
- **AND** disabled state alone is not presented as a validation error

### Requirement: Selectors separate body feedback from focus decoration
Migrated dropdown selectors SHALL provide distinguishable idle, hover, held and disabled body states with steady off-white text and chevrons. Keyboard and explicit accessibility focus SHALL use aqua outer corner brackets separated from the body border by a small gap. Pointer clicks SHALL hide focus decoration without removing actual focus or keyboard operation. Body and focus feedback SHALL remain independent.

#### Scenario: Compare mouse and keyboard states
- **WHEN** a selector is hovered or held with the pointer
- **THEN** its border and background provide the reviewed feedback without recoloring its chevron or showing pointer-induced focus brackets
- **WHEN** the selector receives keyboard or explicit accessibility focus, including while already focused
- **THEN** the outside aqua brackets appear without changing its accepted value or masking the applicable body state
- **AND** pointer movement alone does not remove keyboard focus decoration

#### Scenario: Disable a focused selector
- **WHEN** a selector becomes unavailable while focused or hovered
- **THEN** its disabled presentation and discoverable explanation communicate ineligibility and no input path changes its accepted value
- **AND** its focus ink remains contained and no mouse emphasis makes it appear enabled

### Requirement: Choice inspection is distinct from accepted selection
Migrated choice menus SHALL use the quiet row highlight for pointer or keyboard inspection, with distinct held feedback. Pointer highlight SHALL clear on leaving the row or menu. Keyboard-navigation highlight SHALL remain while its navigation focus remains, including after pointer hover leaves. Accepted selection SHALL use a checkmark independent of inspection; selection alone SHALL NOT retain a row background or edge highlight.

#### Scenario: Pointer leaves an inspected choice
- **WHEN** the pointer enters and then leaves a choice without keyboard navigation focus
- **THEN** the quiet background and edge appear only during hover and clear on leaving, including leaving into menu padding
- **AND** the accepted checkmark and value remain unchanged, including after pointer activation and subsequent departure

#### Scenario: Keyboard inspection survives pointer departure
- **WHEN** keyboard navigation reaches a choice and the pointer temporarily hovers another choice then leaves
- **THEN** hover feedback clears and the keyboard-navigation highlight remains on the actually focused choice
- **AND** no value is committed by inspection, pointer movement or opening the menu
- **AND** leaving keyboard navigation focus clears its highlight without removing the accepted checkmark

### Requirement: Choice content remains readable and unavailable reasons inspectable
Migrated selectors SHALL preserve full accessible choice names and accepted values. Closed long values SHALL ellipsize with discoverable full text; open long choices SHALL wrap within the available menu width without shrinking text or losing their checkmark. Unavailable choices SHALL expose a reason through visible and accessible presentation and support keyboard inspection without permission to select them. Entirely disabled fields SHALL explain their unavailability.

#### Scenario: Inspect a long or unavailable choice
- **WHEN** a long choice or unavailable choice is inspected at a supported size or user scale
- **THEN** the complete choice name and applicable reason are discoverable with pointer and keyboard access
- **AND** inspection or activation of an unavailable choice does not invoke a selection command or move the accepted checkmark

#### Scenario: No selected swell
- **WHEN** an IR field is disabled because no swell is selected
- **THEN** its visible and accessible explanation identifies that prerequisite while the control remains ineligible
- **AND** no missing-file, recovery or asset-relink operation is invented by the presentation

### Requirement: Selector input preserves deliberate guarded selection
Migrated selectors SHALL preserve stable choice identities, captured invocation context, accepted-state refresh and existing binding rejection. Wheel input SHALL NOT change selection and SHALL retain surrounding scrolling. Keyboard inspection, explicit commit, cancellation, popup anchoring and focus return SHALL be defined. One eligible commit SHALL issue at most one existing selection intent; disabled, stale or destroyed contexts SHALL NOT acquire authority.

#### Scenario: Inspect then commit or cancel
- **WHEN** a user opens a selector and navigates its choices
- **THEN** arrow navigation inspects choices without changing accepted state, Return or Space commits an eligible choice once, and Escape cancels without a command
- **AND** dismissal restores focus according to the initiating input method, without pointer-restored focus showing brackets
- **AND** Tab dismisses the popup and continues keyboard traversal without committing

#### Scenario: Refresh or context changes while a menu is open
- **WHEN** the catalogue reorders, the target/session/pass changes, the owner is destroyed, or a queued edit is rejected
- **THEN** the original captured choice identity and existing eligibility guards determine acceptance, never its new displayed index
- **AND** silent refresh and reconciliation issue no extra edit and preserve accepted model state
- **AND** Learn defaults, selected-swell IR and Export gain-pair semantics remain isolated and unchanged

### Requirement: Action menus separate pointer hover from keyboard inspection
Migrated action/context menus SHALL reuse quiet row feedback for neutral actions and distinguish held feedback from hover. Pointer highlight SHALL clear on leaving the row or menu. Keyboard-navigation highlight SHALL remain while its navigation focus remains and reappear after temporary pointer hover leaves. Ordinary actions SHALL NOT retain a selection checkmark, background or edge merely because they were invoked.

#### Scenario: Pointer leaves an action
- **WHEN** the pointer hovers Rename or a reset action and then leaves, including after a pointer click
- **THEN** pointer highlight clears without a lingering focused-row appearance or accepted-selection checkmark
- **AND** no action runs merely because a row is entered or left

#### Scenario: Keyboard inspection remains truthful
- **WHEN** keyboard navigation focuses an action and the pointer temporarily inspects another row then leaves
- **THEN** the keyboard-navigation highlight identifies the actually focused action until its navigation focus leaves or the menu is dismissed
- **AND** pointer input does not remove actual keyboard eligibility when it clears pointer-induced focus decoration

### Requirement: Action menu roles are explicit and readable
Migrated action menus SHALL declare semantic roles independently of labels and component names. Neutral rows SHALL retain off-white content with quiet feedback; destructive rows SHALL retain red content and red hover/held emphasis. Long action labels SHALL remain readable without shrinking. Unavailable actions SHALL expose visible and accessible reasons and permit keyboard inspection without permission to invoke them.

#### Scenario: Inspect Delete versus Rename
- **WHEN** Rename and Delete are hovered, held or keyboard-inspected
- **THEN** Rename uses quiet neutral feedback and Delete uses its explicit destructive red treatment
- **AND** changing either label does not select or remove its semantic role
- **AND** these popup roles do not reclassify existing command buttons

#### Scenario: Inspect an unavailable reset
- **WHEN** a reset is unavailable or its label/reason is long at a supported size or user scale
- **THEN** the complete binding-supplied action and explanation are visible or discoverable through pointer, keyboard and accessibility inspection
- **AND** no inspection or activation path grants permission to execute the unavailable action

### Requirement: Menu presentation preserves captured action authority
Migrated menus SHALL preserve existing action effects, captured target/session/input/pass authority, reset-revision guards, lifetime checks and asynchronous acceptance. One eligible pointer, keyboard or accessibility activation SHALL invoke at most one captured action. Cancellation or stale, disabled and destroyed contexts SHALL invoke none. Presentation SHALL NOT recalculate reset values, widen eligibility or replace action providers.

#### Scenario: Target changes while a reset menu is open
- **WHEN** selection, session, source/input, workflow pass or reset revision supersedes an open reset menu, or the owner is destroyed
- **THEN** queued activation cannot reset a replacement target or bypass existing rejection
- **AND** accepted values, saved state, assets and audio retain their existing behavior

#### Scenario: Invoke or cancel a context action
- **WHEN** an eligible action is activated through pointer, Return, Space or accessibility, or the menu is cancelled with Escape or outside dismissal
- **THEN** activation dispatches the original guarded action once and cancellation dispatches none
- **AND** menu anchoring, owner lifetime and input-appropriate focus return remain valid
- **AND** pointer-restored focus does not introduce keyboard/accessibility focus brackets

#### Scenario: Open a Swells item menu from either pointer entry point
- **WHEN** the user right-clicks a Swells item or clicks that item's three-dot icon
- **THEN** the same Rename/Delete menu opens for the clicked item at the pointer anchor

### Requirement: Numeric and text fields share presentation by editing role
Numeric and text fields SHALL use the approved appearance and accessible naming for their role across the plugin. Editable, unavailable and read-only states SHALL remain distinguishable. Keyboard and accessibility focus SHALL use separate aqua brackets. Pointer input SHALL suppress the brackets while preserving focus, caret and selection.

#### Scenario: Traverse editable and read-only fields
- **WHEN** a user tabs through inspector, Settings, dialog or Diagnostics fields
- **THEN** focus identifies the active field and its accessible name identifies its caption and applicable units
- **AND** read-only values remain available for inspection and copying according to their existing policy, without publishing an edit

#### Scenario: Edit a value embedded in a knob or slider
- **WHEN** the user clicks or types in the numeric entry
- **THEN** typing and selection remain usable with the same units and input constraints
- **AND** the entry retains its own focus treatment without changing the adjacent dial, track or stepper action

#### Scenario: Type into a numeric field
- **WHEN** a user types or pastes into a numeric field
- **THEN** the field blocks letters and preserves the selection when the entire insertion is blocked

#### Scenario: Scroll over a numeric stepper
- **WHEN** the user scrolls over the value or arrows of a numeric stepper
- **THEN** the field moves by one configured step in the wheel direction, within the existing range

### Requirement: Numeric and text field feedback follows the binding
Where a numeric or text field has a feedback position, it SHALL show its applicable help or binding-supplied validation directly below the field or group. The message SHALL wrap within that width and be available through the associated accessible description. Validation SHALL take precedence over routine help, with red text and an affected field border independent of focus.

#### Scenario: An inspector entry rejects a typed value
- **WHEN** a user submits invalid timing or an out-of-range Level in a field with a feedback position
- **THEN** the binding's explanation appears beside the affected field, the accepted value is preserved, and unrelated drafts remain intact
- **AND** an accepted correction or a change of the affected context clears the obsolete explanation

#### Scenario: A compact input has no local feedback position
- **WHEN** an embedded field has only its compact value entry
- **THEN** its existing units, context help and accessible description remain available without a new row or a new validation rule

#### Scenario: A long message wraps in a group
- **WHEN** help or validation is wider than the input area
- **THEN** the message wraps within the field or group's body bounds at the approved text size, and nearby controls remain readable and reachable

### Requirement: Numeric and text fields preserve subtype edit authority
Each field SHALL retain its subtype's edit and refresh rules, existing precision and clamp or reject behavior. The binding SHALL own the accepted value and captured edit context. Routine refresh SHALL issue no edit. Replacing the session or target SHALL prevent a stale draft from changing another value.

#### Scenario: Use the existing commit and cancel paths
- **WHEN** the user presses Return or Escape, changes focus or invokes an explicit submit action
- **THEN** the field follows its documented subtype behavior and commits at most once
- **AND** the Settings bar pair submits together through Return or its explicit action
- **AND** leaving the Settings bar pair or Name field runs validation while preserving explicit submission

#### Scenario: Change swell or session with a source-bar draft
- **WHEN** the selected swell or session changes before a source-bar draft is submitted
- **THEN** the stale draft cannot validate or publish in the new context
- **AND** the pair clears so the user can enter both bars for the new context

#### Scenario: A displayed value is rounded
- **WHEN** a numeric entry shows a rounded value and the user focuses or leaves it without editing
- **THEN** the accepted exact value is preserved
- **AND** a valid typed value follows the binding's existing precision and range rules

#### Scenario: Update one value in an independent pair
- **WHEN** the user edits Export Dry or Wet
- **THEN** only the edited level changes and the existing Character derivation remains authoritative

### Requirement: Empty Rename submissions remain recoverable
Rename SHALL keep its captured target and explicit OK, Return, Cancel and Escape behavior. An empty submission SHALL keep the dialog open with local validation until the user enters an accepted name or cancels.

#### Scenario: Submit an empty name then correct it
- **WHEN** the user submits an empty name using OK or Return
- **THEN** the original name remains accepted and the focused Name field shows the local reason
- **AND** an accepted correction renames the captured swell once, while Cancel or Escape closes without publishing a rename

#### Scenario: The Rename target is no longer current
- **WHEN** the captured swell or session ceases to be valid before submission
- **THEN** the existing ownership guard prevents publication to a different swell or session

### Requirement: Knob and slider feedback separates value, pointer and focus
Migrated knobs and slider tracks SHALL use an aqua value arc or fill, an off-white position indicator, and quiet pointer hover and held feedback. Keyboard and accessibility focus SHALL show thin outer aqua corner brackets with a gap outside the control body. Pointer focus SHALL retain interaction without the brackets. Unavailable controls SHALL remain readable and ineligible for edits.

#### Scenario: Compare the same control by pointer and keyboard
- **WHEN** the user hovers, holds, clicks or tabs to a dial or slider track
- **THEN** hover and held feedback identify pointer interaction, and only keyboard or accessibility focus shows the outer brackets
- **AND** the indicator continues to reflect the accepted value

#### Scenario: Hover or hold a dial at any value
- **WHEN** the user hovers over or holds a dial with the pointer
- **THEN** quiet feedback follows the arc's sweep and leaves its centre and the gap between its ends clear

#### Scenario: Focus a track that has a numeric field
- **WHEN** focus moves between a slider track, its value field or its steppers
- **THEN** focus feedback identifies the actual focused control, with the P4 field and stepper behavior preserved

#### Scenario: Resize a dial with a companion value field
- **WHEN** a dial with an exact-entry field is laid out at different widths
- **THEN** the field uses the shared value width wherever space allows
- **AND** inline feedback aligns beneath the field and wraps within its value column

#### Scenario: Discover the Target entry format
- **WHEN** the user hovers over the Target dial or field
- **THEN** the hover-help footer explains the current entry format, using bar.beat.fraction in musical mode and whole samples in exact-sample mode
- **AND** the accessible field description follows the same units, the format hint adds no height to the Timing row, and validation errors still appear beneath the field

### Requirement: Knob and slider wheel adjustment has a fine mode
Migrated dials and tracks SHALL define coarse and fine wheel steps in their domain. Ctrl+wheel SHALL use the fine step where a smaller legal step exists. A discrete control SHALL keep its minimum legal step. Wheel adjustment SHALL honor direction, range and eligibility. Wheel input over numeric steppers or outside the dial or track SHALL follow its existing policy.

#### Scenario: Fine-adjust Level or a percentage track
- **WHEN** the user compares a normal wheel step with Ctrl+wheel over Level, Learn Event sensitivity, Export Dry or Export Wet
- **THEN** Level steps by 1 dB or 0.1 dB, and the percentage tracks step by 1 or 0.1 percentage points respectively
- **AND** Ctrl on a continuous track does not change ordinary pointer drag

#### Scenario: Fine-adjust snapped timing
- **WHEN** a timing dial is eligible and Snap is on
- **THEN** plain wheel follows adjacent grid points, while Ctrl+wheel uses an unsnapped increment for that gesture
- **AND** the Snap setting, existing Ctrl-drag bypass and established Shift fine behavior remain available

### Requirement: A knob can have an optional drag-only home detent
A knob configured with a home value SHALL attract a drag to that value within its configured band. Continued movement beyond the band SHALL continue the drag. A knob without a home SHALL keep its existing drag behavior. A home detent SHALL apply only to pointer drag and SHALL preserve the control's independent Reset action.

#### Scenario: Drag Perform Level through neutral
- **WHEN** the user drags Level toward 0 dB from either direction
- **THEN** the dial settles at the marked 12 o'clock position when the drag candidate is within ±0.3 dB
- **AND** dragging beyond the band moves past the detent

#### Scenario: Enter a Level close to neutral using another input
- **WHEN** the user enters or steps to a valid Level within the detent band using the wheel, keyboard, accessibility or exact entry
- **THEN** the value follows that input method's normal rules
- **AND** the detent does not round the value to 0 dB

### Requirement: Knob and slider gestures preserve their existing authority
Migrated controls SHALL preserve captured invocation authority, value limits, supported saved values, and reset, cancel and refresh rules. Stale or unavailable gestures SHALL publish no edit. Display-only controls SHALL change only the presentation they own.

#### Scenario: Selection or session changes during a gesture
- **WHEN** a drag, wheel action or Reset loses its captured swell, session, timing or workflow authority
- **THEN** it cannot change a replacement target or session
- **AND** refresh displays the accepted state without publishing an edit

#### Scenario: Adjust a display control or reopen a legacy Level
- **WHEN** a user changes zoom or Height, or opens a project with an older saved Level
- **THEN** zoom and Height affect only the display, and the saved Level follows the established display and first-touch behavior

### Requirement: Boolean controls use presentation suited to their layout
Learn capture options and visible Settings on/off controls SHALL use labeled switches where the layout has room. The Swell Inspector SHALL use inline checkboxes for Enabled, Solo and Freeze. Each control SHALL display its accepted state with a thumb position or checkmark in addition to colour.

#### Scenario: Review spacious settings and compact inspector flags
- **WHEN** a user views Learn, Settings or the Swell Inspector at a supported editor size and scale
- **THEN** Learn and Settings switches keep their labels close to the track, and Enabled, Solo and Freeze stay together on one inspector row
- **AND** labels, hit targets and focus decoration remain readable, contained and independently operable
- **AND** choosing the checkbox presentation for an inspector flag preserves its immediate effect

### Requirement: Switches and checkboxes share independent pointer and focus states
Both forms SHALL distinguish quiet hover and held feedback on the indicator surface. Keyboard and accessibility focus SHALL show thin aqua corner brackets with a small gap around the indicator. A pointer click SHALL hide the brackets while retaining actual focus and keyboard eligibility.

#### Scenario: Hover and press an enabled control
- **WHEN** the user hovers, holds or releases the pointer on a switch or checkbox
- **THEN** the indicator's border and background show distinct hover and held states without obscuring its accepted value
- **AND** the label and indicator activate the same control, and cancelling the press before activation leaves the value unchanged
- **AND** a pointer release can activate the control only after an eligible primary press from the same pointer

#### Scenario: Change focus input method
- **WHEN** the user tabs to a control or explicitly focuses it through accessibility
- **THEN** the brackets appear independently of pointer feedback and without changing the value
- **AND** pointer movement alone leaves those brackets visible
- **WHEN** the user clicks the control, including when it already has focus
- **THEN** the pointer feedback remains available and the brackets disappear

#### Scenario: An on or off control becomes unavailable
- **WHEN** an on/off control is unavailable, including Isolate dry source during wet-only audition
- **THEN** its disabled presentation keeps its accepted state readable and exposes the binding's reason through the applicable help and accessible description
- **AND** hover or press does not make it appear eligible and activation does not change its value

### Requirement: Boolean activation preserves binding authority
Migrated switches and checkboxes SHALL expose a stable accessible name and state. Eligible pointer, Space and accessibility activation SHALL issue at most one existing guarded request. Existing Return behavior SHALL be preserved per binding. Programmatic refresh SHALL silently display accepted state. Disabled, stale or destroyed contexts SHALL not mutate a model.

#### Scenario: A Learn or Settings invocation outlives its capture pass
- **WHEN** the workflow or capture pass changes between invocation and delivery
- **THEN** the existing guard rejects the stale request, and the displayed value reconciles to accepted state without issuing another request
- **AND** a disabled pre-invocation hook that is required by the binding does not make the request eligible

#### Scenario: A selected swell changes while a flag is being activated
- **WHEN** a queued Enabled, Solo or Freeze activation loses its selected swell or session
- **THEN** it cannot change a replacement swell
- **AND** valid flag changes retain their existing selection, audition and render behavior

### Requirement: Editor and Settings use connected tabs

The workflow header, Learn sections, Perform Swell Inspector sections and Settings SHALL use joined tab bodies with aqua fill and readable content for the accepted selection. Hover and held press SHALL have distinct body feedback. Keyboard and explicit accessibility focus SHALL show thin aqua corner brackets separated from the tab body by a small gap.

#### Scenario: Hover and press either a selected or an unselected tab
- **WHEN** the pointer hovers over a tab or holds it down
- **THEN** quiet hover and stronger held feedback appear on its border and background
- **AND** the accepted tab remains recognizable until another selection is accepted
- **AND** adjacent tabs share one seam with legible selected and held edges
- **AND** releasing outside or leaving clears the relevant pointer feedback without selecting a tab

#### Scenario: Switch between keyboard and pointer input
- **WHEN** a tab gains keyboard or explicit accessibility focus
- **THEN** the aqua brackets appear around that tab independently of accepted selection and hover
- **AND** pointer movement alone leaves the brackets visible
- **AND** a pointer click on a tab hides the brackets while retaining actual focus and keyboard operation

#### Scenario: Use the tabs at compact or scaled sizes
- **WHEN** the editor uses its supported compact or larger layout, or Settings uses supported UI scaling
- **THEN** the labels, existing workflow icons, and complete focus decoration remain readable and clear of nearby content
- **AND** all Settings tabs, including Appearance when available, remain visible and reachable at the supported Settings size

### Requirement: Tab inspection and activation are separate

Connected tabs SHALL preserve accepted selection separately from focus and expose tab names, selected state, and availability to accessibility clients. Left and Right arrows and Home and End SHALL inspect choices. Return and Space SHALL request activation of an eligible inspected choice. Pointer or accessibility activation SHALL issue at most one request per invocation. Wheel input SHALL preserve selection and surrounding scrolling.

#### Scenario: Inspect a tab before switching pages
- **WHEN** a user enters the tab strip with the keyboard and navigates to a different tab
- **THEN** the inspected tab receives focus while the accepted tab and visible page remain unchanged
- **AND** assistive technology can query the inspected tab as the focused element independently of the selected tab
- **AND** Return or Space on an eligible tab requests its activation once
- **AND** the displayed selection and page change only after acceptance

#### Scenario: Inspect an unavailable workflow destination
- **WHEN** a workflow destination is unavailable or its existing guard rejects activation
- **THEN** the accepted mode and page remain unchanged
- **AND** an unavailable destination remains inspectable and exposes its existing reason through context help and accessibility

### Requirement: Settings keeps About last and preserves page bindings

Settings SHALL present Playback, Processing, Swell, Audition, Recovery, Appearance (when available), Storage and About in that order. About SHALL remain the last tab as the Settings strip evolves. Each tab and programmatic navigation action SHALL open its corresponding existing page independently of tab position.

#### Scenario: Visit Storage and About through the strip
- **WHEN** a user opens the final two Settings tabs
- **THEN** Storage opens the existing storage inventory and About opens the build information page
- **AND** the order is the same for pointer, keyboard and accessibility navigation

#### Scenario: Open Settings from the Storage shortcut
- **WHEN** an existing action opens Storage directly
- **THEN** Settings shows Storage and marks its tab selected
- **AND** ordinary refresh and resizing preserve the current page, its values and existing scroll and focus behavior
