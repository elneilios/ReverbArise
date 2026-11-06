# Spec Delta

## MODIFIED Requirements

### Requirement: Rhythmic snapping uses trustworthy host coordinates
Snap SHALL offer explicit 1/16 beat, 1/8 beat, 1/4 beat, 1 beat and 1/2/4 bar divisions. Beat units SHALL follow the reported meter denominator and bars its numerator. Quantization SHALL use validated absolute host PPQ origin with nearest-grid rounding and checked sample conversion, never display names, pixels or a fabricated tempo map. The accepted captured grid SHALL remain the editing coordinate system when an independently validated fixed-tempo playback run has a different origin.

#### Scenario: Meter and fractional PPQ
- **WHEN** trustworthy compatible timing reports 4/4, 3/4 or 6/8 with fractional PPQ or negative lead-in
- **THEN** each division quantizes to its absolute host grid with deterministic half ties away from PPQ zero and at most integer-sample rounding error
- **AND** unchanged-origin seeks/loops retain that mapping without changing captured positions

#### Scenario: Missing or changed mapping
- **WHEN** host timing is absent, non-finite, overflowing, capture-rate incompatible, has incompatible tempo/meter, or its origin matches neither the accepted mapping nor an independently validated current fixed-tempo playback run
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
- **WHEN** instantaneous BPM changes or timing does not establish a compatible fixed-tempo mapping
- **THEN** this fixed-tempo repair does not fabricate a global grid or infer exact tempo-ramp conversions
- **AND** exact sample editing and existing temporary bypass remain available under their normal model guards, with truthful timing feedback
