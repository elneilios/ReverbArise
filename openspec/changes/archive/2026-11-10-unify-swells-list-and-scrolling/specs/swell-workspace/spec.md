# Spec Delta

## ADDED Requirements

### Requirement: Swells row actions support pointer, keyboard and accessibility
Swells rows SHALL expose named Enabled and More actions for their swell. The Enabled indicator SHALL be square. Keyboard and accessibility focus SHALL use the shared aqua brackets. Row action navigation and activation SHALL preserve the selected identities unless the requested action itself changes them.

#### Scenario: Traverse a long list and compact inspector
- **WHEN** a user navigates the list by keyboard and Tabs through the primary swell's Enabled and More actions
- **THEN** Up/Down retains list selection behavior, Tab reaches those two actions and continues into the inspector, and Return or Space activates the focused action
- **AND** an off-screen inspector field scrolls into view with its focus treatment without editing its value
- **AND** explicit list selection keeps the existing contextual waveform reveal while Tab traversal and background refresh do not replay it

#### Scenario: Invoke a row action by identity
- **WHEN** a user operates the Enabled checkbox or opens the same row menu by its three dots, right-click, keyboard or accessibility
- **THEN** the action names the swell it targets and remains bound to its captured identity across reorder or row recycling
- **AND** pointer focus keeps its native ownership with brackets suppressed
- **AND** presentation and focus changes alone issue no command or selection edit

### Requirement: Swells menu confirms deletion of a captured selection
Delete from a selected row with multiple swells selected SHALL target the entire captured selection and require confirmation. Delete from an unselected row SHALL target only that row. Rename SHALL be unavailable while multiple swells are selected. Deletion SHALL keep the existing session and edit guards and SHALL either remove the confirmed group or leave it intact.

#### Scenario: Confirm or cancel a group delete
- **WHEN** the user opens the menu on a selected row and chooses Delete for multiple swells
- **THEN** the menu and confirmation show the count, the confirmation opens with Cancel focused, and only explicit confirmation removes the captured swells
- **AND** Cancel, Escape and closing the confirmation leave the swells, selection and scroll context intact

#### Scenario: Open the menu outside the selection
- **WHEN** the user opens the menu on an unselected row while other rows are selected
- **THEN** Delete targets only the invoked row and the existing selection is preserved
- **AND** Rename remains disabled while the multiselect is active

#### Scenario: The captured group becomes invalid
- **WHEN** the list reorders or the selection changes while a menu or confirmation is open
- **THEN** the action can only delete the originally captured IDs
- **AND** a missing target, obsolete session, invalid edit context or deletion capacity failure rejects the whole group without mutating any swell
