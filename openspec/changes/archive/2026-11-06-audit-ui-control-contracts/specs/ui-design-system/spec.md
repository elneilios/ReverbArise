# Spec Delta

## ADDED Requirements

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
