# Spec Delta

## MODIFIED Requirements

### Requirement: Preview and delivery share the Perform wet reference
Loop Export preview, waveform and stereo Float32 WAV SHALL share one finite mix at the accepted rate. Dry 100% SHALL be unity captured dry. Wet 100% SHALL use Perform's accepted starting gains, Level and Live Wet. Wet fader edits SHALL scale that reference. Finite above-unity samples SHALL remain intact and peak warnings SHALL describe the measured file.

#### Scenario: Preview and WAV comparison
- **WHEN** the same Loop is mixed at Dry/Wet 0/100, 100/0, 100/100 and 35/65, previewed and delivered through Drag WAV and Save WAV
- **THEN** steady interior preview samples and both decoded files agree within documented floating-point tolerance
- **AND** the composition equals Perform wet, repeated captured dry, dry plus Perform wet, and 0.35 dry plus 0.65 Perform wet respectively
- **AND** source and render bytes remain intact, peak warnings describe the file, and preview-only boundary ramps are disclosed

#### Scenario: Export an edited Perform loop as a wet stem
- **WHEN** a user fine-tunes a Loop in Perform and exports at the default Dry 0% / Wet 100%
- **THEN** the stem uses the same accepted swell levels and Live Wet as Perform
- **AND** repeat count and lead-in placement preserve the finite composition without changing the effective wet gain
- **AND** missing dry does not change the wet reference or prevent verified wet-only export
- **AND** original dry remains a separate host mixer decision
