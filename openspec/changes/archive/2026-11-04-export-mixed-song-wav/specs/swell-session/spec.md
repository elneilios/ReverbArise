# swell-session Spec Delta

## ADDED Requirements

### Requirement: Export preferences restore independently of monitoring
Export SHALL persist its own validated percentage and initialization flag. First entry without a saved Export preference SHALL seed from the effective most recent audition percentage, treating Wet Only as 100%. Later Export edits/re-entry SHALL retain its value without rewriting Perform monitoring/live preferences. Supported old sessions SHALL default to uninitialized Export state without modifying source, swells or other settings.

#### Scenario: First use and independent return
- **WHEN** Perform audition is 65% and Export has never been initialized
- **THEN** first Export entry starts at 65%; changing Export to 100%, returning to Perform and re-entering restores 100% Export and unchanged 65% Perform

#### Scenario: Legacy wet-only audition
- **WHEN** a supported old state has Wet Only with a latent 50% mixed preference and no Export settings
- **THEN** first Export entry initializes to 100%, preserving that latent preference and all source/IR/render identities

#### Scenario: Real edits notify the host
- **WHEN** Export preference initialization, a changed percentage or a completed revision reference is accepted
- **THEN** persistent non-parameter state is marked changed through the existing safe notification path
- **AND** refresh, preview cursor, failed/cancelled/stale jobs and no-op requests do not create false saved edits

### Requirement: Completed export references retain immutable history
Session state SHALL retain the accepted composition identity, completed managed WAV reference and bounded revision metadata without embedding PCM. Save during pending preparation SHALL retain coherent prior completed references and the latest desired mix separately. Restore SHALL verify references and timing without autoplay or silent replacement. Earlier file bytes and references SHALL remain recoverable after later edits or saves.

#### Scenario: Reopen a completed export
- **WHEN** a saved session with a 65% completed export reopens on the same profile
- **THEN** percentage, immutable revision and verified file reference survive, no voice starts and no new file is silently written over the prior WAV

#### Scenario: Save while a later mix prepares
- **WHEN** a 100% revision is complete, 35% is requested and the project saves before preparation finishes
- **THEN** reopen distinguishes the desired 35% mix from the older completed file, revalidates/prepares current work safely and never labels the 100% file as the ready 35% result

#### Scenario: Missing file or malformed reference
- **WHEN** a completed file is missing/corrupt or restored metadata has unsafe paths, invalid values or unsupported versions
- **THEN** missing files are reported and any recoverable rebuild publishes a new verified revision; malformed state is rejected transactionally without replacing valid state or deleting old assets

#### Scenario: Independent copied instance
- **WHEN** copied instances share an older export and later prepare different percentages
- **THEN** they preserve that immutable file while requests, new revision publication and completion authority stay instance/session scoped


### Requirement: Export inspection is ephemeral
Viewport, detail requests and previous-waveform updating presentation SHALL NOT
be serialized or notify host-save. Restore/instance changes SHALL clear these
transient owners while preserving validated Export preferences and durable WAVs.

#### Scenario: Inspect then restore
- **WHEN** a user pans/zooms or restores during pending waveform detail
- **THEN** creative state and WAV history are unchanged by navigation, old-instance display cannot publish and no preview autoplays
