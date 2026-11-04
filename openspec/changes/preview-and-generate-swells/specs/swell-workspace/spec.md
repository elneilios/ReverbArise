# Spec Delta

## MODIFIED Requirements

### Requirement: Learn exposes independent progressive capture controls
Learn SHALL separate Source Capture/Recapture from Automatic Swells. Automatic Swells off SHALL hide IR, Character, DSP, sensitivity, Auto-generate on Stop and Generate controls while retaining their values and explaining dry-only learning. On SHALL expose real supported generation settings and explicit Generate, with a retained Auto-generate on Stop choice. Unsupported Character algorithms/parameters SHALL be absent rather than selectable placeholders.

#### Scenario: Dry-only controls
- **WHEN** Automatic Swells is switched off while Learn is idle
- **THEN** dry capture and Recapture stay independently available, generation preferences hide, and switching on again restores their accepted values

#### Scenario: Character processing is unavailable
- **WHEN** the build has no implemented Character processing
- **THEN** no inactive/fictitious Character choices or parameter controls appear, and a bundled static IR is not presented as a Character algorithm

### Requirement: Learn generation controls are locked for a whole pass
Automatic Swells, Default IR/file assignment and Marker Sensitivity SHALL be immutable from valid DAW Play through Stop drain/validation. Visible controls and every command/completion path SHALL enforce the same pass lock, with an explanation. After settlement, later detector edits SHALL asynchronously update candidates over accepted coverage without generating; current Default IR SHALL apply to the next explicit or eligible Stop generation snapshot without rewriting accepted pass context.

#### Scenario: Active gesture or chooser crosses Play
- **WHEN** a gesture, typed edit, preset action, chooser or import completes after the pass lock begins
- **THEN** it cannot alter that pass's settings, cannot bypass disabled controls and reports cancellation/deferred next-pass intent truthfully

#### Scenario: A later default is chosen while old generation is pending
- **WHEN** a user changes an allowed future default after source acceptance while earlier wet work remains queued
- **THEN** the new choice is shown, the earlier capture context remains unchanged, any pending Generate with the old default is cancelled and the next Generate uses the new accepted default

### Requirement: Mode guards apply below disabled UI
Capture SHALL reject ordinary creative editing and manual audition, with the explicit guarded Learn Generate transaction as its sole new definition-commit exception; Finalise SHALL reject creative editing while allowing only separately initiated supported preview. Live SHALL permit the current editor/audition capabilities under existing validity rules. Canvas, numeric fields, keyboard, Settings, Diagnostics and async chooser completions SHALL all obey the same mode authority.

#### Scenario: Delayed file chooser changes mode
- **WHEN** an IR chooser opened in Live completes after Finalise is entered
- **THEN** the edit is rejected/cancelled with preserved definitions/assets rather than bypassing the read-only mode

#### Scenario: Keyboard or recovery bypass attempt
- **WHEN** an edit, generation or legacy mapping command is requested through a secondary path in a disallowing mode
- **THEN** the model rejects the disallowed action, preserves state and provides the same reason as the visible control

#### Scenario: Explicit preview versus host output
- **WHEN** supported internal preview is explicitly started in Finalise
- **THEN** it is clearly identified separately from dry-only DAW output and does not enable capture or host-scheduled wet

## ADDED Requirements

### Requirement: Learn offers one explicit Generate action
Generate Swells SHALL be the primary action in Automatic Swells directly below Marker Sensitivity and any supported conditional parameters. It SHALL reuse the shared button family, retain the same label on repeated runs and expose accessible progress/unavailable reasons from accepted model readiness. Auto-generate on Stop SHALL use the shared switch family without competing navigation or a new card.

#### Scenario: Ready or disabled action
- **WHEN** Learn is ready, capturing, settling, analysing, generating or missing usable source/IR
- **THEN** Generate reflects canonical readiness, keyboard/pointer/secondary routes share the same guards and disabled reasons remain readable without a rejected click

#### Scenario: Cancel a waiting automatic request
- **WHEN** the user disables Auto-generate on Stop while its request awaits final analysis or preparation
- **THEN** the not-yet-committed request is cancelled and no lock hides the cancellation path

### Requirement: Manual placement guidance remains readable
A subdued helper directly beneath Generate SHALL read “Want to place swells yourself? Add manual swells in Perform.” It SHALL use existing embedded Inter helper typography and stay visible when Generate is disabled. Supported minimum and larger sizes SHALL preserve readable wrapping, focus bounds and dominant dry-waveform space without shrinking text or adding a competing button.

#### Scenario: Minimum-size busy workspace
- **WHEN** Automatic Swells is ON at 1100x850 and generation is disabled or busy
- **THEN** the full helper remains legible beneath Generate, all settings remain reachable and NoticeBar does not move the workspace

### Requirement: Candidate markers have truthful visual identity
The Learn dry timeline SHALL show sample-aligned candidate arrivals distinctly from committed swells, white inspection pin, solid yellow DAW cursor and provisional source. Timing-yellow candidate heads/dashed stems and status SHALL distinguish settled, provisional and updating results. Overlay refresh SHALL be read-only, bounded at display resolution and preserve actual coverage, gaps and unavailable data.

#### Scenario: Inspect sparse and dense source
- **WHEN** markers are viewed at full-song scale, resized, zoomed or software-scaled with silence, gaps, unavailable ranges and negative pre-roll
- **THEN** marker coordinates remain absolute and truthful, cursor roles stay distinguishable and provisional/stale markers cannot imply settled generation readiness

#### Scenario: Return from Perform
- **WHEN** the user returns to Learn with accepted source and committed swells
- **THEN** current candidate status appears without duplicate definitions, recapture, DSP work from inspection or implicit generation

### Requirement: Generation outcomes use existing feedback and review gates
Candidate counts and progress SHALL use restrained Learn status text; actionable empty/protected-only/no-change/failure outcomes SHALL use the reserved accessible NoticeBar without layout movement. Revised marker presentation and any new control variant SHALL receive scoped native owner review before rollout. UI standards, component/binding checks and actual host/DPI coverage SHALL be recorded separately.

#### Scenario: Unsuccessful Generate
- **WHEN** Generate cannot publish a changed viable result
- **THEN** Learn remains selected, the preserved-work outcome is explained without claiming success and repeated refresh does not repeatedly announce the same notice

#### Scenario: Native acceptance
- **WHEN** the integrated controls and overlay are ready for owner review
- **THEN** one prepared native preview is launched directly with keyboard, disabled, busy and dense-marker states; any required token exception is reviewed before rollout and untested host/DPI coverage remains explicit
