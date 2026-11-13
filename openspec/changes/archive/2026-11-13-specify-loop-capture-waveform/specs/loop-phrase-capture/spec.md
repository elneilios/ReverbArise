# Spec Delta

## MODIFIED Requirements

### Requirement: Learn owns next capture intent
Learn Source Capture SHALL expose a Loop Mode toggle for the next capture. The accepted source kind and revision SHALL determine the source offered to Perform and Export. Selecting a capture type, arming or cancelling SHALL leave the accepted source and its dependent creative state intact.

#### Scenario: Select Loop while a Song is accepted
- **WHEN** the user enables Loop and arms a capture with an accepted Song source
- **THEN** Learn shows that the next capture is Loop and the accepted source remains Song
- **AND** the existing Song source and arrangement remain authoritative until a replacement succeeds

#### Scenario: Re-arm for a replacement
- **WHEN** a take has settled successfully and the user arms again
- **THEN** Learn starts one new replacement attempt and continues to identify the prior take as accepted

#### Scenario: Detect events in an accepted Loop
- **WHEN** Detect events is enabled and Learn has an accepted Loop
- **THEN** Learn shows the automatic marker controls and detects markers in the accepted phrase at the selected sensitivity
- **AND** Learn keeps the accepted phrase's markers visible while the replacement waits to record and hides them while the provisional waveform is displayed
- **AND** Generate Swells can use the accepted Loop's current eligible markers when generation prerequisites hold, while Generate on Stop retains its Song behavior

#### Scenario: Generate while a Loop clock continues
- **WHEN** Learn is idle with an accepted Loop, matching marker analysis and valid generation settings
- **THEN** explicit Generate Swells may publish definitions tied to that source revision while the clock continues
- **AND** a capture restart, source replacement, changed marker inputs or incompatible workflow transition rejects stale work and preserves unrelated manual and frozen definitions

## ADDED Requirements

### Requirement: Learn shows provisional Loop waveform progress
Learn SHALL show the current Loop take's recorded waveform and growing extent from the verified phrase start before acceptance. The waveform SHALL remain provisional until the take settles. Preview data SHALL be display-only and SHALL NOT grant access to the take for generation, playback, export or project save.

#### Scenario: Show audio before a Loop ends
- **WHEN** the active Loop take starts recording and receives audio
- **THEN** Learn shows the take's waveform in phrase-local coordinates starting at sample zero
- **AND** the displayed extent grows with the recorded audio while the take is still in progress
- **AND** any previous source retains its accepted identity until the new take settles successfully

#### Scenario: Detailed coverage reaches its preview limit
- **WHEN** detailed coverage reaches its display limit during an otherwise valid Loop take
- **THEN** Learn continues to show the growing recorded extent and a bounded waveform summary derived from the recorded audio
- **AND** inspection reports that coverage detail is unavailable
- **AND** capture and settlement continue under the existing acceptance rules

#### Scenario: Replace the provisional waveform after settlement
- **WHEN** the Loop take is accepted
- **THEN** Learn replaces the provisional display with the accepted waveform at its verified phrase extent

#### Scenario: Abandon a provisional waveform
- **WHEN** the current Loop take is cancelled, rejected or invalidated by restore
- **THEN** Learn removes its provisional waveform and shows the applicable accepted source waveform when available
