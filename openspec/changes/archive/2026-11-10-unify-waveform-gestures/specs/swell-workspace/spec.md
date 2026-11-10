# Spec Delta

## ADDED Requirements

### Requirement: Main waveforms share mouse navigation

The main Learn, Export and Perform waveform plots SHALL zoom at the pointer with
the wheel and pan horizontally with middle-button drag. Navigation SHALL respect
each view's bounds and SHALL NOT move a marker, edit a handle or seek the DAW.
Right-click and right-drag SHALL leave the view and its content unchanged.

#### Scenario: Navigate a zoomed waveform
- **WHEN** the user wheels over a main waveform and then middle-drags it
- **THEN** zoom retains the sample under the pointer where bounds permit, and the waveform follows the drag at the detail scale
- **AND** ruler, markers and any linked overview or zoom controls stay aligned to the clamped view

#### Scenario: Right button and gesture isolation
- **WHEN** the user right-clicks or right-drags, or releases a middle drag and then moves the pointer
- **THEN** the right button opens no menu and changes no viewport, marker or edit, and the released drag stops panning

### Requirement: Perform positions a local playhead and gives handles priority

On the Perform detail waveform, left-click and left-drag SHALL place an ephemeral
local playhead when the press starts on open waveform. A press on a handle SHALL
route to its existing inspection and edit rules. The local playhead SHALL remain
distinct from host transport, audition and editable swell markers.

#### Scenario: Position on open waveform
- **WHEN** the user clicks or drags on open waveform after zooming or panning
- **THEN** the local playhead identifies the mapped absolute sample in the aligned lanes and leaves the DAW, audition and swell values unchanged
- **AND** subsequent pan and zoom preserve the playhead's sample

#### Scenario: Start on a handle
- **WHEN** the user left-drags from a timing or envelope handle
- **THEN** the handle owns the gesture under its existing Snap, modifier, frozen and stale-edit guards
- **AND** a frozen or otherwise guarded handle remains uneditable and the press does not fall through to the playhead

### Requirement: Canvas focus identifies keyboard and accessibility navigation

The main waveform controls SHALL use aqua corner brackets for keyboard or
accessibility focus. A pointer press that takes focus SHALL suppress the brackets
while preserving focus, keyboard commands and independent interaction feedback.

#### Scenario: Switch between pointer and keyboard
- **WHEN** the user positions or edits by pointer and then uses the focused canvas by keyboard
- **THEN** the pointer gesture leaves the brackets hidden and keyboard use restores them without changing the meaning of the existing command

## MODIFIED Requirements

### Requirement: Timeline navigation preserves exact edits

The editor SHALL align dry, wet, arrivals and handles on one absolute Song scale
with bar references, zoom, pan and snap. The Overview SHALL distinguish audio,
captured silence, unmapped and unavailable coverage. Left-drag on the Overview
SHALL pan the detail viewport at the full-song scale without changing its span.
A click without dragging, wheel and other mouse buttons SHALL leave it unchanged.
Navigation SHALL not edit render inputs. Display-only peak scaling SHALL remain
disclosed.

#### Scenario: Navigate distant sections
- **WHEN** the user left-drags the Perform Overview between distant captured sections
- **THEN** the viewport highlight follows the drag direction within bounds and the detail view and ruler follow at the same zoom
- **AND** playheads, swells and source dependencies are unchanged and the gap remains visibly unmapped

#### Scenario: Use another gesture on the Overview
- **WHEN** the user clicks without dragging, wheels, middle-drags or right-drags over the Overview
- **THEN** the viewport, playheads and handles stay in place

#### Scenario: Loud wet waveform
- **WHEN** a selected raw render exceeds full scale
- **THEN** its envelope remains visible without flat clipping, its display scale is available, and its audio gain is unchanged by drawing

### Requirement: Overview has one truthful waveform viewport

Perform SHALL retain its compact Overview with ruler, real dry/wet summaries and
one detail-viewport highlight. Learn SHALL use one large dry waveform with an
origin-anchored full-song fit, bounded inspection zoom and pan, explicit coverage
states and no wet-envelope editing. Both views SHALL use actual absolute
positions, disclose display scaling and avoid seeking the host through viewport
navigation.

#### Scenario: Sparse map and viewport
- **WHEN** two captured sections with mapped silence are separated by an uncaptured gap and Perform detail navigation changes
- **THEN** the gap contains no invented waveform, silence remains recognizable, ruler/highlight agree with actual bounds and navigation affects only the editor

#### Scenario: Learn has one source view
- **WHEN** Learn displays a sparse capture
- **THEN** its single large timeline starts fitted to the full-song extent, no second bottom Overview or wet lane appears, and all sections/gaps stay at their absolute positions as the user zooms and pans

#### Scenario: Truly empty instance
- **WHEN** Perform has neither captured audio nor a selected swell
- **THEN** Swells (0), No swells yet, Press play to begin mapping your song, No swell selected and No audio mapped yet appear in its stable regions
- **AND** Learn uses its mode-appropriate DAW Play prompt; neither mode shows fake data or forced import/generation, and saved-off capture/generation policy is explained rather than silently enabled

### Requirement: Learn inspection is read-only, persistent across focus and ephemeral

Learn SHALL provide read-only absolute-sample inspection. Left-click or drag
SHALL pin or relocate across focus changes. Waveform-focused Escape SHALL clear
coordinate and pin together. Inspection SHALL remain available during capture,
stay ephemeral and leave capture, generation and Perform unchanged. Project/session
reset or editor reopen SHALL clear it. Inspection, transport and provisional
indicators SHALL be distinguishable.

#### Scenario: Relocate and retain a pin
- **WHEN** a left-click or drag pins a position and focus moves, the pointer leaves, capture advances or the user navigates the viewport
- **THEN** the same absolute inspection position remains and its marker is visible whenever the sample is in view
- **AND** another left-click or drag relocates it, right-click does not relocate it and Escape outside waveform focus does not clear it

#### Scenario: Inspect an active or settling pass
- **WHEN** hover, pin relocation or waveform keyboard inspection occurs while recording/draining/validating
- **THEN** inspection remains responsive without changing capture authority, accepted source/context, job counts, saved state or Perform selection

#### Scenario: Reset inspection consistently
- **WHEN** Learn is re-entered or the editor's project/instance/session is replaced, restored or cleared
- **THEN** coordinate and pinned flag clear together, hover resumes and no marker is restored from project state
- **AND** ordinary auto-fit expansion and DAW seeks preserve the absolute pin without changing manual navigation or seeking the host

### Requirement: Export waveform positions internal preview only

Left-click and left-drag SHALL position the ready internal preview using the same
absolute mapping as waveform, ruler and cursor. Stopped clicks SHALL not autoplay.
Playing seeks SHALL resume after bounded preparation. A pending previous
waveform SHALL support navigation while positioning remains disabled. Position
SHALL be ephemeral and SHALL not change DAW transport or export audio.

#### Scenario: Positioning while zoomed or panned
- **WHEN** a ready waveform is clicked or dragged after viewport navigation
- **THEN** cursor/readout identify the mapped clamped sample and Play starts there without changing view, mix or WAV alignment
- **AND** middle-drag pans without changing preview position

#### Scenario: Navigate the previous waveform while pending
- **WHEN** a previous waveform is displayed while a replacement mix is pending
- **THEN** wheel zoom and middle-drag pan remain available, while left-click and left-drag leave preview position and readiness unchanged
