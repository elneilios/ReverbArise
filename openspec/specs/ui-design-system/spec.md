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
