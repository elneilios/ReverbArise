# Spec Delta

## MODIFIED Requirements

### Requirement: Workspace reflects actual processing and selection
The workspace SHALL adapt to mode: Learn shows capture settings and one full-song dry timeline; Perform retains its eight-region editor; Export retains its currently supported guarded content until its dedicated workspace is delivered. Processing and selection SHALL remain identity-bound, truthful and mode-appropriate. A requested render or mode SHALL NOT be shown as already ready.

#### Scenario: Select while background work completes
- **WHEN** the user selects a swell in Perform while allowed analysis or another swell's rendering completes
- **THEN** its row, handles and inspector remain bound to that identity and show actual IR/state, with editing available only under mode guards

#### Scenario: Empty or unavailable selection
- **WHEN** the session is empty or a result is waiting, stale or failed
- **THEN** the active workspace explains its state and enables only valid mode/selection actions without false ready waveforms

#### Scenario: Stable empty and loaded composition
- **WHEN** the same editor size transitions through empty, mapped, selected and updating states within a mode
- **THEN** its region bounds/dividers remain coherent and no developer toolbar displaces them or promises a deferred export capability

#### Scenario: Return to Perform
- **WHEN** a prepared user leaves Learn for Perform
- **THEN** the working swell list, selected inspector, editor navigation, audition and compact Overview return without losing definitions, selection or edits

### Requirement: Marker controls are prominent and instance scoped
Marker sensitivity SHALL remain one saved instance value independent of selection. Learn SHALL expose a horizontal slider and typeable 0-100% value within enabled Automatic Swells settings; Perform SHALL retain its prominent rotary knob and Auto markers switch. Low means more selective marking. Saved-off policy SHALL remain effective and truthful; changing representation SHALL NOT reset other preferences.

#### Scenario: Tune automatic markers from the main view
- **WHEN** the user turns/types the Perform knob or adjusts/types the idle Learn slider
- **THEN** the canonical percentage updates coherently without resetting selected envelopes, IRs, blend or monitoring preferences
- **AND** Learn changes configure a future pass while explicit permitted Perform reanalysis retains its existing meaning

#### Scenario: Disable marking without losing the workflow
- **WHEN** Auto markers is off in Perform
- **THEN** marking is visibly Off, saved sensitivity remains editable, existing swells remain usable and permitted manual actions/render refresh remain independent
- **AND WHEN** Automatic Swells is off in Learn
- **THEN** generation controls are hidden, dry capture remains available and automatic pass generation is disabled

#### Scenario: An explicit master policy prevents marking
- **WHEN** legacy saved master/marker preferences disagree or automatic processing is disabled
- **THEN** Learn does not falsely show effective automatic generation as enabled or silently enable a saved-off policy
- **AND** an explicit user enable action has a documented consistent effect on the underlying marking/processing policy

#### Scenario: Resize and restore marker controls
- **WHEN** the editor is resized across supported sizes, Diagnostics is toggled or saved state reopens
- **THEN** each mode's visible sensitivity control is reachable and synchronized with saved state, while selection changes do not change its value

### Requirement: Overview has one truthful waveform viewport
Perform SHALL retain its compact Overview with ruler, real dry/wet summaries and one detail-viewport highlight. Learn SHALL use one large full-song dry Overview instead of a duplicate strip, with origin-anchored extent, explicit coverage states and no wet-envelope editing. Both views SHALL use actual absolute positions, disclose display scaling and avoid seeking the host through viewport navigation.

#### Scenario: Sparse map and viewport
- **WHEN** two captured sections with mapped silence are separated by an uncaptured gap and Perform detail navigation changes
- **THEN** the gap contains no invented waveform, silence remains recognizable, ruler/highlight agree with actual bounds and navigation affects only the editor

#### Scenario: Learn has one source view
- **WHEN** Learn displays a sparse capture
- **THEN** its large timeline is the full-song Overview, no second bottom Overview or wet lane appears, and all sections/gaps stay at their absolute positions

#### Scenario: Truly empty instance
- **WHEN** Perform has neither captured audio nor a selected swell
- **THEN** Swells (0), No swells yet, Press play to begin mapping your song, No swell selected and No audio mapped yet appear in its stable regions
- **AND** Learn uses its mode-appropriate DAW Play prompt; neither mode shows fake data or forced import/generation, and saved-off capture/generation policy is explained rather than silently enabled

## ADDED Requirements

