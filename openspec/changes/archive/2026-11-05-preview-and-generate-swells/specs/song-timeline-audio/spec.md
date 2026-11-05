# Spec Delta

## MODIFIED Requirements

### Requirement: Accepted source retains deterministic generation provenance
Accepted source SHALL resolve the immutable generation context assigned to each admitted span. Ordinary additive capture SHALL preserve protected spans' provenance; Recapture SHALL replace only encountered provenance with its source. Saves/consumers SHALL observe one coherent accepted source/context record, including across loops, sparse seeks and instance copies. Explicit or opted-in shared Generate SHALL freeze current accepted source/settings/default intent separately without rewriting this capture history.

#### Scenario: Additive capture crosses old and new coverage
- **WHEN** a pass with a different Default IR crosses old mapped audio/silence and an uncaptured interval
- **THEN** old source/context remains unchanged and only the newly admitted interval receives the new pass context

#### Scenario: Different contexts meet at an attack boundary
- **WHEN** onset confirmation or extraction spans two accepted contexts
- **THEN** the arrival context remains its immutable capture provenance; a new shared Generate deliberately uses the request's current accepted Default IR instead of treating that historical context as an unrequested generation command

#### Scenario: Audio identity is unchanged
- **WHEN** identical PCM is encountered with unchanged context, or an explicit context-only update is accepted
- **THEN** unchanged audio-content identities and existing render/calibration dependencies are reused without unnecessary convolution

#### Scenario: Context metadata is unavailable
- **WHEN** accepted source is readable but required generation provenance or its IR cannot be verified
- **THEN** dry source stays preserved; recovery of a saved assignment reports its missing dependency without substitution, while a new Generate must explicitly validate its own current source/default snapshot and cannot silently repair missing historical data
