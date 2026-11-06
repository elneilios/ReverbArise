# Spec Delta

## MODIFIED Requirements

### Requirement: Timing incompatibility is explicit
Live wet SHALL require valid absolute sample timing and compatible accepted asset identity, rate and constant meter. Accepted tempo-map evolution SHALL not reject aligned audio. Fixed-tempo sessions SHALL retain independent run validation and the existing one-sample origin guard. Demonstrated alignment conflicts SHALL suppress affected wet with a reason; missing grid evidence alone SHALL not. Fresh runtime admission SHALL follow restore/reset without rewriting stored samples or identities.

#### Scenario: Change tempo or sample rate
- **WHEN** the host rate/meter or a demonstrated alignment conflict becomes incompatible with prepared assets
- **THEN** those swells are unavailable for live playback and the workspace explains the incompatibility; default Dry + Wet continues passing dry

#### Scenario: Playing clock differs from parked source mapping
- **GIVEN** verified ready wet at fixed absolute sample positions and a compatible fixed rate, tempo and meter
- **WHEN** a fresh playing run supplies a valid mapping whose origin differs from the parked/source mapping, including the recorded approximately 16-sample difference
- **THEN** the playing mapping is validated independently and eligible wet plays at its stored absolute sample positions
- **AND** no saved mapping, source, render, placement or persistent identity is rewritten or offset, and a resident valid window remains eligible for a warm join

#### Scenario: Zero-offset transport remains compatible
- **WHEN** a fresh playing run has the same origin as parked/source timing
- **THEN** existing absolute-position output, routing and gain behavior remain unchanged

#### Scenario: Mapping changes inside a playing run
- **WHEN** origin changes by more than the existing one-sample allowance during an established fixed-tempo playing run
- **THEN** affected live wet is suppressed with the actual timing reason and the established run mapping is retained
- **AND** seeking, looping, preparation misses and revision updates do not silently establish a replacement baseline

#### Scenario: Invalid timing does not create a new run
- **WHEN** absolute host timing is missing, nonfinite or out of range, a callback range is invalid, or required fixed-tempo run validation fails
- **THEN** affected wet is unavailable and that observation cannot establish a valid playing mapping or masquerade as a confirmed Stop
- **AND** returning timing is freshly validated against the retained run mapping before wet resumes

#### Scenario: Confirmed Stop followed by a new Play
- **WHEN** valid stopped transport is followed by a fresh valid playing run
- **THEN** a new runtime mapping is validated independently, with no automatic audition and no persistent edit
- **AND** only complete current assets can join at the new absolute Song position without replaying elapsed audio

#### Scenario: Session and workflow admission are independent
- **WHEN** state is restored, the audio session is replaced, or Perform is re-entered while the host is already playing
- **THEN** live admission requires fresh runtime validation and current asset dependencies, and no stale run identity can admit old work
- **AND** disabled live output remains disabled and Capture, Audition, internal Export and host offline timing behavior retain their existing contracts

#### Scenario: Existing tempo automation and unavailable grid
- **WHEN** an accepted sample-aligned swell spans a tempo step or ramp at the supported rate and constant meter
- **THEN** live and host offline playback use the same accepted positioned wet samples without a BPM-driven mute, stale transition or render restart
- **AND** missing PPQ or uncertain local grid precision alone does not silence otherwise admitted audio
- **AND** fixed-tempo drift, session/run retirement, missing assets and genuine sample-alignment conflicts retain explicit rejection

#### Scenario: Seek and block partitions across automation
- **WHEN** variable live/offline blocks seek or loop through a prepared swell spanning tempo changes
- **THEN** each path joins at the same absolute offset, without global BPM extrapolation or replay of elapsed lead-in
- **AND** unverified musical loop bounds are not converted using the current BPM as a whole-song mapping

### Requirement: Adjustable regeneration and per-swell render freeze
The system SHALL save independent Regeneration sensitivity and per-swell Freeze render preferences. Freeze SHALL require ready committed wet and retain verified audio across source changes, capture clear and reopen, at its saved sample rate and constant meter under accepted timing evidence. Frozen shape/gain/placement/activity SHALL remain editable. Source/IR/support changes SHALL require unfreeze and current dependency validation.

#### Scenario: Keep an accepted performance fixed
- **WHEN** the user freezes a ready swell and subsequent source capture differs or is unavailable
- **THEN** live playback and offline export use its verified saved wet independently of current source and IR availability
- **AND** captured Dry + Wet audition still requires available dry context

#### Scenario: Separate detection and regeneration controls
- **WHEN** the user reduces Regeneration sensitivity
- **THEN** more performance variation is tolerated against the retained baseline without changing Marker sensitivity or explicit exact recapture
- **AND** both sensitivity preferences and frozen choices survive save/reopen

#### Scenario: Unfreeze an accepted render
- **WHEN** the user unfreezes a swell after its source changes
- **THEN** the system validates current dependencies and regenerates as needed
- **AND** source, IR and render-support editing become available again

