# Spec Delta

## MODIFIED Requirements

### Requirement: IR replacement retains one bounded accepted contribution

An eligible IR-only replacement SHALL retain at most one verified accepted contribution per swell, separately from requested state and pending work. A fitted shape derived for that replacement SHALL remain part of the pending result until acceptance. Failed or superseded requests SHALL not replace accepted audio. Source, timing, explicit shape or placement changes, deletion and invalid assets SHALL revoke retention. Activity, solo and live routing SHALL govern audibility. Retention SHALL NOT become a blanket stale-render fallback.

#### Scenario: Rapid selection and failed preparation
- **WHEN** response B then C are requested while accepted response A is audible, and B completes late or C fails
- **THEN** B cannot publish, A remains the sole retained contribution while its dependencies remain valid, and C's failure is reported without muting unaffected neighbours or claiming C is ready

#### Scenario: Source or editing changes during replacement
- **WHEN** source dependencies, explicit shape, placement or host mapping change while an old contribution is retained
- **THEN** retention is revoked and normal invalidation/repreparation applies. Deleting or disabling the swell cannot leave a ghost contribution.

#### Scenario: Offline bounce during a pending replacement
- **WHEN** host offline rendering snapshots a session with an eligible accepted contribution and an unfinished IR-only request
- **THEN** that snapshot renders the accepted audible version consistently, discloses the pending requested version and does not switch mid-export to a background completion
- **AND** no unavailable wet is fabricated and separate plugin instances retain independent snapshots

#### Scenario: Handover to a fitted shape
- **WHEN** the latest IR replacement needs a fitted shape and completes with an eligible retained contribution
- **THEN** the retained render stays eligible until the new IR, effective shape and prepared audio can take effect together using the existing bounded handover
- **AND** later audition, internal Export and host bounce snapshots use the accepted effective shape
