# Spec Delta

## MODIFIED Requirements

### Requirement: Audition has independent protected level control
Audition SHALL use the accepted generated starting gains and explicit monitoring gain. The wet branch SHALL report measured overload while preserving the float signal. Existing short gain and playback ramps SHALL operate without callback allocation or blocking.

#### Scenario: Loud imported IR
- **WHEN** a generated swell uses a loud imported IR
- **THEN** it uses the accepted starting gain and a deliberate user boost can overload Wet Only without changing the stored render or capture

#### Scenario: Quiet render and live adjustment
- **WHEN** the user changes monitoring level while auditioning a quiet render
- **THEN** only the requested gain is applied with a smooth transition and no regeneration

#### Scenario: Combined output level
- **WHEN** live dry or diagnostic tone accompanies audition
- **THEN** the displayed audition peak describes the prepared audition contribution

#### Scenario: Restore audition preference
- **WHEN** a project saved with an accepted audition route is reopened
- **THEN** the route restores without starting audition

#### Scenario: Legacy state without trim
- **WHEN** a pre-release snapshot loads with absent legacy trim
- **THEN** the new monitoring gain is determined by the accepted starting gains, Level and Live Wet
