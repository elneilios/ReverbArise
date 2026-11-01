# Spec Delta

## Purpose

Provide a usable single-screen workspace for inspecting captured audio and editing individual swells without exposing an engineering workflow by default.

## ADDED Requirements

### Requirement: Workspace reflects actual processing and selection
The main view SHALL provide a status header, left swell list, central aligned source/wet editor, right selected-swell inspector, audition strip and lower song overview. Rows SHALL show enabled state, name, independent IR name and actual processing state. Selection SHALL use a stable swell identity across asynchronous updates.

#### Scenario: Select while background work completes
- **WHEN** the user selects a swell while capture, analysis or another swell's rendering completes
- **THEN** its row, handles and inspector remain bound to that identity and show its actual IR and state

#### Scenario: Empty or unavailable selection
- **WHEN** the session is empty or the selected result is waiting, stale or failed
- **THEN** the workspace explains the state and enables only valid actions without displaying false ready waveforms or controls

### Requirement: Timeline navigation preserves exact edits
The editor SHALL align dry, wet, arrivals and handles on one absolute Song scale with bar references, zoom, pan and snap. The overview SHALL distinguish audio, captured silence, unmapped and unavailable coverage and support viewport navigation. Navigation SHALL not edit render inputs. Display-only peak scaling SHALL remain disclosed.

#### Scenario: Navigate distant sections
- **WHEN** the user clicks or drags the overview between distant captured sections
- **THEN** detail view and ruler follow the viewport without moving swells or changing source dependencies, and the gap remains visibly unmapped

#### Scenario: Loud wet waveform
- **WHEN** a selected raw render exceeds full scale
- **THEN** its envelope remains visible without flat clipping, its display scale is available, and its audio gain is unchanged by drawing

### Requirement: Inspector exposes supported musical editing
The inspector SHALL provide per-swell IR loading, name, enabled state, arrival, source and audible bounds, fades, curve presets/tension and gain. Timing SHALL use musical positions and time units by default with precise typeable values and bidirectional handles. Sample-accurate editing SHALL remain available without raw sample addresses dominating the main view. Unsupported controls SHALL be omitted.

#### Scenario: Type and drag a precise edit
- **WHEN** a user drags a fade or bound and then types an exact value
- **THEN** the handle, field and canonical edit agree without unintended whole-bar rounding or unnecessary convolution

#### Scenario: Invalid timing or crop
- **WHEN** timing is unavailable or an edit creates an invalid crop
- **THEN** the inspector retains the prior valid value or shows the explicit invalid edit state, without silently changing unrelated swells

### Requirement: Envelope handles follow the visible curve
Endpoint, fade and tension controls SHALL expose true curve anchors, generous hit areas and hover/active feedback using shared painting/hit geometry. Coincident controls SHALL use visibly connected labelled grab targets that keep zero fades reachable. Drag feedback SHALL reflect accepted edits immediately; fine envelope edits SHALL not be forced onto a coarse grid. Numeric and keyboard alternatives SHALL remain available.

#### Scenario: Grab and drag a fade
- **WHEN** the user grabs a visible fade handle and drags between grid lines
- **THEN** the intended fade changes smoothly, the handle remains on its curve, values update coherently and unrelated timing or selection does not jump

#### Scenario: Co-located or zero-length controls
- **WHEN** an endpoint and fade boundary coincide or a fade has zero duration
- **THEN** hit selection is deterministic with clear feedback and both endpoint and fade-duration controls remain directly grabbable through labelled collision targets connected to their actual anchors, and zero duration does not show a fictitious active tension curve

### Requirement: Fade tension is continuously editable
Each nonzero fade segment SHALL provide a continuous tension handle and precise value in addition to its curve preset. The displayed envelope and prepared wet audio SHALL use the same tension law, retain monotonic endpoints and preserve the existing preset at neutral tension. Tension changes SHALL not reconvolve source audio or modify dry audio.

#### Scenario: Bend a fade
- **WHEN** the user moves a fade's tension point vertically and then types a precise value
- **THEN** curve, numeric value and shaped wet agree continuously without changing fade endpoints or raw render identity

#### Scenario: Restore a previous project
- **WHEN** saved state has no tension settings
- **THEN** neutral tension preserves its existing curve presets and audible shape without resetting other edits

### Requirement: Wet endpoint controls gap and overlap independently of fade
The user SHALL be able to drag the wet endpoint before or beyond the fixed dry arrival. Its relative position SHALL determine the displayed gap or overlap. Wet fade-out duration and tension SHALL remain independently editable. Endpoint edits SHALL leave dry timing/level and other swells unchanged, retain requested bounds while support is prepared, and disclose any additional render-source context.

#### Scenario: Intentional gap
- **WHEN** the wet endpoint is placed 200 ms before dry arrival
- **THEN** wet ends at that endpoint, a 200 ms gap is shown, and dry remains at its original arrival with no transition-driven gain change

#### Scenario: Overlap with independent fade-out
- **WHEN** the wet endpoint is placed 300 ms beyond dry arrival and the wet fade-out is set to 100 ms
- **THEN** wet overlaps the first 300 ms of the source event and fades over its final 100 ms, with the dry event unshifted and unfaded by transition controls

#### Scenario: Extend beyond current support
- **WHEN** the requested endpoint exceeds the current wet render's support
- **THEN** the UI preserves that requested endpoint, highlights required render-source extension and shows preparation or missing coverage instead of silently clamping the handle to the old render

