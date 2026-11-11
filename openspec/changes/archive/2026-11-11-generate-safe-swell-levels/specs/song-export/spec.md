# Spec Delta

## MODIFIED Requirements

### Requirement: Mix endpoints use a shared disclosed gain plan
Song Export SHALL sum independently scaled dry and accepted wet at levels in [0,1]. Preview and WAV SHALL share each swell's accepted generated starting gain, Level, shape and edge treatment. Export SHALL preserve finite above-unity Float32 samples and report measured overload without aggregate attenuation. Documented monitoring ramps remain excluded from the file.

#### Scenario: Endpoint and blend oracle
- **WHEN** the same snapshot is prepared at Dry/Wet 0/100, 100/0, 100/100, 35/65 and 0/0
- **THEN** its samples equal accepted wet, captured dry, dry plus wet, 0.35 dry plus 0.65 wet, and silence respectively
- **AND** overlapping wet retains its relative levels and original assets retain their bytes

#### Scenario: Monitoring preferences are independent
- **WHEN** Song Export mix changes while Perform uses a different live wet level or routing
- **THEN** only Export's composition preference changes and Perform preferences are not baked into the file

#### Scenario: Boosted Song wet exceeds full scale
- **WHEN** the accepted wet mix includes a deliberately boosted swell and the file peak exceeds full scale
- **THEN** preview and Float32 WAV retain the requested gain and report the measured overload
