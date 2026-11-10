# Spec Delta

## ADDED Requirements

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