### Requirement: Engineering tools remain accessible outside the main workflow
Generate, Suggest, Regenerate, recapture, mapping preferences, raw timing, peaks, tone and asset/recovery tools SHALL remain reachable in a collapsible Diagnostics area. Fresh-use automatic processing and ordinary editing SHALL work with Diagnostics closed. The UI SHALL disclose that prepared swells currently require manual audition and do not play with host transport.

#### Scenario: Closed diagnostics
- **WHEN** a user inserts the plugin, plays input, selects a prepared swell and edits it
- **THEN** capture, target creation, rendering and editing work without opening Diagnostics or pressing a generation button

#### Scenario: Recovery and deferred transport
- **WHEN** the user opens Diagnostics or asks how to hear prepared swells
- **THEN** manual recovery tools remain functional and audition is clearly distinguished from deferred Song-synchronized playback

### Requirement: Supported editor sizes remain usable
The workspace SHALL keep its main workflow and typeable controls usable at its documented minimum and larger supported sizes. Many swells SHALL scroll without losing selection. Diagnostics expansion SHALL not permanently obscure required controls; numeric and keyboard alternatives SHALL remain available for precise editing.

#### Scenario: Resize and inspect many swells
- **WHEN** the editor is resized across supported sizes with hundreds of swells and Diagnostics expanded or collapsed
- **THEN** controls remain readable and reachable, the selected row is identifiable, and essential actions do not overlap


### Requirement: Zero fades remain directly editable on the canvas
Fade-in and fade-out duration controls SHALL remain directly grabbable at zero duration and when coincident with endpoints. Shared painting/hit-test geometry SHALL distinguish the true curve anchor from a displaced labelled collision target. Dragging SHALL use the initial value and pointer delta without a jump caused by that displacement. Endpoint editing SHALL remain independently accessible; numeric entry SHALL be an alternative, not a prerequisite.

#### Scenario: Open an initial zero fade
- **WHEN** a user grabs an initial zero-length fade-in or fade-out control and drags it
- **THEN** only that duration opens smoothly, the audible bound and dry arrival stay fixed, and endpoint and fade controls remain distinguishable

#### Scenario: Zoom merges neighbouring handles
- **WHEN** zoom or a short fade makes hit areas overlap
- **THEN** labelled targets and their anchor connectors keep each supported control reachable with consistent hover/active feedback

### Requirement: Whole-swell movement translates wet audio and envelope together
The selected wet body SHALL provide a move gesture and a precise placement alternative that translate wet PCM and its envelope by the same signed offset. Duration, fade durations, presets, tensions, gain and logical crop SHALL remain unchanged. Dry arrival, base source selection, detector anchor and unrelated swells SHALL stay fixed. Placement SHALL be saved, default zero for legacy state, and use checked arithmetic. Movement alone SHALL not reconvolve or rewrite raw assets.

#### Scenario: Move a shaped swell across the dry arrival
- **WHEN** the user drags the whole wet body earlier or later
- **THEN** waveform and every envelope point translate together with unchanged spacing, fades and tensions, while gap/overlap readouts update around the fixed dry event

#### Scenario: Precise move and invalid placement
- **WHEN** the user types or nudges placement, or requests a position that overflows representable bounds
- **THEN** valid movement preserves off-grid internal spacing and stable viewport/selection, while invalid movement is rejected transactionally without clamping individual points

#### Scenario: Reopen a moved swell
- **WHEN** a project with nonzero wet placement reopens
- **THEN** placement and shape survive, prior states use zero placement, raw assets remain reusable and audition does not start

### Requirement: Audition scope and monitoring safety are visible
The audition strip SHALL expose a saved Isolate selected toggle defaulting off, distinguish overlapping-window audition from isolation, and disclose short monitoring safety ramps separately from editable fade durations and level protection. The selected anchor/window and participating count SHALL be clear; unavailable required members SHALL be identified.

#### Scenario: Choose overlapping or isolated listening
- **WHEN** the user toggles Isolate selected with overlapping swells in view
- **THEN** scope and member count reflect the prepared audible group, Wet Only/Dry + Wet and blend remain independent, and no render is regenerated solely for that toggle


### Requirement: Marker controls are prominent and instance scoped
The main workspace SHALL expose one prominent rotary Marker sensitivity knob with a typeable 0-100% value and adjacent Auto markers On/Off switch, accessible with Diagnostics closed. Low SHALL mean more selective marking and the switch SHALL default on. Both SHALL bind to saved per-instance state independently of swell selection; any diagnostic sensitivity display SHALL share that value.

#### Scenario: Tune automatic markers from the main view
- **WHEN** a user turns or types the knob while listening to busy source
- **THEN** the value and effective marking/analyzing status update coherently, target-count feedback is available, and no selected-swell envelope, IR, blend or monitoring preference is reset

#### Scenario: Disable marking without losing the workflow
- **WHEN** Auto markers is off
- **THEN** the main view clearly shows Off, the stored sensitivity remains visible/editable, existing swells remain usable and capture/manual edits/render refresh are not presented as disabled by that switch

#### Scenario: An explicit master policy prevents marking
- **WHEN** Auto markers is on but automatic processing is explicitly disabled
- **THEN** the UI identifies marking as paused by that master policy and does not silently enable it or show an active marking state

#### Scenario: Resize and restore marker controls
- **WHEN** the editor is resized across 1100x850, 1400x900 and 1600x1000, Diagnostics is toggled, or a saved instance reopens
- **THEN** the knob, typeable value and switch stay readable/reachable, saved values remain synchronized and changing selected swells does not change these global instance preferences