### Requirement: Learn provides the approved two-column capture composition
Learn SHALL show the existing branded header/mode navigation, narrow left capture settings and one large right dry timeline. The Swells list, selected-swell inspector, audition band and duplicate Overview SHALL be hidden. Layout SHALL follow approved mockups and the flat visual system at the existing opening/minimum size and supported larger sizes.

#### Scenario: Fresh learning workspace
- **WHEN** a fresh instance opens in Learn
- **THEN** the timeline prompts DAW Play, the two independent control groups are visible and unrelated editing/export controls do not occupy the workspace

#### Scenario: Return with source already present
- **WHEN** a user returns to Learn from Perform
- **THEN** accepted source and full-song context remain visible, Recapture is off unless explicitly armed, and the Perform editor state is retained for return

#### Scenario: Compact settings and keyboard focus
- **WHEN** Learn is shown at the supported minimum or larger size and the user tabs through its controls
- **THEN** matching section headings, smaller consistent field labels and tightly grouped descriptions remain top-aligned; both right-aligned switches and all other interactive controls retain complete focus outlines inside their bounds without touching the column separator
- **AND** the timeline heading, caption, ruler and status remain aligned without overlap, preserving the approved header and dominant dry canvas

#### Scenario: Hover beside a selected workflow mode
- **WHEN** a mode segment is selected and the pointer hovers an adjacent segment
- **THEN** its entire hover border remains visible at the shared edge without changing selection, square joins, rounded outer ends, keyboard focus or accessibility order

### Requirement: Learn exposes independent progressive capture controls
Learn SHALL separate Source Capture/Recapture from Automatic Swells. Automatic Swells off SHALL hide IR, Character, DSP and sensitivity controls while retaining their values and explaining dry-only learning. On SHALL expose real supported generation settings. Unsupported Character algorithms/parameters SHALL be absent rather than selectable placeholders.

#### Scenario: Dry-only controls
- **WHEN** Automatic Swells is switched off while Learn is idle
- **THEN** dry capture and Recapture stay independently available, generation preferences hide, and switching on again restores their accepted values

#### Scenario: Character processing is unavailable
- **WHEN** the build has no implemented Character processing
- **THEN** no inactive/fictitious Character choices or parameter controls appear, and a bundled static IR is not presented as a Character algorithm

### Requirement: Learn generation controls are locked for a whole pass
Automatic Swells, Default IR/file assignment and Marker Sensitivity SHALL be immutable from valid DAW Play through Stop drain/validation. Visible controls and every command/completion path SHALL enforce the same pass lock, with an explanation. After settlement, later edits SHALL affect future passes without modifying the accepted pass context.

#### Scenario: Active gesture or chooser crosses Play
- **WHEN** a gesture, typed edit, preset action, chooser or import completes after the pass lock begins
- **THEN** it cannot alter that pass's settings, cannot bypass disabled controls and reports cancellation/deferred next-pass intent truthfully

#### Scenario: A later default is chosen while old generation is pending
- **WHEN** a user changes an allowed future default after source acceptance while earlier wet work remains queued
- **THEN** the new choice is shown for future learning and the earlier pass retains its snapshotted settings

### Requirement: Learn Default IR choices use accepted managed identities
The Learn Default IR selector SHALL offer all real bundled responses, its current custom assignment and Open from File using the supported importer. Pending intent SHALL be distinguished from the accepted default. Cancelled, failed or obsolete imports SHALL preserve the prior default; assignments to existing swells SHALL remain unchanged.

#### Scenario: Choose a default before playback
- **WHEN** the user selects a bundled response or successfully imports a mono/stereo WAV/WavPack while idle
- **THEN** it becomes the accepted future default with its stable managed identity and is used for new eligible swells from the next pass

#### Scenario: Cancel or fail a file import
- **WHEN** the picker is cancelled or decoding/validation fails
- **THEN** the prior accepted default remains selected/usable, failure is explained, and no existing assignment changes

#### Scenario: Playback begins during a pending import
- **WHEN** DAW Play locks the pass before a requested default is accepted
- **THEN** the pass uses the displayed previously accepted default, late completion cannot replace it, and any deferred/discarded request is disclosed

### Requirement: IR pickers share the instance's retained imported choices
Learn and Perform IR pickers SHALL include bundled IRs and every successfully accepted imported managed IR in this instance, deduplicated by content identity. Imported choices SHALL remain available when unused or deselected and across save/reopen. Reuse SHALL verify managed assets without requiring the original external file. Failed, cancelled and superseded imports SHALL NOT enter the collection. The collection SHALL be instance-local rather than a scanned or global library.

