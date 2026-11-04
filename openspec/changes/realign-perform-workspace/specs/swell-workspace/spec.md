# Spec Delta

## MODIFIED Requirements

### Requirement: Workspace reflects actual processing and selection
The workspace SHALL adapt to mode: Learn shows capture settings and one full-song dry timeline; Perform shows the production three-column editor, audition and compact Overview; Export shows its dedicated preview and mix/file controls. Processing and selection SHALL remain identity-bound, truthful and mode-appropriate. A requested render or mode SHALL NOT be shown as already ready.

#### Scenario: Select while background work completes
- **WHEN** the user selects a swell in Perform while allowed analysis or another swell's rendering completes
- **THEN** its row, handles and inspector remain bound to that identity and show actual IR/state, with editing available only under mode guards

#### Scenario: Empty or unavailable selection
- **WHEN** the session is empty or a result is waiting, stale or failed
- **THEN** the active workspace explains its state and enables only valid mode/selection actions without false ready waveforms or current-WAV claims

#### Scenario: Stable empty and loaded composition
- **WHEN** the same editor size transitions through empty, mapped, selected and updating states within a mode
- **THEN** its region bounds/dividers remain coherent and no developer toolbar displaces them or promises a deferred export capability

#### Scenario: Return to Perform
- **WHEN** a prepared user leaves Learn for Perform
- **THEN** the working swell list, selected inspector, editor navigation, audition and compact Overview return without losing definitions, selection or edits

#### Scenario: Dedicated production Perform
- **WHEN** Perform is displayed at supported sizes
- **THEN** the chronological Swells list is left, real aligned dry/wet editor is central and Selected Swell inspector is right, with audition and one separate compact Overview below
- **AND** shared header, Settings, reserved notices, production Inter/semantic roles and existing editing capabilities remain usable without changing Learn or Export composition

### Requirement: Marker controls are prominent and instance scoped
Marker sensitivity SHALL remain one saved instance value independent of selection. Learn SHALL expose a horizontal slider and typeable 0-100% value within enabled Automatic Swells settings. Perform SHALL omit the obsolete Auto markers and sensitivity tooling. Low means more selective marking. Saved-off policy SHALL remain effective and truthful; changing presentation SHALL NOT reset preferences.

#### Scenario: Tune automatic markers from the main view
- **WHEN** the user adjusts/types the idle Learn slider
- **THEN** the canonical percentage updates coherently without resetting selected envelopes, IRs, blend or monitoring preferences
- **AND** Learn changes retain their current future-pass meaning without introducing a generation workflow through this composition change

#### Scenario: Disable marking without losing the workflow
- **WHEN** Automatic Swells is off in Learn
- **THEN** generation controls are hidden, dry capture remains available and automatic pass generation is disabled
- **AND WHEN** the user enters Perform
- **THEN** existing swells and manual editing remain usable without a marker-control band or changes to saved marking preferences

#### Scenario: An explicit master policy prevents marking
- **WHEN** legacy saved master/marker preferences disagree or automatic processing is disabled
- **THEN** Learn does not falsely show effective automatic generation as enabled or silently enable a saved-off policy
- **AND** an explicit user enable action has a documented consistent effect on the underlying marking/processing policy

#### Scenario: Resize and restore marker controls
- **WHEN** the editor is resized across supported sizes, Diagnostics is toggled or saved state reopens
- **THEN** Learn's visible sensitivity control remains reachable and synchronized with saved state, Perform retains no obsolete marker band, and selection does not change the instance value

## ADDED Requirements

### Requirement: Perform compact composition requires native acceptance
Perform SHALL use owner-approved scoped compact spacing with invariant production typography, semantic ink, focus and hit targets. Dense, sparse, empty, pending and failed states SHALL remain readable and bounded. New or materially revised control families SHALL receive native look/feel approval before wider rollout.

#### Scenario: Review compact spacing
- **WHEN** a compact Perform composition is proposed
- **THEN** a directly launched native preview supplies actual row, panel, gap, lane and focus measurements for owner approval before production adoption
- **AND** shared comfortable Learn/header consumers retain their accepted geometry