#### Scenario: Frozen wet crosses accepted tempo changes
- **WHEN** verified frozen wet spans a pre-existing accepted step/ramp or is restored while the host is at another BPM in the same map
- **THEN** its saved PCM remains eligible at stored absolute samples after runtime admission, independently of current source availability
- **AND** a demonstrated map-alignment conflict or incompatible rate/meter remains explicit without modifying the frozen asset

### Requirement: Offline tail timing does not invalidate completed wet
After all admitted wet intervals have ended, an offline host PPQ-origin reset with compatible valid sample rate and constant meter SHALL preserve prior export results and selected dry routing. It SHALL NOT report completed wet as incomplete solely because of that reset. Demonstrated alignment conflicts while wet remains required SHALL still fail explicitly with sample/PPQ context; accepted tempo automation or missing musical coverage alone SHALL not imply a conflict.

#### Scenario: PPQ resets after the last swell
- **WHEN** offline callbacks advance beyond every prepared wet endpoint but PPQ freezes or resets
- **THEN** completed wet retains its successful or skipped-member status and Dry + Wet passes dry while Wet Only outputs silence

#### Scenario: PPQ changes during required wet
- **WHEN** the host mapping demonstrably conflicts with accepted alignment while an admitted swell is current or still due
- **THEN** the export error remains visible with timing context rather than claiming success

#### Scenario: Tempo changes before and after the last wet endpoint
- **WHEN** a bounce crosses accepted tempo changes and then reaches a host tail with no remaining required wet
- **THEN** output and completed/skipped-member status remain truthful; no fixed-BPM prediction creates extra wet or falsely rejects finished audio

### Requirement: IR replacement retains one bounded accepted contribution
An eligible IR-only replacement SHALL retain at most one verified accepted contribution per swell, separately from requested state and pending work. Failed or superseded requests SHALL not replace accepted audio. Source, demonstrated timing-alignment conflicts, shape or placement changes, deletion and invalid assets SHALL revoke retention; activity/solo/live routing SHALL still govern audibility. Retention SHALL NOT become a blanket stale-render fallback.

#### Scenario: Rapid selection and failed preparation
- **WHEN** response B then C are requested while accepted response A is audible, and B completes late or C fails
- **THEN** B cannot publish, A remains the sole retained contribution while its dependencies remain valid, and C's failure is reported without muting unaffected neighbours or claiming C is ready

#### Scenario: Source or editing changes during replacement
- **WHEN** source dependencies, shape or placement change, or host mapping demonstrably conflicts with accepted alignment while an old contribution is retained
- **THEN** retention is revoked and normal invalidation/repreparation applies; deleting or disabling the swell cannot leave a ghost contribution

#### Scenario: Offline bounce during a pending replacement
- **WHEN** host offline rendering snapshots a session with an eligible accepted contribution and an unfinished IR-only request
- **THEN** that snapshot renders the accepted audible version consistently, discloses the pending requested version and does not switch mid-export to a background completion
- **AND** no unavailable wet is fabricated and separate plugin instances retain independent snapshots

#### Scenario: Accepted tempo map evolves during IR replacement
- **WHEN** the current BPM changes within accepted map evidence while one verified contribution is retained
- **THEN** that BPM change alone does not revoke retention, modify its saved baseline or alter the pending latest-request guards

### Requirement: Playback validity tracks actual dependencies
Only verified current or explicitly frozen wet SHALL enter live playback, except a disclosed retained contribution during an eligible IR-only replacement. Changed source, shape, placement or demonstrated timing-alignment conflicts SHALL invalidate affected publications; cancelled generations SHALL not publish. Identical/unrelated capture SHALL preserve output. Manual missing-source recovery SHALL remain ineligible unless explicitly frozen while ready; frozen assets SHALL follow the render-freeze requirement.

#### Scenario: Recapture while listening
- **WHEN** capture commits identical source followed by changed source inside one swell's dependencies
- **THEN** identical capture does not disrupt it; changed capture removes its outdated contribution until a complete current replacement is ready, while unaffected swells remain playable

#### Scenario: Saved asset cannot verify
- **WHEN** reopen finds missing or corrupt source, IR or wet assets
- **THEN** affected wet remains excluded with an actionable diagnostic and unaffected verified swells remain eligible

#### Scenario: Replace only the IR
- **WHEN** a user requests a different IR for a verified ready swell without changing its other dependencies
- **THEN** its accepted old contribution remains eligible with its original verified IR/render identity while the replacement prepares, and only a complete latest valid replacement becomes current

#### Scenario: Grid-only evidence changes
- **WHEN** accepted musical evidence is refined without changing PCM, explicit sample bounds, shape, gain or placement
- **THEN** verified sample-based wet remains valid, conversion-dependent work checks its own evidence revision and no unrelated render is reconvolved
