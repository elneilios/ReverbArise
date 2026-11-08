# Spec Delta

## ADDED Requirements

### Requirement: Learn offers Song and tempo-synced Loop capture

Learn SHALL retain Song capture at the host playhead and offer Loop capture from a verified musical Start. Loop SHALL support Fixed length in bars and beats and Follow capture gate. Arm SHALL show the trigger and end condition and SHALL report any missing timing prerequisite.

#### Scenario: Arm a fixed-length Loop

- **WHEN** the user arms Fixed length with a verified clock and a supported length
- **THEN** Learn SHALL wait for the configured Start and capture from the verified phrase origin to the selected musical offset

#### Scenario: Arm Follow capture gate

- **WHEN** the user arms Follow capture gate with verified bar timing
- **THEN** Learn SHALL wait for the rising gate and capture from the verified phrase origin
- **AND** the falling gate or configured Capture Stop SHALL request a bar-aligned end

#### Scenario: Timing is unavailable

- **WHEN** the selected Loop capture route cannot verify the required musical timing
- **THEN** Learn SHALL identify the missing timing and preserve the last accepted take

### Requirement: Capture Stop completes a Loop on a verified bar boundary

After a Follow capture gate end request, Learn SHALL capture until the first verified bar boundary at or after the request and exclude that boundary's first sample. Learn SHALL show when it is waiting for the boundary. Loss of the necessary timing or audio before the boundary SHALL leave the last accepted take intact.

#### Scenario: Stop arrives during a bar

- **WHEN** the gate falls between verified bar boundaries and the clock and audio continue
- **THEN** Learn SHALL continue recording to the next boundary and accept the verified take only after capture succeeds

#### Scenario: Stop lands on the bar

- **WHEN** an end request lands exactly on a verified bar boundary
- **THEN** Learn SHALL end the take at that boundary without capturing another bar

#### Scenario: Host transport stops before the boundary

- **WHEN** a capture end request is followed by loss of the clock or audio before the required boundary
- **THEN** Learn SHALL report the interrupted take once observable and retain the previous accepted take and edits

### Requirement: Accepted take selects Perform and Export timing

The last accepted take SHALL select Song or Loop behavior in Perform and Export. The Learn preference SHALL affect the next capture only. Loop editing and export SHALL use the verified source and saved timing. Live Cue SHALL additionally require a compatible reference and a complete prepared swell set.

#### Scenario: Arm or fail a replacement capture

- **WHEN** the user changes the Learn preference, arms, cancels or fails a replacement capture
- **THEN** the last accepted take and its edits SHALL remain available
- **AND** only successful acceptance of a replacement SHALL change the source and timing used by Perform and Export

#### Scenario: Reopen a Loop project

- **WHEN** the host restores a project with an accepted Loop and saved edits
- **THEN** the plugin SHALL restore the verified source and its matching swell identities, assets and navigation
- **AND** live playback SHALL wait for a fresh compatible reference and Cue
- **AND** a missing or changed source SHALL be reported as unavailable while its saved identity and edits are retained

#### Scenario: Restore an earlier Song project

- **WHEN** the host restores an existing project with Song capture and swells
- **THEN** it SHALL retain its Song timing, source, swells, preferences, playback and export behavior
