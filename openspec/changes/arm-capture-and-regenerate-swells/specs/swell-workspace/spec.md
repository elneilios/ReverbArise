# Spec Delta

## MODIFIED Requirements

### Requirement: Learn Default IR choices use accepted managed identities

Learn Sound SHALL present bundled presets and Custom IR. The contextual IR File row SHALL expose retained managed choices and supported import. Pending intent SHALL remain distinct from the accepted default. Cancelled, failed or obsolete imports SHALL preserve the prior default and existing swell assignments.

#### Scenario: Choose a default before playback
- **WHEN** the user selects a bundled preset or accepts a managed response or mono/stereo WAV/WavPack import while idle
- **THEN** it becomes the accepted future Sound with its stable managed identity and is used for new eligible swells from the next pass

#### Scenario: Cancel or fail a file import
- **WHEN** the picker is cancelled or decoding/validation fails
- **THEN** the prior accepted default remains selected and usable, failure is explained at the IR File row, and no existing assignment changes

#### Scenario: Playback begins during a pending import
- **WHEN** Arm locks the pass before a requested default is accepted
- **THEN** the pass uses the displayed previously accepted default, late completion cannot replace it, and any deferred or discarded request is disclosed

### Requirement: Learn provides the approved two-column capture composition
Learn SHALL show the existing branded header/mode navigation, narrow left capture settings and one large right dry timeline. The Swells list, selected-swell inspector, audition band and duplicate Overview SHALL be hidden. Layout SHALL follow approved mockups and the flat visual system at the existing opening/minimum size and supported larger sizes.

#### Scenario: Fresh learning workspace
- **WHEN** a fresh instance opens in Learn
- **THEN** the timeline prompts the user to arm capture and supplies the selected mode's timing guidance, the two independent control groups are visible and unrelated editing/export controls do not occupy the workspace

#### Scenario: Return with source already present
- **WHEN** a user returns to Learn from Perform
- **THEN** accepted source and full-song context remain visible, Arm is off, and the Perform editor state is retained for return

#### Scenario: Compact settings and keyboard focus
- **WHEN** Learn is shown at the supported minimum or larger size and the user tabs through its controls
- **THEN** matching section headings, smaller consistent field labels and tightly grouped descriptions remain top-aligned; both right-aligned switches and all other interactive controls retain complete focus outlines inside their bounds without touching the column separator
- **AND** the timeline heading, caption, ruler and status remain aligned without overlap, preserving the approved header and dominant dry canvas

#### Scenario: Hover beside a selected workflow mode
- **WHEN** a mode segment is selected and the pointer hovers an adjacent segment
- **THEN** its entire hover border remains visible at the shared edge without changing selection, square joins, rounded outer ends, keyboard focus or accessibility order

### Requirement: Learn generation controls are locked for a whole pass
Automatic Swells, Default IR/file assignment and Marker Sensitivity SHALL be immutable from capture Arm through cancellation or settlement. Visible controls and every command/completion path SHALL enforce the same pass lock, with an explanation. After settlement, later detector edits SHALL asynchronously update candidates over accepted coverage without generating; current Default IR SHALL apply to the next explicit or eligible Stop generation snapshot without rewriting accepted pass context.

#### Scenario: Active gesture or chooser crosses Play
- **WHEN** a gesture, typed edit, preset action, chooser or import completes after the pass lock begins
- **THEN** it cannot alter that pass's settings, cannot bypass disabled controls and reports cancellation/deferred next-pass intent truthfully

#### Scenario: A later default is chosen while old generation is pending
- **WHEN** a user changes an allowed future default after source acceptance while earlier wet work remains queued
- **THEN** the new choice is shown, the earlier capture context remains unchanged, any pending Generate with the old default is cancelled and the next Generate uses the new accepted default

### Requirement: Learn offers one explicit Generate action
When Detect events is enabled, Learn SHALL show one shared primary generation button in a fixed lower action area on both tabs. The taller button SHALL centre the swell icon and full label together, using larger semibold text. One centred helper line SHALL sit below it. Footer hover help and accessibility SHALL explain which swells a rebuild replaces. Auto-generate on Stop SHALL remain a switch in the capture settings.

#### Scenario: Ready or disabled action
- **WHEN** Learn is ready, capturing, settling, analysing, generating or missing usable source/IR
- **THEN** Generate reflects canonical readiness, keyboard/pointer/secondary routes share the same guards and disabled reasons remain readable without a rejected click
- **AND** the enabled button shows “Place swells manually in Perform” below it, while a disabled or busy button uses the same line for its status

#### Scenario: Cancel a waiting automatic request
- **WHEN** the user disables Auto-generate on Stop while its request awaits final analysis or preparation
- **THEN** the not-yet-committed request is cancelled and no lock hides the cancellation path

### Requirement: Manual placement guidance remains readable
Learn SHALL show “Place swells manually in Perform” on one line using the existing helper role. The helper SHALL remain in the shared action area when Detect events is off. When generation is disabled or busy with Detect events on, the same line SHALL show the current status.

#### Scenario: Automatic Swells off and editor reopen
- **WHEN** Automatic Swells is disabled in an open editor or before reopening it
- **THEN** the manual placement helper remains visible while the generation button is hidden

#### Scenario: Minimum-size busy workspace
- **WHEN** generation is disabled or busy at the minimum supported editor size
- **THEN** the status occupies a single line, the full reason is available in footer hover help and accessibility, and tab settings remain reachable

## ADDED Requirements

### Requirement: Learn tabs share a visible content gutter
Learn SHALL align the visible tabs, capture headings and leading controls, and Sound Preset and contextual IR File rows on a shared left gutter. Focus decoration SHALL have room outside the visible control edges.

#### Scenario: Compact Learn sidebar alignment
- **WHEN** the musician switches tabs with Loop Mode on or off at the compact size and a supported 100% or 125% UI scale
- **THEN** the visible leading edges remain aligned and the IR File selector and Import button retain their spacing and keyboard focus decoration

### Requirement: Learn presents one Arm control for Song and Loop
The Capture & Generate tab SHALL place a shared Arm button and adjoining capture state field at the top of Source Capture. The field SHALL show waiting, recording, finishing, settling and idle or unavailable status in one row. Full status SHALL be available through hover help and accessibility. Recapture Audio SHALL remain a separate replacement permission.

#### Scenario: Switch the next capture mode while idle
- **WHEN** a user changes between Song and Loop
- **THEN** Arm stays in the same place and invokes the selected capture route with its timing and source permission
- **AND** keyboard focus, activation and state feedback follow the shared latch policy

### Requirement: The Learn generation button communicates rebuild intent
The action SHALL show Generate Swells before generation and Regenerate Swells when an arrangement or prior automatic deletions exist. Both labels SHALL invoke the same explicit rebuild policy. Its accessible name SHALL match its visible label and the icon SHALL support recognition alongside the text.

#### Scenario: Compact Learn with the taller action
- **WHEN** the Capture & Generate and Sound tabs are reviewed at the compact supported window size at 100% and 125% UI scale where available
- **THEN** the same button and helper stay in place and remain visible across tabs, with the full label, icon and focus decoration fitting while tab settings remain reachable
- **AND** hovering the button displays the rebuild explanation in the Learn footer
- **AND** disabled, keyboard-focused and busy states retain their readable status and shared interaction policy
