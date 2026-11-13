# Spec Delta

## ADDED Requirements

### Requirement: Every Sound assignment supports shared shaping

Every available preset and Custom IR SHALL support Level, Damping, High Pass and Low Pass. New assignments SHALL default to 0 dB Level, 0% Damping, and Off for each cut unless accepted Learn defaults specify other values.

#### Scenario: Set the full creative range
- **WHEN** an eligible assignment is edited
- **THEN** Damping accepts 0–100%, and each cut accepts Off or a cutoff within 20 Hz–20 kHz
- **AND** Level retains its existing per-context range and gain behavior

#### Scenario: Apply source cuts to the wet response
- **WHEN** a High Pass or Low Pass is enabled
- **THEN** the wet render reflects the corresponding high-pass or low-pass filtering of the captured source before convolution
- **AND** dry playback and captured PCM remain unchanged

#### Scenario: Shorten a ringing response
- **WHEN** Damping increases from 0% on a full response
- **THEN** progressively less of the IR tail contributes to the wet swell, spanning the original decay through a short accent at the top of the range
- **AND** the selected arrival stays fixed and Envelope stays independently editable

#### Scenario: Dampen a rhythmic preset
- **WHEN** Stereo Ping Pong Crescendo uses its shared Damping dial
- **THEN** Damping shortens the ring within each hit while preserving its echo grid, stereo assignment and saved Damping behavior

### Requirement: Shared shaping preserves accepted sound on reopen

Existing swells SHALL retain their settings, immutable source and IR references, and accepted audio when they reopen. The neutral shared processing SHALL bypass the new filters and decay shaping exactly.

#### Scenario: Load a project from before shared shaping
- **WHEN** a project lacks the new shared values
- **THEN** absent values read as neutral and any previously saved Level or preset-specific Damping retains its existing value and sound
- **AND** opening the project does not queue regeneration just to add shared controls

#### Scenario: Adopt shared values for new swells
- **WHEN** Learn accepts a shared shaping edit
- **THEN** new automatic and manual swells inherit the accepted settings
- **AND** no existing swell acquires those values solely because the defaults changed

#### Scenario: Save an accepted shaped assignment
- **WHEN** a project with a non-neutral shared setting is saved and reopened
- **THEN** its canonical setting and accepted render reopen together
- **AND** a failed, stale, superseded or frozen edit cannot replace that accepted pair
