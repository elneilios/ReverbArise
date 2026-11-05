## ADDED Requirements

### Requirement: Source-edge upgrades require explicit regeneration
Verified saved renders from before source-edge conditioning SHALL remain usable on restore with unchanged inputs. Regenerate Selected SHALL upgrade selected unfrozen renders; Regenerate All SHALL include older unfrozen ready renders while skipping ready renders already using current conditioning. Frozen renders SHALL remain unchanged. Upgrades SHALL preserve capture, swell identities and edits and SHALL not require recapture.

#### Scenario: Reopen an older saved project
- **WHEN** a project with verified unchanged older renders is restored
- **THEN** its saved wet assets remain current without automatic replacement solely due to the conditioning upgrade

#### Scenario: Upgrade existing overlapping swells
- **WHEN** Regenerate All is requested with older, current and frozen ready swells
- **THEN** older unfrozen renders are regenerated with new processing identities, current and frozen ready renders retain their assets, and capture samples and swell edits remain unchanged

#### Scenario: Failed upgrade
- **WHEN** regeneration fails or is superseded
- **THEN** the previous complete wet asset and captured source remain stored and no partial result becomes ready