#### Scenario: Reuse an unselected learning import in Perform
- **WHEN** a custom learning IR is successfully imported, subsequently deselected everywhere, and the user edits another swell's IR
- **THEN** that custom IR remains selectable and selecting it changes only the intended assignment

#### Scenario: Reopen or copy an instance
- **WHEN** the instance is saved/restored or copied with an unused successful import
- **THEN** the imported choice survives without duplicates and subsequent imports into one instance do not change another instance's choices

### Requirement: Learn coverage can be understood without waveform-height guessing
Learn SHALL explicitly distinguish captured audio, exact-zero silence, uncaptured gaps, unavailable assets and provisional recording. A concise key and inspectable range/status text SHALL supplement colour and waveform height. Capture totals SHALL distinguish mapped duration, observed extent and provisional progress without inventing a known song length.

#### Scenario: Quiet audio beside silence and a gap
- **WHEN** source includes a quiet release tail, exact-zero stored samples, an uncaptured interval and an unavailable range
- **THEN** each has the correct labelled/inspectable state; a flat-looking tail is not relabelled silence and unavailable data is not relabelled empty

#### Scenario: Provisional recapture is cancelled
- **WHEN** incoming replacement peaks are shown and the pass is cancelled or fails
- **THEN** the provisional indication disappears, prior accepted source stays visible and no recording progress is represented as committed coverage

### Requirement: Learn auto-fit retains full dry-source context stably
The Learn viewport SHALL include song origin and the highest accepted/admitted provisional source position, with coalesced threshold-based expansion and modest headroom. Sparse seeks SHALL retain actual gaps. Loops, selection, wet lead-in and same-extent Recapture SHALL NOT cause repeated rescaling; Learn and Perform navigation state SHALL remain independent.

#### Scenario: Capture late and early sections
- **WHEN** playback captures bars 86-90, then seeks to bars 1-4
- **THEN** the Learn view includes origin through the late section with a real gap, and the backward seek does not continually shrink/refit it

#### Scenario: Recapture without extending coverage
- **WHEN** an armed pass revisits already known extent
- **THEN** the scale stays stable while the real provisional/accepted content changes

#### Scenario: Timing is unavailable
- **WHEN** musical timing or asset compatibility is unavailable
- **THEN** the ruler/status uses truthful available coordinates and does not invent bars, stretch old source or hide unavailable regions

### Requirement: Learn inspection is read-only, persistent across focus and ephemeral
Learn SHALL provide read-only absolute-sample inspection. Left-click SHALL pin/relocate across focus changes; right-click SHALL NOT pin. Waveform-focused Escape SHALL clear coordinate/pin together. Inspection SHALL stay enabled during capture, remain ephemeral and leave capture, generation and Perform unchanged. Project/session reset or editor reopen SHALL clear it. Inspection, transport and provisional indicators SHALL be distinguishable.

#### Scenario: Relocate and retain a pin
- **WHEN** a left-click pins a position, focus moves to a setting, the pointer moves or capture advances
- **THEN** the same absolute inspection position and its visible marker remain, and another left-click relocates it
- **AND** right-click does not create/relocate the pin and Escape outside waveform focus does not clear it

#### Scenario: Inspect an active or settling pass
- **WHEN** hover, pin relocation or waveform keyboard inspection occurs while recording/draining/validating
- **THEN** inspection remains responsive without changing capture authority, accepted source/context, job counts, saved state or Perform selection

#### Scenario: Reset inspection consistently
- **WHEN** Learn is re-entered or the editor's project/instance/session is replaced, restored or cleared
- **THEN** coordinate and pinned flag clear together, hover resumes and no marker is restored from project state
- **AND** ordinary auto-fit expansion and DAW seeks preserve the absolute pin without changing viewport policy or seeking the host

### Requirement: Learn inspection readout uses reliable musical timing
The existing inspection row SHALL show exact sample and coverage classification, plus bar/beat and elapsed time from song origin only with reliable coherent host timing and compatible capture rate. Missing/incompatible timing SHALL be explicitly unavailable. Full coverage ranges SHALL remain accessible without new controls or fabricated coordinates.

#### Scenario: Inspect with and without reliable timing
- **WHEN** an inspected sample has valid coherent host timing and a compatible capture rate
- **THEN** its bar/beat and elapsed time from song origin are shown with exact sample and truthful audio/silence/gap/unavailable/provisional classification
- **AND WHEN** that timing is unavailable or incompatible
- **THEN** exact sample/coverage remain available and musical/elapsed timing is explicitly unavailable rather than fabricated
