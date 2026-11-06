# Spec Delta

## ADDED Requirements

### Requirement: Explicit Swells-list navigation fits the accepted primary
Explicit list selection in Perform SHALL frame the accepted primary using the same pan, zoom and complete-context policy as Fit. Both waveform lanes, target, envelope handles, ruler, toolbar and Overview SHALL share that viewport. Repeating selection of an already-fitted swell SHALL remain visually stable. Selection SHALL follow persistent identity and canonical absolute position.

#### Scenario: Navigate distant swells
- **WHEN** the user clicks an offscreen swell and then another distant swell
- **THEN** each accepted primary immediately receives the same view as explicit Fit, including available source, requested envelope, placed wet and overlap context
- **AND** the view changes zoom as needed to show that complete region with existing Fit margins

#### Scenario: Already visible and repeated selection
- **WHEN** a clicked swell is already visible at a manually chosen pan or zoom
- **THEN** selection still applies contextual Fit to show its complete region
- **AND WHEN** the same selected row is clicked again after panning or zooming away
- **THEN** its complete fitted region returns without requiring a different selection first
- **AND** repeating selection on the already-fitted region causes no visual viewport movement

#### Scenario: Pending rendering and timeline boundaries
- **WHEN** the selected render is pending, or its context includes negative lead-in or reaches known timeline boundaries
- **THEN** automatic framing matches explicit Fit's finite bounded view using available timing and does not wait for rendering or invent waveform content

#### Scenario: Stable identities and multiple selection
- **WHEN** rows reorder or names change before explicit selection, including Ctrl/Shift selection
- **THEN** automatic Fit follows the accepted primary's persistent identity and canonical absolute timing while preserving existing selected-ID and modifier semantics
- **AND** empty or rejected obsolete-session selection does not fit another row's swell

#### Scenario: Navigation has no creative or audio side effects
- **WHEN** list selection automatically fits a swell
- **THEN** only selection and navigation change; captured PCM, timing, shape, enablement, IR/render assets and generation inputs remain unchanged
- **AND** selection does not start playback, seek the host or request rendering

## MODIFIED Requirements

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
- **AND WHEN** background publication, incidental selection refresh or mode return occurs after user navigation
- **THEN** the user view is retained until explicit Fit or navigation, including explicit Swells-list selection that applies contextual Fit, without host seeking or repeated auto-fit

#### Scenario: Rendering completes after list navigation
- **WHEN** a pending selected render completes after the user has selected another swell or manually panned or zoomed
- **THEN** matching waveform content refreshes without replaying list reveal, refitting or overriding the user's newer view
