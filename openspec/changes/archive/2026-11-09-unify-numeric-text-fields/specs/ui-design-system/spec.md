# Spec Delta

## ADDED Requirements

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
