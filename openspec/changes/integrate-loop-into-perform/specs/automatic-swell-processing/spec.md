# Spec Delta

## ADDED Requirements

### Requirement: Automatic Swells supports accepted Loop sources

When Automatic Swells is enabled for Loop capture, the existing automatic marker workflow SHALL generate markers from verified Loop coverage in phrase-relative coordinates. Generated markers SHALL use the shared swell editor and its existing protection and revision rules.

#### Scenario: Accept a Loop with Automatic Swells enabled

- **WHEN** a Loop take is accepted with Automatic Swells enabled and the analysis finds eligible events
- **THEN** generated markers SHALL be placed in the accepted phrase and be editable in the shared Perform editor
- **AND** obsolete analysis results SHALL be prevented from changing the current take or its edits

#### Scenario: Accept a Loop with Automatic Swells disabled

- **WHEN** a Loop take is accepted with Automatic Swells disabled
- **THEN** the captured waveform SHALL remain available for manual swell creation
