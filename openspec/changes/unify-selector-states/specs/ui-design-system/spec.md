# Spec Delta

## ADDED Requirements

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
