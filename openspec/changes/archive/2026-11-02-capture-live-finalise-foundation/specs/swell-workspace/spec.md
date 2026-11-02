# Spec Delta

## MODIFIED Requirements

### Requirement: Workspace reflects actual processing and selection
The foundation workspace SHALL retain the existing eight-region Live composition while adding mode selection/status without a dedicated Capture/Finalise redesign. Capture/Finalise SHALL show mode-appropriate read-only content and lock unavailable creative actions. Selection/processing SHALL remain identity-bound and SHALL NOT imply a requested render or mode is already ready.

#### Scenario: Select while background work completes
- **WHEN** the user selects a swell while allowed analysis or another swell's rendering completes
- **THEN** its row, handles and inspector remain bound to that identity and show actual IR/state, with editing available only under mode guards

#### Scenario: Empty or unavailable selection
- **WHEN** the session is empty or a result is waiting, stale or failed
- **THEN** the workspace explains its state and enables only valid mode/selection actions without false ready waveforms

#### Scenario: Stable empty and loaded composition
- **WHEN** the same editor size transitions through empty, mapped, selected and updating states in this foundation
- **THEN** existing region bounds/dividers remain coherent, no developer toolbar displaces them and mode/readiness text does not promise the deferred workspace/export implementation

### Requirement: Both sensitivity controls remain global
Marker sensitivity SHALL remain a saved instance preference independent of selection and Recapture. The prior Regeneration sensitivity value SHALL be preserved for compatibility but its implicit source-overwrite control SHALL be disabled/retired with an explanation. Mode/Recapture SHALL exclusively govern source admission; no replacement behavior SHALL be hidden behind either sensitivity.

#### Scenario: Two distinct sensitivities
- **WHEN** an older project restores either sensitivity value and the user changes marker sensitivity in an allowed mode
- **THEN** marker selection policy remains independent, the saved tolerance value is retained without authorizing source overwrite, and selected-swell edits remain unchanged

## ADDED Requirements

### Requirement: Mode selector explains eligibility and transition locks
The existing editor SHALL provide Capture, Live and Finalise selection with concise roles/status and unavailable reasons. Recording/Stop settlement SHALL lock mode selection. Capture readiness SHALL be distinguished from pending wet work; no export implementation or unverified host Play/Stop action SHALL be exposed by this foundation.

#### Scenario: Stop is settling
- **WHEN** a capture pass is draining or validating
- **THEN** the mode control explains the lock, source remains provisional and no manual switch bypasses it
- **AND** successful useful source acceptance automatically selects Live even with wet jobs still pending

#### Scenario: Insufficient material
- **WHEN** there is no useful compatible source or no enabled prepared swell
- **THEN** Live/Finalise availability reflects the distinct prerequisites and explains what is missing

### Requirement: Mode guards apply below disabled UI
Capture SHALL reject creative editing and manual audition; Finalise SHALL reject creative editing while allowing only separately initiated supported preview. Live SHALL permit the current editor/audition capabilities under existing validity rules. Canvas, numeric fields, keyboard, Settings, Diagnostics and async chooser completions SHALL all obey the same mode authority.

#### Scenario: Delayed file chooser changes mode
- **WHEN** an IR chooser opened in Live completes after Finalise is entered
- **THEN** the edit is rejected/cancelled with preserved definitions/assets rather than bypassing the read-only mode

#### Scenario: Keyboard or recovery bypass attempt
- **WHEN** an edit, generation or legacy mapping command is requested through a secondary path in a disallowing mode
- **THEN** the model rejects the disallowed action, preserves state and provides the same reason as the visible control

#### Scenario: Explicit preview versus host output
- **WHEN** supported internal preview is explicitly started in Finalise
- **THEN** it is clearly identified separately from dry-only DAW output and does not enable capture or host-scheduled wet

### Requirement: Recapture status is independent and honest
An explicitly labelled Recapture existing audio control SHALL show its armed/off/settling/failure state independently of Automatic Swells. Legacy mapping controls SHALL route through workflow guards or be unavailable with an explanation. Saved mode and toggles SHALL NOT silently overwrite source or enable another policy.

#### Scenario: Return to Capture
- **WHEN** the user returns from Live to Capture with existing mapped material
- **THEN** protected coverage remains visible, Recapture remains off unless explicitly armed and the next ordinary playback fills unmapped positions only

#### Scenario: Successful recapture finishes
- **WHEN** a useful armed pass commits successfully on Stop
- **THEN** Recapture shows off, Live is selected and background rendering status remains visible without claiming all swells ready

### Requirement: Foundation visuals require native and host review
Mode selection, guarded controls and source/settlement status SHALL remain usable at 1100x850 and existing larger supported sizes. Native evidence SHALL exercise empty, recording, settling, Live, Finalise and failure states. User visual review SHALL precede final FL Studio acceptance; untested host/DPI results SHALL remain open.

#### Scenario: Review the minimal mode integration
- **WHEN** the foundation UI is ready for review
- **THEN** actual native captures and binding/guard tests show readable controls and no overlap, with a user checkpoint before broader host validation
