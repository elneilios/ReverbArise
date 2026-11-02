# Spec Delta

## MODIFIED Requirements

### Requirement: Envelope controls preserve time alignment
Small outlined circular Start/End handles SHALL stay on the waveform centre line at their exact times. Outlined circular fade/tension controls SHALL remain on the curve and stack in paint order at low zoom, with topmost hit priority. Only genuine coincidences in both time and amplitude SHALL receive a small offset. Zooming SHALL separate distinct times without editing envelope metadata.

#### Scenario: Full fade coincides with endpoint
- **WHEN** a fade or tension control genuinely shares a sample-coordinate anchor with another control
- **THEN** controls remain separately grabbable and their relationship to the actual curve stays clear

#### Scenario: Zoomed-out controls overlap
- **WHEN** distinct controls become close or overlap only because of zoom
- **THEN** they stay on the actual curve with topmost hit priority and separate naturally on zoom-in
- **AND** no long collision connector or arbitrary repacking moves them away

#### Scenario: Fade edits do not move endpoint handles vertically
- **WHEN** either fade is zero, full length or edited
- **THEN** Start/End stay at the waveform centre line while fade/tension controls follow the curve
- **AND** zero fades at unity naturally separate from the endpoint circles without an artificial offset
