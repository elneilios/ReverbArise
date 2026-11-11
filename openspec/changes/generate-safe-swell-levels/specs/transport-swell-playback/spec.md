# Spec Delta

## MODIFIED Requirements

### Requirement: Live routing is independent of manual monitoring
Live enable, routing and wet level SHALL remain independent of audition, defaulting to enabled Dry + Wet at 0 dB. Dry + Wet SHALL add prepared wet to incoming dry at unity. Wet Only SHALL suppress dry during valid enabled playback. Capture and Finalise SHALL preserve dry passthrough and saved Live preferences.

#### Scenario: Play in context
- **WHEN** a prepared project plays in default Dry + Wet
- **THEN** output is the live input plus positioned wet without captured-dry replay or audition controls

#### Scenario: Route wet only and disable
- **WHEN** Wet Only is selected during Live playback and playback is subsequently disabled
- **THEN** enabled Wet Only outputs prepared wet and disabling restores dry and removes wet with the existing routing ramps

#### Scenario: Enter Finalise from Wet Only
- **WHEN** Finalise is entered from Live with Wet Only selected
- **THEN** scheduled wet stops through its bounded transition, output becomes dry passthrough and returning to Live restores the preference

### Requirement: Wet level protection is transparent
Song and Loop wet playback SHALL apply the accepted generated starting gains, Level and Live Wet. Live Wet SHALL keep its -60 to +12 dB range and existing gain transition. The wet path SHALL preserve finite above-unity float samples and report overload without applying shared attenuation.

#### Scenario: Loud overlapping IR results
- **WHEN** default generated swells overlap
- **THEN** the wet signal uses the accepted starting gains that satisfy the default ceiling

#### Scenario: Boost a musical schedule with headroom
- **WHEN** Live Wet changes from 0 to +6 dB and the wet sum remains below full scale
- **THEN** the wet sum rises by the corresponding ratio after its gain transition

#### Scenario: Boost an already hot schedule
- **WHEN** Live Wet changes from 0 to +6 dB
- **THEN** the wet sum rises by the corresponding ratio after its gain transition
- **AND** a peak above full scale is reported without reducing the wet sum

#### Scenario: Publication pages do not change the balance
- **WHEN** playback changes pages, seeks, or loops with the same accepted schedule
- **THEN** the same accepted starting gains apply independently of peak placement and block partitioning

#### Scenario: Accepted source-matched version
- **WHEN** a source-matched swell plays while an IR or source replacement is pending
- **THEN** live and offline snapshots pair the last accepted audio with its corresponding starting gain
- **AND** the replacement gains and audio publish as one coherent revision

#### Scenario: Energy method survives export and looping
- **WHEN** accepted swells play or export from their saved energy-calibrated revision
- **THEN** both paths use the same saved starting gains and report the applicable measured overload
- **AND** paging and repetition do not remeasure or normalize the mix

### Requirement: Loop playback pins coherent prepared revisions
Loop wet SHALL use a complete prepared revision of the accepted source at the host sample rate. Queued launches SHALL require current render dependencies. An audible pass SHALL retain its last complete mix while a compatible edit prepares, then crossfade to the complete successor at the current Loop position without re-cueing. An intentionally silent mix SHALL allow a playing Loop to keep its position. Publication and retirement SHALL occur outside the callback. Resource admission SHALL account for pinned data and worst-case overlapping contributions within the shared preparation budget.

#### Scenario: Edits invalidate a queued or active revision
- **WHEN** placement or render dependencies change while a Loop is queued or playing
- **THEN** a queued launch with obsolete dependencies is cancelled
- **AND** an audible Loop keeps its phase and uses the last complete mix until a compatible successor crossfades in during repeating playback
- **AND** a change to the accepted source identity or sample-rate compatibility cancels or fades the old playback

#### Scenario: Prepared workload exceeds the supported bound
- **WHEN** the pinned memory or worst-case scheduled overlap exceeds the admission budget
- **THEN** Cue reports the limiting resource and dry continues without partial or truncated wet
- **AND** an existing compatible pass retains its last complete mix
- **AND** the callback performs no allocation, deallocation, disk I/O, convolution, blocking or locking

#### Scenario: Fixed-grid Loop peak protection follows the rendered mix
- **WHEN** a default generated Loop plays at its fixed sample period
- **THEN** the accepted gains satisfy the default ceiling at launch, during repeat and through the final admitted pass
- **AND** variable phase playback uses the same saved gains with its supported alignment accounted for during generation

## ADDED Requirements

### Requirement: Realtime Dry + Wet preserves the float sum
Realtime Song and Loop Dry + Wet SHALL output the direct sum, including finite samples above full scale. Overload reporting SHALL leave both contributions at their requested gains. Existing stop fades SHALL apply in realtime and host offline processing.

#### Scenario: Dry and wet peaks coincide
- **WHEN** live dry plus wet exceeds full scale during playback
- **THEN** the output reports overload and preserves the exact float sum in each channel within numerical tolerance
- **AND** later samples receive no recovery attenuation

#### Scenario: Offline rendering begins during Loop playback
- **WHEN** an offline callback disables Loop playback while wet audio is sounding or fading
- **THEN** the existing wet stop fade is emitted with the same gain behavior as in realtime

## REMOVED Requirements

### Requirement: Realtime Dry + Wet output has linked peak protection
**Reason**: Linked reduction can duck both dry and wet after a deliberate gain boost. Overload reporting replaces the output guard.
**Migration**: The new behavior applies to the pre-release build without an audible compatibility mode.