#### Scenario: Long content and resizing
- **WHEN** many swells, long names/IRs, existing multiple selection and empty/loading/error states are shown at 1100x850, 1400x900, 1400x1000, 1600x1000 and width breakpoints at software 100/125/150/200%
- **THEN** labels, exact fields, focus and actions do not overlap or clip; supported truncation provides full tooltip/accessibility detail and inspector scrolling preserves every supported edit
- **AND** native owner acceptance, software scale, actual Windows DPI and FL Studio coverage are recorded separately

#### Scenario: Composite divider continuity
- **WHEN** the complete native Perform composition is painted with its actual child components
- **THEN** horizontal rules remain continuous across the available width at the header, audition, Overview and information boundaries, with column content inset and vertical rules separated from those horizontal rules

### Requirement: Perform toolbar provides horizontal Zoom and contextual Fit
Perform SHALL provide one inline row containing Zoom minus, horizontal zoom slider, Zoom plus, Fit, Snap toggle button and rhythmic-grid selector, replacing Prev/Next and page-span controls. Zoom SHALL change only horizontal view scale and synchronize the real dry/wet, ruler, handles and Overview viewport. Display-amplitude scaling SHALL remain separately disclosed.

#### Scenario: Zoom extremes and anchors
- **WHEN** buttons, slider or pointer-wheel navigation reach either zoom extreme
- **THEN** the viewport remains finite and bounded, with stable pointer/selected-target/playhead context and all time layers aligned
- **AND** source PCM, swell positions, selection, gains, render identities and job counts remain unchanged

#### Scenario: Fit selected and unselected material
- **WHEN** Fit is activated with a selected swell, including pending wet or overlap
- **THEN** the view includes its actual source, requested pre-roll/arrival/end and available render-tail context without fabricated audio
- **AND WHEN** no swell is selected
- **THEN** Fit shows available captured extent with truthful sparse gaps, or a finite empty view

#### Scenario: Initial entry and return
- **WHEN** Perform first receives usable source without a saved view
- **THEN** it Fits once without waiting for all wet renders
- **AND WHEN** selection, background publication or mode return occurs after user navigation
- **THEN** the user view is retained until explicit Fit or navigation, without host seeking or repeated auto-fit

### Requirement: Snap preference and temporary bypass are independent
Snap SHALL be a latching toggle button, not a checkbox. Its remembered On/Off preference SHALL be independent of the remembered rhythmic grid. Ctrl SHALL temporarily bypass snap and show the button off without changing either preference. Shift SHALL independently enable existing fine-drag; Ctrl+Shift SHALL combine both behaviors.

#### Scenario: Hold and release Ctrl
- **WHEN** Snap is remembered On and Ctrl is pressed with or without an active drag
- **THEN** effective snapping and the toggle button's visible/accessibility pressed state become Off while held
- **AND WHEN** Ctrl is released
- **THEN** both return On without a saved-preference change, host dirty notification or key-only handle movement

#### Scenario: Saved Off and combined modifiers
- **WHEN** Snap is remembered Off and Ctrl is pressed/released
- **THEN** it remains Off and the grid choice is retained
- **AND WHEN** Ctrl+Shift is held during a timing drag
- **THEN** movement is unsnapped and fine; Shift alone remains fine with the remembered snap policy, and Ctrl alone bypasses snap with normal drag precision

#### Scenario: Modifier lifecycle and existing shortcuts
- **WHEN** modifier keys change during a gesture, editor focus leaves/returns, or Ctrl-wheel zoom or existing list-selection shortcuts are used
- **THEN** button state and effective snap reconcile with actual Ctrl state without a stuck bypass, another target's edit, an unintended saved toggle or broken existing shortcuts
- **AND** key transitions alone do not move handles; subsequent pointer movement uses the effective policy without an artificial drag-baseline jump

### Requirement: Rhythmic snapping uses trustworthy host coordinates
Snap SHALL offer explicit 1/16 beat, 1/8 beat, 1/4 beat, 1 beat and 1/2/4 bar divisions. Beat units SHALL follow the reported meter denominator and bars its numerator. Quantization SHALL use validated absolute host PPQ origin with nearest-grid rounding and checked sample conversion, never display names, pixels or a fabricated tempo map.

