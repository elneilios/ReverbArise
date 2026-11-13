# Spec Delta

## ADDED Requirements

### Requirement: Ping Pong exposes the same musical controls in both Sound contexts

The shared Sound editor SHALL show Tempo Sync, Echo Rate, Stereo Spread, Build Intensity, Gate Amount, Damping and Level for Stereo Ping Pong Crescendo. The controls SHALL use the existing labeled switch and compact knob grid. Echo Rate SHALL use a stepped dial with a read-only musical value and the full subdivision name available to accessibility.

#### Scenario: Reset one Sound dial
- **WHEN** the user double-clicks an editable Sound dial or chooses its reset action
- **THEN** only that dial returns to its default, using eighth notes, 85% Stereo Spread, 75% Build Intensity, 0% Gate Amount, 0% Damping, or 0 dB Level as applicable
- **AND** the reset affects the current Learn defaults or the selected Perform swell according to the active workspace

#### Scenario: Compare Learn and Perform
- **WHEN** Ping Pong is accepted in Learn and on a selected Perform swell
- **THEN** the same control family and ordering appear in both contexts at the compact and expanded plugin sizes
- **AND** Learn edits the future default, Perform edits only the selected swell, and Freeze applies the existing response-edit guard and help

#### Scenario: Inspect an unavailable tempo
- **WHEN** the displayed Sync-on assignment uses the authored tempo or retains a previous response because compatible timing is unavailable
- **THEN** the editor reports the timing condition using existing contextual feedback and keeps showing the accepted waveform
- **AND** the established delayed preparation overlay remains scoped to Sound while an eligible request is pending
