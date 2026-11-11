# Spec Delta

## ADDED Requirements

### Requirement: Plugin dialogs provide shared presentation and keyboard access

Plugin-owned Rename, deletion and recovery dialogs SHALL reuse approved text,
command, field, feedback and focus presentation. Tab and Shift+Tab SHALL reach
their eligible controls in a coherent order. Return or Space SHALL activate
the eligible focused action according to its control contract. Initial focus
SHALL identify the editable Name field or a safe non-destructive dismissal.

#### Scenario: Open and submit Rename
- **WHEN** Rename opens for an eligible captured swell
- **THEN** the Name field receives focus with its current name selected
- **AND** its existing OK and Return submission, local validation, Cancel and Escape behavior remain available
- **AND** an empty submission stays in the dialog with its field error rather than closing or renaming

#### Scenario: Open a deletion or recovery warning
- **WHEN** a confirmed multi-delete or asset/metadata recovery warning opens
- **THEN** the initial focused action does not delete, locate, restore or otherwise start an operation
- **AND** deletion identifies the captured group and requires explicit confirmation
- **AND** recovery retains its existing messages and eligible Locate, Restore and Continue actions, with metadata-failure variants retaining their existing restrictions

### Requirement: Surface dismissal preserves cancellation semantics

Escape, an explicit cancellation action and title-bar close SHALL dismiss the
applicable plugin-owned dialog or auxiliary window without invoking its primary
action. Native-picker cancellation SHALL follow the platform's behavior.
Dismissal SHALL preserve existing field cancellation rules, accepted Settings
values and the separate authority of operations already running.

#### Scenario: Cancel a modal dialog or native picker
- **WHEN** the user cancels a pending dialog or picker, including through Escape or its window close affordance
- **THEN** the pending invocation publishes no rename, deletion, import, save, folder change or recovery operation
- **AND** later focus-loss or completion callbacks cannot turn that cancellation into acceptance
- **AND** dismissing an inner surface leaves its surviving parent surface open

#### Scenario: Close an auxiliary window while editing or working
- **WHEN** the user closes Settings or Diagnostics with Escape or title-bar close
- **THEN** that window hides, its active field follows its established cancel or focus-loss rules, and accepted values remain accepted
- **AND** Escape cannot submit a cancelled draft through a later focus-loss callback
- **AND** existing Storage hide-driven safe cancellation and guarded handoff behavior remain intact, without inventing operation acceptance or rollback

### Requirement: Closed surfaces return focus to a valid owner

After a user completes or dismisses a plugin-owned surface or native picker,
focus SHALL return to its surviving, visible and eligible opening control or a
stable visible owner fallback. Keyboard or accessibility return SHALL retain
visible focus decoration; pointer return SHALL retain actual focus without
adding it. Restoration SHALL respect parent lifetime and application activation.

#### Scenario: Return from a nested window or picker
- **WHEN** the user closes Diagnostics opened from Settings, or completes or cancels a picker opened from a field or Storage action
- **THEN** focus returns to that opening action in its still-visible owner
- **AND** keyboard navigation can continue without restarting at the editor's first control

#### Scenario: The original opening control no longer exists
- **WHEN** a virtualized Swell row, opening control or parent window disappears or becomes unavailable before return
- **THEN** a surviving visible list, owning window or editor provides the appropriate eligible fallback
- **AND** a hidden parent is not reopened and a destroyed editor receives no focus request

#### Scenario: An asynchronous result arrives in the background
- **WHEN** the owner has closed or the user has activated another application before an asynchronous result arrives
- **THEN** the result does not reactivate an obsolete window or steal foreground focus

### Requirement: File and folder selection retains native picker behavior

IR import, Export Save, Storage Backup/Restore/Locate and working-folder
selection SHALL use Windows-native file or folder pickers with their existing
selection modes, filters and overwrite behavior. Picker presentation SHALL
retain platform keyboard navigation and cancellation. Repeated activation
SHALL NOT create competing pickers for the same pending owner invocation.

#### Scenario: Choose or cancel an IR or Export file
- **WHEN** a user opens IR import or Save WAV from an eligible action
- **THEN** the existing file filter and open/save behavior are presented through the native picker
- **AND** cancellation publishes no operation while a valid result is delivered at most once through its existing guarded owner
- **AND** Save lifecycle integration leaves approved ready-file drag and cursor behavior unchanged

#### Scenario: Choose a Storage folder or backup receipt
- **WHEN** a Storage or working-folder action requires a destination, original asset folder or completed backup receipt
- **THEN** its existing folder or receipt-file selection mode remains intact
- **AND** choosing an arbitrary folder does not replace a required exact-receipt selection
- **AND** cancellation leaves the accepted folder, receipt and protected assets unchanged

### Requirement: Surface completion retains captured operation authority

Dialog and picker results SHALL retain their captured target, session, pass,
revision, request and lifetime guards. An obsolete or duplicate result SHALL
NOT edit a replacement context. Presentation and focus handling SHALL NOT
grant operation eligibility, replace validation, discard protected assets or
change the existing authority for cancellation of background work.

#### Scenario: A target or session changes before acceptance
- **WHEN** Rename, deletion, IR import, Export Save or a Storage selection completes after its captured context becomes stale
- **THEN** the existing guard rejects the obsolete result without changing a replacement swell, source, export or project folder
- **AND** the rejection and return-focus path do not publish a second request

#### Scenario: The editor is destroyed before completion
- **WHEN** a dialog, native picker or scheduled focus return outlives its owning editor
- **THEN** its continuation is safely discarded and cannot access the destroyed owner
- **AND** captured source, IRs, render assets and saved project identities retain their established protections

### Requirement: Auxiliary windows remain accessible at supported size and scale

Settings and Diagnostics SHALL retain comfortable text and one user-scale
transform within the current display work area. Scrolling SHALL keep content
and focused controls reachable without clipping focus decoration. Closing and
reopening SHALL preserve accepted page state and existing bindings; ordinary
reflow SHALL retain scroll position where it still fits.

#### Scenario: Navigate a compact or magnified auxiliary window
- **WHEN** the user tabs forward or backward through long Settings or Diagnostics content at 100% or 125% user scale in constrained bounds
- **THEN** scrolling reveals the actual focused control and its full focus decoration
- **AND** content is scaled once, text remains readable, and the native close affordance remains reachable within the work area
- **AND** Settings keeps its connected tabs with About last

#### Scenario: Reflow and reopen without publishing edits
- **WHEN** content height, supported window bounds or user scale changes, or an auxiliary window is closed and reopened
- **THEN** accepted page and model state remain under their existing owners without new edits or duplicate windows
- **AND** reflow preserves the reading position, clamping only when the available scroll range becomes shorter
