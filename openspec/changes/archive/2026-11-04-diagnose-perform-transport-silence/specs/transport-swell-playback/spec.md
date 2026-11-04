# Spec Delta

## MODIFIED Requirements

### Requirement: Timing incompatibility is explicit
Live wet SHALL require valid absolute host samples and fixed rate/tempo/meter compatible with accepted assets. A fresh valid playing run SHALL establish an ephemeral sample-to-PPQ mapping independent of parked/source origin. Each run SHALL retain its mapping within the existing one-sample origin tolerance; unsupported changes SHALL suppress wet with a reason. Fresh validation SHALL precede resumption. Stored sample positions, edits and assets SHALL remain unchanged.

#### Scenario: Change tempo or sample rate
- **WHEN** the host mapping becomes incompatible with prepared assets
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
- **WHEN** origin changes by more than the existing one-sample allowance during an established playing run
- **THEN** affected live wet is suppressed with the actual timing reason and the established run mapping is retained
- **AND** seeking, looping, preparation misses and revision updates do not silently establish a replacement baseline

#### Scenario: Invalid timing does not create a new run
- **WHEN** host timing is missing, nonfinite or out of range, or a callback range is invalid
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

## ADDED Requirements

### Requirement: Live admission diagnostics distinguish failure gates
Developer Diagnostics SHALL distinguish configuration/workflow exclusion, unavailable publication, incompatible timing, invalid callback range and missing prepared coverage. Observations SHALL identify their callback or worker context and expose mapping values, preparation/publication revisions, window bounds and retry outcome without modifying session state or adding unbounded callback work.

#### Scenario: Published mapping is rejected while playing
- **GIVEN** a publication exists and enabled Perform playback rejects incoming host timing
- **WHEN** developer Diagnostics is inspected
- **THEN** it identifies the rejected timing dimension and displays source/published and observed sample, PPQ, rate, tempo, meter, inferred origins and origin difference with enough precision to assess the supported tolerance
- **AND** callback range validity, publication revision and current position are visible without presenting worker observations as the same callback

#### Scenario: A valid playing origin differs from source origin
- **WHEN** a separately validated playing run has an origin offset from parked/source timing
- **THEN** Diagnostics reports the run identity, its baseline and validation result separately from the informational source/publication origin delta
- **AND** a supported cross-state offset is not falsely reported as a current mapping rejection, while a rejected change within the run remains identifiable

#### Scenario: Stop clears a play-gated failure
- **WHEN** transport stops after a mapping rejection
- **THEN** stopped readiness is distinguished from successful playing admission and a bounded last-rejection observation remains identifiable as historical evidence
- **AND** no stopped observation claims wet was audible or a window was newly prepared solely because the playing flag cleared

#### Scenario: Disabled output and preparation misses
- **WHEN** workflow, saved live enable, routing, wet level or unavailable coverage explains absent wet
- **THEN** Diagnostics exposes the relevant configuration or preparation reason separately from timing rejection, including current/prepared revision and published coverage when available

#### Scenario: Inspect without disturbing the failing instance
- **WHEN** Diagnostics opens, refreshes or closes during playback or while stopped
- **THEN** project state, source/render identities, assets, transport and output remain unchanged
- **AND** callback observation publication performs no allocation, deallocation, lock, blocking wait, file access or logging

### Requirement: Transient live preparation rejection recovers at current position
Recoverable preparation or publication rejection SHALL remain eligible for bounded worker retry while current dependencies and timing are valid. Recovery SHALL validate the latest session, source/render identity and mapping before admitting complete prepared wet at current Song time. Unsupported timing and invalid assets SHALL remain explicitly unavailable; recovery SHALL NOT reinterpret stored positions or fabricate ready output.

#### Scenario: Stopped preparation followed by moving Song playback
- **GIVEN** an instance with 61 eligible swells has a stopped prepared window
- **WHEN** compatible Song playback starts before a swell and advances through it
- **THEN** available wet follows its absolute position and later windows remain eligible for preparation without an Export roundtrip, reset or recapture

#### Scenario: A transient publication attempt fails
- **WHEN** a preparation/publication attempt is transiently rejected and its blocking condition subsequently clears while playback continues
- **THEN** a later bounded worker attempt can publish a fully revalidated current window and wet joins at the then-current offset with the existing bounded transition
- **AND** elapsed wet is not replayed and retry status remains distinct from permanent timing incompatibility

#### Scenario: Dependencies change during retry
- **WHEN** source, render, session generation or mapping changes before a pending retry publishes
- **THEN** obsolete work is rejected and only complete output validated against the latest compatible dependencies can be admitted

#### Scenario: Genuine incompatibility persists
- **WHEN** rate, tempo, meter or the established playing-run mapping remains unsupported
- **THEN** affected wet stays unavailable with its actual reason and stored positions and accepted assets remain intact
- **AND** retry does not relax mapping guards or report readiness without validation

#### Scenario: Worker revalidates across clock transitions
- **WHEN** preparation spans a confirmed Stop/Play, restore or live admission transition
- **THEN** publication revalidates current run identity, timing, source/render dependencies and coverage or remains retryable
- **AND** a matching render revision or resident window alone does not certify a superseded clock validation, while still-valid resident PCM can be reused without changing its positions
