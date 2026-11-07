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
