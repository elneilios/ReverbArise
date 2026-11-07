# Spec Delta

## ADDED Requirements

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
