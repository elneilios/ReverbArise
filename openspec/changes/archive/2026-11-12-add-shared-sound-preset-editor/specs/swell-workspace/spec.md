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
- **WHEN** DAW Play locks the pass before a requested default is accepted
- **THEN** the pass uses the displayed previously accepted default, late completion cannot replace it, and any deferred or discarded request is disclosed

### Requirement: IR pickers share the instance's retained imported choices

Both Custom IR file selectors SHALL offer every successfully accepted managed import in the instance, deduplicated within the managed collection by content identity. A file matching bundled PCM SHALL remain selectable as Custom IR. Retained choices SHALL survive deselection and reopen, require verified managed assets, and exclude failed, cancelled and superseded imports. The collection SHALL remain instance-local.

#### Scenario: Reuse an unselected learning import in Perform
- **WHEN** a custom learning IR is successfully imported, subsequently deselected everywhere, and the user edits another swell's Sound
- **THEN** that custom IR remains selectable without the original external file and selecting it changes only the intended assignment

#### Scenario: Reopen or copy an instance
- **WHEN** the instance is saved and restored or copied with an unused successful import
- **THEN** the imported choice survives without duplicates and subsequent imports into one instance do not change another instance's choices

### Requirement: Learn presents a concise truthful waveform preview

Learn SHALL use a shorter width-proportional waveform band with unchanged time mapping and unscaled PCM min/max representation. It SHALL distinguish Detect events from Generate Swells, show the future assignment in Sound and consolidate capture state, duration and event count. Exact samples, coverage and silence classifications SHALL remain in existing Diagnostics. The manual hint SHALL remain visible with detection off.

#### Scenario: Inspect audio and diagnostics
- **WHEN** Learn is resized or inspected during capture, settled preview or detection OFF
- **THEN** waveform amplitude ratios, gaps and musical timing remain accurate, concise status stays readable and existing Diagnostics retains exact technical inspection

#### Scenario: Inspect sparse and dense source
- **WHEN** markers are viewed at full-song scale, resized, zoomed or software-scaled with silence, gaps, unavailable ranges and negative pre-roll
- **THEN** marker coordinates remain absolute and truthful, cursor roles stay distinguishable and provisional or stale markers cannot imply settled generation readiness

#### Scenario: Return from Perform
- **WHEN** the user returns to Learn with accepted source and committed swells
- **THEN** current candidate status appears without duplicate definitions, recapture, DSP work from inspection or implicit generation

### Requirement: Learn separates capture and sound access

Learn SHALL retain its dry canvas, left settings column and Capture & Generate and Sound tabs. Capture policies and sensitivity SHALL stay in the first tab. The shared editor SHALL expose the future Sound assignment and editable Level in Sound. Generate, readiness and manual-placement guidance SHALL remain visible below either tab. Unsupported DSP controls SHALL remain absent.

#### Scenario: Inspect defaults and return to capture
- **WHEN** the user switches Learn tabs during allowed or locked workflow states
- **THEN** the same accepted defaults and guards remain authoritative, Generate availability stays truthful, and switching presentation alone does not capture, generate or modify existing swells

### Requirement: Sound editing has consistent context-bound interactions

Learn Sound and Perform Sound SHALL use the same editor and expose the same supported controls for the displayed definition, including Level. The host context SHALL convey Learn defaults or the selected swell without a repeated scope label. Refresh or switching context SHALL leave accepted edits and renders intact.

#### Scenario: Edit two independent contexts
- **WHEN** a user changes Learn's accepted Sound and Level and then opens Perform Sound for an existing swell
- **THEN** Perform retains that swell's settings and Learn retains the new defaults for future swells
- **AND** changing the selected swell's Sound or Level changes neither the default nor any other swell or instance

#### Scenario: Refresh and incidental input
- **WHEN** accepted state refreshes, Sound tabs switch, catalogue order changes or the pointer wheel crosses a selector
- **THEN** no assignment is submitted and selection follows the stable preset or managed file identity independently of row position
- **AND** the accepted selection and waveform remain authoritative while a request is pending

## ADDED Requirements

### Requirement: Preset selection is deliberate and contextual

The shared picker SHALL provide Previous, Preset and Next in that order. Commands SHALL wrap in catalogue order. The selector SHALL expose the full accessible name, accepted checkmark, independent hover and keyboard inspection, and input-sensitive focus. Only Custom IR SHALL expose the IR File selector and import action.

#### Scenario: Inspect and commit a choice
- **WHEN** the user opens the menu, inspects an option, dismisses it, explicitly commits, or cycles past an end of the catalogue
- **THEN** inspection and dismissal preserve accepted selection, deliberate selection submits the chosen identity and cycling wraps in the requested direction
- **AND** the compact picker truncates long visual names while preserving their full accessible names

#### Scenario: Choose Custom IR without a file
- **WHEN** Custom IR is chosen without a usable file for the current scope
- **THEN** the contextual row permits a retained choice or native import while the previously accepted preset and response remain in effect
- **AND** cancel preserves the prior Sound and failure remains actionable at that row

### Requirement: Sound controls use the supported definition and inspector space

Sound SHALL use the approved compact two-column knob geometry and exact entry for continuous values. Supported discrete rate values SHALL have a stepped dial and readable musical value. Descriptions SHALL follow the controls. The 276-unit inspector, tabs, flags, fixed Reset strip and Envelope SHALL retain their established roles.

#### Scenario: Inspect a supported discrete parameter
- **WHEN** a definition supplies Echo Rate
- **THEN** supported dial gestures and keyboard edits select only declared subdivisions and accessibility reports the musical value
- **AND** bundled presets and Custom IR expose only Level until another parameter is implemented for that definition

#### Scenario: Navigate at supported sizes or inspect frozen controls
- **WHEN** the user reaches overflow at 900×600, expands at 100% or 125%, or hovers a response control disabled by Freeze
- **THEN** labels and values remain readable, scrolling reveals focused controls, and each frozen response control supplies Unfreeze guidance through the aqua context-help footer and its accessible description

### Requirement: Preset waveform decoration reflects resolved responses

The closed picker SHALL show the accepted response waveform and menu options SHALL show their own resolved previews when available. Decoration SHALL preserve picker geometry, text, checkmark, chevron, focus and inspection readability. Waveforms SHALL remain presentation-only and use bounded off-thread peak preparation.

#### Scenario: Open a menu or prepare a replacement
- **WHEN** the user inspects bundled, Custom IR or a supported generated option, or requests a response change
- **THEN** available rows show their corresponding previews, including the accepted custom file or cached default response for a generated option
- **AND** the closed picker and menu draw responses in the reverse time direction heard in the swell
- **AND** the closed waveform remains the accepted one through pending or failed changes and is absent when no accepted preview exists

### Requirement: Slow Sound preparation has panel-local feedback

Only a request pending longer than two seconds SHALL tint the whole Sound content panel and block input within it. It SHALL show a centered small waveform and accessible caption naming the requested sound. Reduced motion SHALL show a static waveform. Tabs, flags, fixed Reset and the rest of the workspace SHALL remain outside the overlay.

#### Scenario: Request settles or context changes
- **WHEN** preparation succeeds, fails, is cancelled or superseded, or the owning context changes
- **THEN** short requests never flash the overlay, stale completions cannot change current feedback, and any covering overlay clears when its request loses ownership
- **AND** focus is restored to a valid control in the same context when appropriate without stealing focus elsewhere