#### Scenario: Meter and fractional PPQ
- **WHEN** trustworthy compatible timing reports 4/4, 3/4 or 6/8 with fractional PPQ or negative lead-in
- **THEN** each division quantizes to its absolute host grid with deterministic half ties away from PPQ zero and at most integer-sample rounding error
- **AND** unchanged-origin seeks/loops retain that mapping without changing captured positions

#### Scenario: Missing or changed mapping
- **WHEN** host timing is absent, non-finite, overflowing, capture-rate incompatible or no longer matches the accepted fixed tempo/meter/origin
- **THEN** effective Snap On prevents horizontal snapped publication and explains unavailable timing rather than guessing
- **AND** Snap Off or Ctrl bypass permits valid unsnapped sample edits while navigation and vertical editing remain usable

#### Scenario: Timing changes during a drag
- **WHEN** a drag's session, selection, mode or validated mapping changes
- **THEN** further publication from that obsolete gesture is cancelled with prior accepted musical/source state preserved and an understandable reason

### Requirement: Horizontal handle snapping preserves exact musical edits
Effective Snap SHALL quantize horizontal target, source-bound and editable envelope-time gestures to one absolute grid. Vertical amplitude/tension SHALL remain continuous. Whole-wet translation SHALL preserve internal spacing. Exact numeric entry SHALL remain unsnapped. Every accepted edit SHALL preserve ordering, source PCM, unrelated identities and existing model guards.

#### Scenario: Endpoints and fades with wet placement
- **WHEN** an arrival, source boundary, wet start/end or fade boundary is dragged with nonzero placement and Snap On
- **THEN** its absolute displayed timing lands on a valid grid point and agrees with the exact inspector value through canonical validation
- **AND** a domain with no valid grid point rejects the edit instead of claiming an off-grid clamp is snapped

#### Scenario: Vertical and translated edits
- **WHEN** a tension/amplitude handle is moved vertically or the wet body is translated horizontally
- **THEN** vertical values remain continuous, while translated snap anchors move the complete wet shape without quantizing its internal durations or moving dry PCM

#### Scenario: Fine and exact alternatives
- **WHEN** Ctrl bypass, Shift fine-drag, combined Ctrl+Shift, exact numeric input or keyboard alternatives are used with overlapping handles or zero fades
- **THEN** the effective timing policy is consistent, hit anchors and accepted values agree, and no unrelated parameter, captured source or selected identity changes

#### Scenario: Inspector timing knobs follow the grid
- **WHEN** Target, Source Start/End, Move Wet, Pre-roll, End Offset or Fade In/Out is adjusted by knob drag or wheel with effective Snap On
- **THEN** the corresponding absolute timeline anchor snaps through the same validated host grid and canonical bounds as the canvas, with Move Wet preserving internal spacing
- **AND** Ctrl bypasses snapping, Shift retains fine drag and Ctrl+Shift combines both without key-only movement or modifier-induced jumps
- **AND** unavailable timing or a domain without a valid grid point rejects snapped publication with a reason; accepted dial/value/canvas feedback agrees
- **AND** typed values remain exact and unsnapped, curve and level knobs remain continuous, and stale session/selection/mode/source/timing intents cannot publish

### Requirement: Perform navigation preferences survive reopen safely
Perform SHALL retain valid view, requested Snap state and grid across editor reopen, mode return and project save/reopen, independently per instance. Legacy or malformed optional navigation values SHALL fall back safely without discarding valid musical state. Held modifiers and active gestures SHALL remain ephemeral; restore SHALL not autoplay, recapture or regenerate audio.

#### Scenario: Reopen current and legacy state
- **WHEN** a project with user zoom and Snap Off is reopened, or a legacy state lacks navigation preferences
- **THEN** valid preferences return, while legacy uses contextual Fit, Snap On and quarter-beat grid without changing source/render/IR references, IDs or existing parameters

#### Scenario: Save during temporary bypass
- **WHEN** a project is saved while Ctrl temporarily masks remembered Snap On
- **THEN** reopen retains the remembered On/grid preferences and has no stuck bypass or restored gesture

#### Scenario: Invalid preferences and copied instances
- **WHEN** optional view/grid values are malformed or an instance is copied/restored
- **THEN** invalid UI fields alone default safely, valid musical content survives, and subsequent navigation does not change another instance
