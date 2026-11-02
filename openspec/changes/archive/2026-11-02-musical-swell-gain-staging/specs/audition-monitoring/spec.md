# Spec Delta

## MODIFIED Requirements

### Requirement: Monitoring protection and trim remain transparent
The aggregate wet branch SHALL follow the saved gain policy: legacy -6 dBFS protection without upward normalization, or the shared musical calibration/protection contract. The prepared audition sum SHALL receive additional attenuation only when needed to stay within that policy's ceiling before trim. Neither guard SHALL alter saved renders. Saved -60 to 0 dB trim SHALL follow the sum and default to 0 dB when absent. Nonzero trim SHALL remain visible and resettable.

#### Scenario: Hot correlated blend
- **WHEN** protected wet and captured dry combine above the audition ceiling
- **THEN** only the manual audition contribution is attenuated to the saved policy's ceiling before trim, and Diagnostics identifies branch and blend attenuation separately

#### Scenario: Existing low trim
- **WHEN** a saved session contains -24 dB trim
- **THEN** it restores unchanged with a visible trim indication and a reset-to-0 action, without auto-starting audition

#### Scenario: Level rises below protection onset
- **WHEN** musical swell Level increases by 6 dB during a unity-trim Wet Only comparison with sufficient headroom
- **THEN** its prepared wet amplitude increases accordingly without a compensating hidden guard change
- **AND** if the actual ceiling is reached, requested gain and protection attenuation remain separately visible

### Requirement: Group monitoring preserves dry and relative wet levels
Overlapping wet SHALL retain each member's saved fixed or source-matched baseline, user gain/envelope and positioned offsets before shared aggregate protection. Captured dry SHALL be mixed once. Dry=1-wet and wet=wet blend, policy ceiling, combined-sum protection and saved trim SHALL remain separate from transition editing. No additional per-member normalization beyond the saved calibration or dry duplication SHALL occur.

#### Scenario: Correlated overlapping wet and one dry branch
- **WHEN** two hot wet contributions overlap with captured dry at 0%, 50% and 100% Wet
- **THEN** a sample oracle matches the aligned wet sum and single dry branch with disclosed policy-specific aggregate/sum attenuation, raw assets and live dry remain unchanged, and relative member gains are preserved

#### Scenario: Audition and export compare equal membership
- **WHEN** the same musical-policy membership is monitored at 100% Wet and exported Wet Only with corresponding unity output controls
- **THEN** the wet gain plan and interior samples agree within declared tolerance, with monitoring boundary ramps identified separately
- **AND** switching isolation or blend does not rewrite the musical baseline or saved live preferences

#### Scenario: Matching stays fixed across audition scope
- **WHEN** a source-matched swell is auditioned with different isolation, blend, listening-tail or trim settings
- **THEN** its measured baseline is reused without remeasuring against the audition dry window or counteracting envelope edits
- **AND** only the explicit monitoring factors and aggregate protection context change

#### Scenario: Energy calibration does not follow monitoring loudness
- **WHEN** an energy-calibrated swell is monitored Wet Only or Dry + Wet with changed blend, trim, isolation or tail
- **THEN** its accepted dry/default-wet RMS measurement and baseline remain fixed, Level remains relative to that baseline, and only the explicit monitoring/protection factors change
