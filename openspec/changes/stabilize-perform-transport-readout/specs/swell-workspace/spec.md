# Spec Delta

## MODIFIED Requirements

### Requirement: Host information is informational and truthful
The bottom band SHALL show available host musical position, BPM and meter, with unavailable indications for unknown values. Processing/capture/readiness and important failures SHALL remain concisely visible in normal UI. No host Play/Stop strip or unverified seek/play/stop action SHALL be introduced.

#### Scenario: Unknown or stopped transport
- **WHEN** host position or musical mapping is unavailable, stops or seeks
- **THEN** each readout reflects its actual availability and state without prototype constants or fabricated beat positions

#### Scenario: Presentation remains read-only
- **WHEN** transport values update or the footer is resized, inspected or reopened
- **THEN** only its presentation changes, without host transport requests, timing changes, audio processing changes or saved-state edits

## ADDED Requirements

### Requirement: Perform transport fields retain stable geometry
Perform SHALL reserve independent Bar, Beat, BPM and meter slots with consistently aligned numeric values. At a fixed editor size and scale, field boundaries, separators and the complete status group's bounds SHALL remain fixed across value and availability updates. Geometry SHALL depend on available space and the display policy rather than current text width.

#### Scenario: Playback and seeking cross digit boundaries
- **WHEN** playback or seeking crosses bars 9 to 10 and 99 to 100, or beat and fractional-beat boundaries in either direction
- **THEN** all field bounds, separator positions and enclosing status bounds remain identical, with existing musical rounding preserved

#### Scenario: Tempo, meter and availability change
- **WHEN** tempo changes between integral and fractional displays, meter changes including 4/4 to 6/8 and 12/8, or any host field becomes unavailable and returns
- **THEN** neighbouring slots and separators stay anchored and each field reports its actual value or explicit unavailability

### Requirement: Stable transport information remains readable
The footer SHALL retain existing typography, semantic ink and dark styling at supported logical sizes and scales. Its slots SHALL accommodate realistic multi-digit bars, fractional beats, fractional tempos and meters without truncation or overlap. Oversized values SHALL remain inspectable without moving neighbouring slots or silently changing host values or displayed precision.

#### Scenario: Supported size and scale matrix
- **WHEN** the footer is shown at 1100x850, 1400x900, 1400x1000 and 1600x1000 with 100%, 125%, 150% and 200% scale coverage
- **THEN** ordinary and multi-digit values, units, separators and unavailable indications remain legible and contained above context help, with no automatic font shrinking
- **AND** native software-scale evidence and physical Windows/host coverage are recorded separately

#### Scenario: Large positions and unusual host values
- **WHEN** display values include negative lead-in, six-digit bars, three-digit beats with fractional display, four-digit tempos with two decimals or three-digit meter parts
- **THEN** they fit without truncation and maintain the same slot geometry as ordinary values
- **AND** still larger representable values retain a full accessible/help representation and a clear bounded overflow indication without overlapping adjacent fields

#### Scenario: Scoped presentation acceptance
- **WHEN** the revised read-only footer is ready for production integration
- **THEN** its native preview is launched directly for owner look/feel review, including changing and unavailable values at minimum size
- **AND** new tokens or visual exceptions receive explicit scoped approval before rollout, with standards and regression coverage updated together
