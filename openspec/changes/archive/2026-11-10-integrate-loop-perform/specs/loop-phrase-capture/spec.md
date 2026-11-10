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
- **AND** a pending replacement keeps the accepted phrase's markers visible
- **AND** Generate Swells can use the accepted Loop's current eligible markers when generation prerequisites hold, while Generate on Stop retains its Song behavior

#### Scenario: Generate while a Loop clock continues
- **WHEN** Learn is idle with an accepted Loop, matching marker analysis and valid generation settings
- **THEN** explicit Generate Swells may publish definitions tied to that source revision while the clock continues
- **AND** a capture restart, source replacement, changed marker inputs or incompatible workflow transition rejects stale work and preserves unrelated manual and frozen definitions
