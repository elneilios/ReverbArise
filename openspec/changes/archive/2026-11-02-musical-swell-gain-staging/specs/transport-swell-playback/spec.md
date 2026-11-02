# Spec Delta

## MODIFIED Requirements

### Requirement: Wet level protection is transparent
Playback SHALL follow the saved gain policy. Legacy sessions SHALL retain their -6 dBFS fixed aggregate guard and -60 to 0 dB wet level. Musical sessions SHALL use the approved musical ceiling and actual-aggregate preparation from swell-gain-staging, with -60 to +12 dB user wet level. Gain SHALL ramp over 5 ms, protection SHALL remain fixed for an accepted schedule, and stored renders SHALL remain unchanged. The wet ceiling SHALL not be claimed for dry plus wet output.

#### Scenario: Loud overlapping IR results
- **WHEN** several loud prepared swells overlap
- **THEN** their protected aggregate stays within the saved policy's wet ceiling, relative contributions remain intact, and attenuation is visible separately from user gain

#### Scenario: Boost a musical schedule with headroom
- **WHEN** musical live wet level changes from 0 to +6 dB and the actual sum has sufficient headroom
- **THEN** positioned wet rises by the corresponding amplitude ratio after the 5 ms ramp while live dry is unchanged
- **AND** the same saved wet level and fixed protection plan apply to offline export

#### Scenario: Publication pages do not change the balance
- **WHEN** playback moves between prepared pages of the same accepted schedule, seeks or loops
- **THEN** its musical protection gain stays unchanged, including when a loud member's own peak lies outside the current page

#### Scenario: Accepted source-matched version
- **WHEN** a source-matched swell plays while an IR/source replacement and its recalculated baseline are pending
- **THEN** live and offline snapshots use the accepted audio with its corresponding saved baseline, never the replacement's baseline on the old audio
- **AND** looping over unchanged captured performances does not continually remeasure the reference or alter the balance

#### Scenario: Energy method survives export and looping
- **WHEN** source-energy calibration is accepted with its method revision and RMS/reference metadata
- **THEN** live and offline paths use that same saved baseline and disclose sample-peak protection separately
- **AND** neither host block partitions, page boundaries nor a changed IR recording level introduces repeated normalization during playback

### Requirement: Live preferences restore without starting voices
Live enable, routing and wet level SHALL persist independently of audition preferences in bounded versioned metadata, with explicit migration from existing projects. The saved gain policy SHALL determine valid gain ranges and restoration behavior. Restore SHALL clear active voices and require fresh host timing and validated assets. Restoring state SHALL not request host transport or resume manual audition.

#### Scenario: Reopen an older project
- **WHEN** a project without live playback preferences is restored
- **THEN** enabled Dry + Wet with 0 dB user wet gain is selected, existing swell edits and audition preferences are preserved, and wet waits for valid playing transport and preparation
- **AND** its gain policy remains legacy-compatible until explicitly changed

#### Scenario: Restore positive musical wet level
- **WHEN** a musical-policy project saved with +4 dB live wet level reopens
- **THEN** +4 dB restores unchanged under its saved policy and validated ceiling, without autoplay or clamping to 0 dB
