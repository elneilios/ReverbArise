# Spec Delta

## ADDED Requirements

### Requirement: Export aligns the initial Loop lead-in to a musical grid

For an accepted Loop, Export SHALL include the full initial negative wet lead-in. Using verified musical timing, it SHALL set the WAV start at the nearest selected grid boundary at or before the earliest required sample. It SHALL show the placement offset from the intended Bar 1.

#### Scenario: Export a lead-in with a four-bar grid

- **WHEN** the first downbeat needs 2.4 bars of lead-in and the user selects a four-bar grid
- **THEN** the WAV SHALL start four bars before Bar 1
- **AND** Export SHALL show where to drop the WAV so the first downbeat lands at the intended Playlist position

#### Scenario: Export with no negative wet

- **WHEN** the admitted mix has no wet before the first downbeat
- **THEN** the WAV SHALL start at Bar 1

#### Scenario: Musical placement is unavailable

- **WHEN** the accepted Loop has no verified mapping to the selected grid
- **THEN** Export SHALL explain why grid-aligned delivery is unavailable

### Requirement: Export renders a bounded Loop repeat count

Loop Export SHALL use the shared mix, preview, Save and drag behavior for the selected repeat count. It SHALL include one initial lead-in, preserve continuous timing and overlapping wet at internal wraps, and retain the final scheduled wet tail. Preparation SHALL be bounded and tied to one accepted source and mix revision.

#### Scenario: Export several repeats

- **WHEN** the user prepares more than one Loop repeat
- **THEN** each scheduled cycle SHALL contribute its lead-in and tails at their correct positions
- **AND** preview, Save and drag SHALL use the same prepared revision and downbeat offset

#### Scenario: Export an existing Song

- **WHEN** the last accepted take is a Song
- **THEN** Export SHALL keep the existing Song controls, origin and tail behavior
