# Spec Delta

## MODIFIED Requirements

### Requirement: Playback validity tracks actual dependencies
Only verified current or explicitly frozen wet SHALL enter live playback, except a disclosed retained contribution during an eligible IR-only replacement. Changed source, shape, placement or mapping SHALL invalidate affected publications; cancelled generations SHALL not publish. Identical/unrelated capture SHALL preserve output. Manual missing-source recovery SHALL remain ineligible unless explicitly frozen while ready; frozen assets SHALL follow the render-freeze requirement.

#### Scenario: Recapture while listening
- **WHEN** capture commits identical source followed by changed source inside one swell's dependencies
- **THEN** identical capture does not disrupt it; changed capture removes its outdated contribution until a complete current replacement is ready, while unaffected swells remain playable

#### Scenario: Saved asset cannot verify
- **WHEN** reopen finds missing or corrupt source, IR or wet assets
- **THEN** affected wet remains excluded with an actionable diagnostic and unaffected verified swells remain eligible

#### Scenario: Replace only the IR
- **WHEN** a user requests a different IR for a verified ready swell without changing its other dependencies
- **THEN** its accepted old contribution remains eligible with its original verified IR/render identity while the replacement prepares, and only a complete latest valid replacement becomes current

## ADDED Requirements

### Requirement: IR replacement retains one bounded accepted contribution
An eligible IR-only replacement SHALL retain at most one verified accepted contribution per swell, separately from requested state and pending work. Failed or superseded requests SHALL not replace accepted audio. Source, timing, shape or placement changes, deletion and invalid assets SHALL revoke retention; activity/solo/live routing SHALL still govern audibility. Retention SHALL NOT become a blanket stale-render fallback.

#### Scenario: Rapid selection and failed preparation
- **WHEN** response B then C are requested while accepted response A is audible, and B completes late or C fails
- **THEN** B cannot publish, A remains the sole retained contribution while its dependencies remain valid, and C's failure is reported without muting unaffected neighbours or claiming C is ready

#### Scenario: Source or editing changes during replacement
- **WHEN** source dependencies, shape, placement or host mapping change while an old contribution is retained
- **THEN** retention is revoked and normal invalidation/repreparation applies; deleting or disabling the swell cannot leave a ghost contribution

#### Scenario: Offline bounce during a pending replacement
- **WHEN** host offline rendering snapshots a session with an eligible accepted contribution and an unfinished IR-only request
- **THEN** that snapshot renders the accepted audible version consistently, discloses the pending requested version and does not switch mid-export to a background completion
- **AND** no unavailable wet is fabricated and separate plugin instances retain independent snapshots

### Requirement: Completed IR replacements join the current Song position safely
A complete latest IR replacement SHALL become active at a processing boundary with a disclosed transition of at most 5 ms at the current absolute Song offset. It SHALL not restart elapsed lead-in, sustain duplicate old/new output or restart neighbouring swells. Preparation, asset work, lifetime retirement and budget checks SHALL remain outside the real-time callback, whose mixing SHALL stay bounded.

#### Scenario: Handover during an overlapping swell
- **WHEN** a replacement is ready during Song playback with another swell overlapping
- **THEN** the old/new contribution transitions at the same Song position without an abrupt discontinuity or doubled steady-state level, while the neighbour's timing and samples are unchanged

#### Scenario: Replacement while stopped or during manual audition
- **WHEN** an IR replacement finishes with Song stopped
- **THEN** it becomes eligible for subsequent playback without starting a voice, moving host transport or automatically restarting manual audition

#### Scenario: Replacement exceeds resources
- **WHEN** the old contribution plus pending preparation or handover cannot fit the supported memory budget
- **THEN** the request reports resource pressure, retains eligible accepted output and does not silently raise the budget, truncate audio or allocate/block on the live callback
