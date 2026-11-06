# Spec Delta

## ADDED Requirements

### Requirement: Musical presentation and entry share local conversion
Learn, Perform and Export rulers, position labels and musical range/relative entry SHALL use the same accepted local mapping and precision policy as Snap. Conversion SHALL resolve each endpoint at its actual sample position, including across tempo boundaries. Unverified regions SHALL show unavailable musical values with an accessible reason; valid sample/time alternatives SHALL remain usable without shifting audio or redesigning controls.

#### Scenario: Range spans a tempo boundary or gap
- **WHEN** a bar/beat or relative-duration edit spans tempo changes or missing evidence
- **THEN** each required endpoint and intervening conversion authority is checked, verified results agree with canvas/inspector samples and unavailable conversions preserve the prior valid value
- **AND** current playhead BPM is not extrapolated across the range, a gap is not hidden and exact sample/time entry remains available

#### Scenario: Partial ruler and truthful current transport
- **WHEN** the viewport contains both verified mapping and uncertain ramp interiors or sparse gaps
- **THEN** musical marks appear only where supported, absolute layers remain aligned and current host BPM/PPQ remain informational rather than rewriting the accepted ruler
- **AND** overflow, inverse ambiguity and unsupported precision remain explicit in existing helper/accessibility text

### Requirement: Mapping status distinguishes audio and musical readiness
Existing notices, helpers and Diagnostics SHALL distinguish accepted sample alignment, local grid coverage/precision, demonstrable mapping conflict and unavailable comparison. Exact inputs and provenance SHALL be inspectable without implying simultaneous callback/UI snapshots or complete host-map knowledge. Status refresh SHALL remain read-only, bounded and independent of wet audibility.

#### Scenario: Wet works while a ramp grid is unavailable
- **WHEN** safely sample-aligned wet is admitted while a requested musical conversion lacks precision
- **THEN** feedback identifies the local conversion limitation without reporting an audio-clock failure or globally disabling editing
- **AND** accepted evidence, source/render identities, routing and remembered Snap/grid preferences remain unchanged

## MODIFIED Requirements

### Requirement: Rhythmic snapping uses trustworthy host coordinates
Snap SHALL offer explicit 1/16 beat, 1/8 beat, 1/4 beat, 1 beat and 1/2/4 bar divisions. Beat units SHALL follow the reported meter denominator and bars its numerator. Quantization SHALL use verified local accepted musical mapping with nearest-grid rounding and checked sample conversion, never display names, pixels or invented map precision. The accepted captured grid SHALL remain the editing coordinate system when an independently validated fixed-tempo playback run has a different origin.

#### Scenario: Meter and fractional PPQ
- **WHEN** trustworthy compatible timing reports 4/4, 3/4 or 6/8 with fractional PPQ or negative lead-in
- **THEN** each division quantizes to its absolute host grid with deterministic half ties away from PPQ zero and at most integer-sample rounding error
- **AND** unchanged-origin seeks/loops retain that mapping without changing captured positions

#### Scenario: Missing or changed mapping
- **WHEN** host timing is absent, non-finite, overflowing, capture-rate incompatible, has incompatible meter or demonstrated mapping conflict, lacks sufficient local precision, or its fixed-tempo origin matches neither the accepted mapping nor an independently validated current fixed-tempo playback run
- **THEN** effective Snap On prevents horizontal snapped publication and explains unavailable timing rather than guessing
- **AND** Snap Off or Ctrl bypass permits valid unsnapped sample edits while navigation and vertical editing remain usable
- **AND** a known rejected current run SHALL NOT become available through a fallback to the saved origin, even if the latest observation happens to match that saved origin

#### Scenario: Timing changes during a drag
- **WHEN** a drag's session, selection, mode or validated mapping changes
- **THEN** further publication from that obsolete gesture is cancelled with prior accepted musical/source state preserved and an understandable reason

#### Scenario: Stable fixed-tempo playback offset
- **GIVEN** accepted capture timing and an independently validated current playback run have compatible rate, tempo and meter
- **WHEN** the host remains consistent with that run although its origin differs from the accepted capture origin, including the recorded approximately +16-sample case
- **THEN** Snap remains available and quantizes to the same accepted captured-grid sample positions as before Play
- **AND** positive, negative and zero stable offsets use the same validation policy without a hard-coded offset or expanded drift tolerance
- **AND** Play, Stop, seeks and loops alone do not rewrite accepted timing, source, swell positions or render assets

#### Scenario: Actual drift after an admitted offset
- **GIVEN** a shifted fixed-tempo run previously made the captured grid available
- **WHEN** subsequent host timing violates that run's existing one-sample mapping guard or its rate, tempo or meter compatibility
- **THEN** snapped publication becomes unavailable and obsolete gestures cannot continue under the previous admission
- **AND** diagnostics distinguish the run rejection from an informational source-to-run offset

#### Scenario: Shared editing and readiness
- **WHEN** the shifted run is valid and the user uses canvas timing gestures, inspector knobs/wheel, snapped keyboard actions or target insertion
- **THEN** each path uses the same accepted grid and availability decision with existing canonical bounds, modifiers, exact-entry and identity guards
- **AND** grid availability does not require an audible swell, prepared render, selected IR, enabled wet output or an open Diagnostics window

#### Scenario: Stopped empty and restored sessions
- **WHEN** the transport is stopped, an empty session bootstraps timing, or a saved session is reopened
- **THEN** existing trustworthy stopped/empty behavior remains and a retired or unavailable run cannot authorize another session's grid
- **AND** unchanged source/render identities and saved Snap/grid preferences survive with no autoplay or persistent clock migration

#### Scenario: Tempo automation remains outside fixed-grid coverage
- **WHEN** a legacy fixed-grid evaluation encounters instantaneous BPM changes or lacks a compatible fixed-tempo mapping
- **THEN** that legacy path does not fabricate a global grid or infer exact ramp conversions
- **AND** exact sample editing and temporary bypass remain available; accepted tempo-map operations use their separate verified local evidence

#### Scenario: Tempo automation uses verified local coverage
- **WHEN** tempo automation supplies locally verified conversion in one region and insufficient evidence in another
- **THEN** the verified region supports local Snap at integer-sample precision, while unsupported regions cannot fabricate a global grid or exact ramp conversions
- **AND** exact sample editing and existing temporary bypass remain available under their normal model guards, with truthful timing feedback

#### Scenario: Shared local map revision during a gesture
- **WHEN** the user edits by canvas, inspector knob/wheel, snapped keyboard action or target insertion across tempo-map coverage
- **THEN** all paths use one accepted conversion result, canonical bounds and evidence revision; only an affected mapping/authority change cancels a pending gesture
- **AND** ordinary playhead BPM progress does not move the editing grid, mutate captured evidence or cancel a still-valid edit

### Requirement: Learn inspection readout uses reliable musical timing
The existing inspection row SHALL show exact sample and coverage classification, plus bar/beat only from verified local accepted mapping and elapsed time from valid absolute samples and capture rate. Missing/incompatible timing SHALL be explicitly unavailable. Full coverage ranges SHALL remain accessible without new controls or fabricated coordinates.

#### Scenario: Inspect with and without reliable timing
- **WHEN** an inspected sample has verified local accepted musical mapping and a compatible capture rate
- **THEN** its bar/beat and elapsed time from song origin are shown with exact sample and truthful audio/silence/gap/unavailable/provisional classification
- **AND WHEN** that timing is unavailable or incompatible
- **THEN** exact sample/coverage remain available and musical timing is explicitly unavailable rather than fabricated; sample-derived elapsed time remains available when its independent rate/sample inputs are valid
