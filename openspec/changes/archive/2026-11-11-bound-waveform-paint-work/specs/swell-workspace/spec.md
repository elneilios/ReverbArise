# Spec Delta

## ADDED Requirements

### Requirement: Waveform repaint work follows display resolution
Unchanged waveform layers SHALL be reusable during cursor-only refresh. Detail waveform drawing SHALL restrict work to visible data and display resolution. Full-song overview drawing SHALL use display-resolution summaries while preserving actual extrema, sparse coverage and swell positions. Hidden inline overview content SHALL cause no waveform drawing.

#### Scenario: Long song with hundreds of prepared swells
- **GIVEN** a five-minute capture with 200 prepared swell summaries at the supported editor size
- **WHEN** the user views the full song or zooms and scrolls through it
- **THEN** waveform drawing is bounded by displayed summaries instead of repeatedly drawing every subpixel peak
- **AND** audio, captured silence, unmapped and unavailable coverage remain distinct and no control or host position changes merely from navigation

#### Scenario: Cursor-only refresh
- **WHEN** transport or audition cursor position changes without waveform, viewport, size or display-setting changes
- **THEN** unchanged waveform layers are reused while cursors, selection and handles remain current

### Requirement: Display summaries follow accepted edits and data revisions
Display summaries SHALL reflect accepted source, render, shape, gain, placement, availability, viewport, size and scaling changes without showing another selection's data. Summarisation SHALL preserve exact model coordinates and SHALL NOT change audio, queue convolution, write assets or alter saved state. Interaction geometry SHALL remain derived from exact coordinates.

#### Scenario: Edit while a new render arrives
- **WHEN** a selected swell is shaped or moved while a background render or source revision is published
- **THEN** the next presented summary reflects the accepted identities and settings, with matching inspector values and handles and no stale waveform attributed to another swell

#### Scenario: Narrow impulse and sparse gap
- **WHEN** one-sample extrema, negative lead-in and an unmapped gap are viewed at full-song and close zoom
- **THEN** real extrema remain represented within their pixel columns, the gap contains no invented waveform and timing/selection hit geometry remains sample-derived

### Requirement: Waveform performance claims have matched native evidence
Waveform optimisation acceptance SHALL include matched native baseline and candidate paint distributions, visual/component and binding checks, and an editor-refresh measurement. Native software fixture results SHALL remain distinct from interactive host/DPI acceptance. Unperformed checks SHALL remain explicitly open.

#### Scenario: Review measured improvement
- **WHEN** the waveform change is presented for owner review
- **THEN** the audit's 3/5/20-minute and 10/100/200-swell scenarios report p50, p95 and maximum paint/refresh times on the same machine, build and dimensions
- **AND** cursor-only summary reuse and faithful zoom/scroll/edit invalidation are demonstrated with automated checks before one consolidated host smoke test
