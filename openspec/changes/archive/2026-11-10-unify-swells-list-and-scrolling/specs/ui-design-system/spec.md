# Spec Delta

## MODIFIED Requirements

### Requirement: Choice inspection is distinct from accepted selection
Migrated choice menus SHALL use the quiet row highlight for eligible pointer targets and for keyboard inspection, with distinct held feedback. Pointer highlight SHALL clear on leaving the row or menu. Keyboard-navigation highlight SHALL remain while its navigation focus remains, including after pointer hover leaves. Accepted selection SHALL use a checkmark independent of inspection; selection alone SHALL NOT retain a row background or edge highlight.

#### Scenario: Pointer leaves an inspected choice
- **WHEN** the pointer enters and then leaves an eligible choice without keyboard navigation focus
- **THEN** the quiet background and edge appear only during hover and clear on leaving, including leaving into menu padding
- **AND** the accepted checkmark and value remain unchanged, including after pointer activation and subsequent departure

#### Scenario: Keyboard inspection survives pointer departure
- **WHEN** keyboard navigation reaches a choice and the pointer temporarily hovers another choice then leaves
- **THEN** hover feedback clears and the keyboard-navigation highlight remains on the actually focused choice
- **AND** no value is committed by inspection, pointer movement or opening the menu
- **AND** leaving keyboard navigation focus clears its highlight without removing the accepted checkmark

#### Scenario: Pointer crosses an unavailable dropdown choice
- **WHEN** the pointer moves from an available choice onto an unavailable choice or clicks the unavailable choice
- **THEN** the prior pointer highlight clears and the unavailable choice receives no pointer hover or held highlight
- **AND** keyboard inspection remains available, the unavailable reason stays discoverable and the accepted checkmark and value remain unchanged

### Requirement: Action menus separate pointer hover from keyboard inspection
Migrated action/context menus SHALL reuse quiet row feedback for eligible neutral actions and distinguish held feedback from hover. Pointer highlight SHALL clear on leaving the row or menu. Keyboard-navigation highlight SHALL remain while its navigation focus remains and reappear after temporary pointer hover leaves. Ordinary actions SHALL NOT retain a selection checkmark, background or edge merely because they were invoked.

#### Scenario: Pointer leaves an action
- **WHEN** the pointer hovers Rename or a reset action and then leaves, including after a pointer click
- **THEN** pointer highlight clears without a lingering focused-row appearance or accepted-selection checkmark
- **AND** no action runs merely because a row is entered or left

#### Scenario: Keyboard inspection remains truthful
- **WHEN** keyboard navigation focuses an action and the pointer temporarily inspects another row then leaves
- **THEN** the keyboard-navigation highlight identifies the actually focused action until its navigation focus leaves or the menu is dismissed
- **AND** pointer input does not remove actual keyboard eligibility when it clears pointer-induced focus decoration

#### Scenario: Pointer crosses a disabled action
- **WHEN** the pointer moves from an eligible action onto a disabled action or clicks the disabled action
- **THEN** the prior pointer highlight clears and the disabled action receives no pointer hover or held highlight
- **AND** keyboard inspection remains available and no action runs

### Requirement: Action menu roles are explicit and readable
Migrated action menus SHALL declare semantic roles independently of labels and component names. Neutral rows SHALL retain off-white content with quiet feedback; destructive rows SHALL retain red content and red hover/held emphasis. Long action labels SHALL remain readable without shrinking. Unavailable actions SHALL permit keyboard inspection without activation. Reasons SHALL be visible and accessible where needed. Disabled Rename during multiselect SHALL show its label alone.

#### Scenario: Inspect Delete versus Rename
- **WHEN** eligible Rename and Delete actions are hovered, held or keyboard-inspected
- **THEN** Rename uses quiet neutral feedback and Delete uses its explicit destructive red treatment
- **AND** changing either label does not select or remove its semantic role
- **AND** these popup roles do not reclassify existing command buttons

#### Scenario: Inspect an unavailable reset
- **WHEN** a reset is unavailable or its label/reason is long at a supported size or user scale
- **THEN** the complete binding-supplied action and explanation are visible or discoverable through pointer, keyboard and accessibility inspection
- **AND** no inspection or activation path grants permission to execute the unavailable action

#### Scenario: Inspect Rename during multiselect
- **WHEN** the Swells menu opens while multiple swells are selected
- **THEN** Rename is disabled with its dimmed label alone and its unavailable state is exposed accessibly

### Requirement: Menu presentation preserves captured action authority
Migrated menus SHALL preserve existing action effects, captured target/session/input/pass authority, reset-revision guards, lifetime checks and asynchronous acceptance. One eligible pointer, keyboard or accessibility activation SHALL invoke at most one captured action, including a request for confirmation. Cancellation or stale, disabled and destroyed contexts SHALL invoke none. Presentation SHALL NOT recalculate reset values, widen eligibility or replace action providers.

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
- **THEN** the same Rename/Delete menu opens from either entry point with actions scoped to the clicked row and its selection membership

## ADDED Requirements

### Requirement: Popup highlights fill the available row
Shared action and choice popups SHALL fill the available row width for eligible hover and held feedback and for keyboard inspection. A scrollbar gutter SHALL be reserved only when rows overflow.

#### Scenario: Highlight short and scrolling popups
- **WHEN** a user inspects a row in a short popup or in a popup with overflowing content
- **THEN** the highlight reaches the inner right edge in the short popup and the visible row edge in the scrolling popup
- **AND** text, selection markers and the scrollbar remain readable and usable
