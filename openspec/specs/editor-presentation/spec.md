# editor-presentation Specification

## Purpose

Provide a complete, readable compact editor that expands responsively and keeps every supported action discoverable across Learn, Perform and Export, independently of musical state and host display scaling.

## Requirements

### Requirement: Compact composition is the baseline
With no valid saved presentation preference, the editor SHALL open at 900x600 logical pixels at 100% UI scale. All three modes SHALL support that composition using the owner-approved responsive typography and usable interaction targets, without a separate compact workflow. The minimum content size at 100% UI scale SHALL be 900x600.

#### Scenario: First opening and complete mode journey
- **WHEN** a user opens a fresh presentation profile and visits Learn, Perform and Export with suitable session material
- **THEN** the editor opens at 900x600, each mode retains its established major regions, and its supported actions are visible or have obvious labelled access points
- **AND** warnings, disabled reasons, normal-sized help and exact-value alternatives remain accessible

### Requirement: Larger windows add useful workspace
Resizing above the compact baseline SHALL increase useful waveform or panel space, preserve readable control geometry and use measured responsive breakpoints where groups no longer fit. Expansion SHALL NOT uniformly enlarge controls, change workflow or hide existing capabilities. Intermediate widths and heights SHALL remain operable.

#### Scenario: Resize through a breakpoint
- **WHEN** a user grows and shrinks the editor through a navigation or panel breakpoint
- **THEN** groups reflow without overlap, lost focus or unintended edits, and waveform coordinates remain aligned
- **AND** larger windows provide more editing or visible panel space rather than proportionally larger padding

### Requirement: Overflow is discoverable and keyboard accessible
Each scrollable or sectioned settings surface SHALL visibly indicate additional content and provide mouse and keyboard access. Common actions SHALL remain visible or have persistent labelled entry points. Focus traversal SHALL reveal the complete focused control and its focus treatment without covering pinned actions.

#### Scenario: Reach an initially offscreen control
- **WHEN** content exceeds a panel or a user tabs to a control outside its current viewport
- **THEN** visible scrolling or labelled section access discloses the content, and focus reveals the intended control without editing its value
- **AND** wheel input outside controls scrolls normally, while controls retain their established deliberate wheel policies

#### Scenario: Learn and Export at compact height
- **WHEN** Automatic Swells is enabled in Learn or a ready, pending or failed composition is shown in Export at 900x600
- **THEN** Generate and its readiness explanation, or playback and file actions respectively, remain visible or directly accessible through labelled controls without clipped content
- **AND** capture state, manual-placement guidance and delivery readiness remain truthful

### Requirement: Window size and UI scale are independent
The editor SHALL distinguish workspace size from proportional UI scale and waveform zoom. It SHALL provide named size presets including the 900x600 baseline, free resizing, and validated UI scale choices including 100% and 125% when they fit. Scale SHALL apply consistently to controls, text, input coordinates and owned auxiliary UI without double-applying host or Windows scaling.

#### Scenario: Change workspace size or UI scale
- **WHEN** a user changes a size preset, freely resizes, or selects an available UI scale
- **THEN** sizing adjusts the available layout area, scaling adjusts apparent control size, and neither changes the musical timeline viewport as an editing intent or alters audio settings
- **AND** menus, file-choice invocation, exact editing, focus and waveform hit anchors remain coherent

#### Scenario: Unvalidated shrink choice
- **WHEN** a scale below 100% has not passed scoped readability and interaction review
- **THEN** it is not offered as a supported choice and is not used automatically to make the baseline fit

### Requirement: Presentation preferences survive safely
Valid window size and UI scale SHALL survive editor close/reopen and application restart as local user presentation preferences. Changes SHALL affect only the invoking open editor; later editors SHALL use the retained preference. Missing, corrupt or out-of-range preferences SHALL fall back safely without affecting DAW project state, source assets or musical preferences.

#### Scenario: Multiple instances and restart
- **WHEN** one instance changes size or scale while another editor is open, then editors are reopened or the application restarts
- **THEN** the other open editor does not resize unexpectedly and later openings use the valid retained local preference
- **AND** presentation changes do not dirty musical state, autoplay, recapture, render, or modify plugin/parameter/asset identities

#### Scenario: Missing or malformed preference storage
- **WHEN** storage is absent, invalid, partially written or unavailable
- **THEN** usable in-memory defaults remain available, invalid fields fall back independently, and a presentation failure cannot invalidate or discard a valid project

### Requirement: Screen fit remains recoverable
Sizing and scaling choices SHALL account for the current display work area and host/window chrome. Choices known not to fit SHALL be unavailable with an explanation. Reopening or changing displays SHALL recover an oversized preference to a fitting supported composition without silently reducing typography or offering a clipped workspace as supported.

#### Scenario: Reopen on a smaller display
- **WHEN** saved size or scale no longer fits the current work area but 900x600 at 100% does
- **THEN** the editor restores a fitting size and scale, keeps Settings accessible and preserves all musical state

#### Scenario: Display cannot fit the supported baseline
- **WHEN** the display or host embedding cannot fit even the supported baseline
- **THEN** the limitation is explained through accessible presentation feedback and recovery controls remain reachable where the host permits
- **AND** the combination is recorded as unsupported rather than claimed as a passing compact layout

### Requirement: Resizing and scaling preserve truthful rendering
At supported sizes and scales, text, vector artwork, waveforms, cursors and edit handles SHALL remain sharp and aligned. Changing size, scale, display or mode SHALL invalidate stale presentation layers as needed without changing source timing, render dependencies, accepted audio or saved edits.

#### Scenario: Change scale with cached waveform content
- **WHEN** a selected swell is displayed, navigated, resized, scaled and moved between supported display scales
- **THEN** waveform layers refresh at the appropriate resolution and hit testing, ruler, source bounds and envelope handles agree on canonical positions
- **AND** source/render identities and audio job counts remain unchanged by presentation alone

### Requirement: Typography responds to available workspace
Text SHALL follow the approved compact-to-expanded role scale using both logical width and height, independently of user magnification. At 900x600 the heading/navigation/body/value roles SHALL use 16/14/12/11 heights; at 1100x850, 18/16/13/12; at 1400x1000, 21/18/14/13. Intermediate sizes SHALL interpolate continuously within these bounds without changing field precision or shrinking hit targets.

#### Scenario: Grow a compact editor
- **WHEN** the user resizes through compact, intermediate and expanded dimensions
- **THEN** text follows the bounded role scale, shorter dimensions constrain growth, focus and fields fit, and edits/source/render identities remain unchanged

### Requirement: Compact composition preserves comfortable clearances
At the compact baseline the UI SHALL retain 12-pixel outer horizontal margins and at least 8-pixel clearances between header/workspace, inspector content/column edges, tabs/content, Sound content/reset actions and reset actions/the lower divider. Exact-entry text SHALL retain horizontal insets; controls and focus SHALL not collide with separators.

#### Scenario: Inspect dense compact content
- **WHEN** current controls, long labels and focused entries are shown at 900x600
- **THEN** measured content and focus stay inside their assigned regions with the named clearances, and every supported action remains discoverable

#### Scenario: Perpendicular dividers remain separated
- **WHEN** Learn, Perform or Export bands and columns are painted at compact or expanded sizes
- **THEN** vertical dividers stop at least 8 logical pixels short of perpendicular horizontal rules, using the named divider-clearance policy, without touching T or cross junctions

