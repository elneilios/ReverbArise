# audition-monitoring Specification

## Purpose
Let users compare the swells audible in a selected listening window, or optionally isolate its selected swell, with captured dry context through a protected manual monitoring path independent of Song playback scheduling.

## Requirements

### Requirement: Captured dry and shaped wet audition align absolutely
Dry + Wet SHALL align captured dry and positioned wet by absolute sample position in the selected anchor's listening window, once. It SHALL include the base source and configured post-roll beyond the later of dry arrival/wet end, retain original dry sample order/timing, and reject missing required dry samples. Eligible neighbouring wet SHALL follow scope rules without expanding the window or duplicating dry.

#### Scenario: Wet pre-roll and dry tail
- **WHEN** wet begins before the selected dry event and dry ends after the edited wet end
- **THEN** audition includes the entire union and required listening tail with dry attack at its original relative position and no reversed or shifted dry samples

#### Scenario: Missing captured dry
- **WHEN** required dry source cannot be verified or read
- **THEN** Dry + Wet reports Dry source not captured for this span and cannot start, while independently verified committed wet remains available in explicitly labeled Wet Only mode

### Requirement: Audition blend has explicit endpoints
The audition band SHALL expose one continuous Dry-to-Wet slider and exact 0-100% entry. Below 100%, gains SHALL be dry=1-wet and wet=wet; 100% SHALL use genuine Wet Only eligibility/window semantics without requiring dry. Separate mode buttons SHALL be absent. Restore SHALL show 100% for saved Wet Only and the saved blend for mixed mode, without rewriting latent preferences or changing live routing.

#### Scenario: Endpoints and reference blend
- **WHEN** the continuous audition control is set to 0%, 50%, 65% and 100% Wet
- **THEN** its prepared contribution is respectively dry only, the half-gain sum, 35% dry plus 65% wet, and wet only, subject to disclosed monitoring protection and trim

#### Scenario: Mode round trip
- **WHEN** a user sets 35% Wet, moves the control to 100% and then explicitly enters 35%
- **THEN** 100% excludes recorded dry, the previous mixed preference remains recoverable in saved state while Wet Only is active, and the explicit 35% edit restores the mixed contribution

#### Scenario: Restore existing monitoring state
- **WHEN** an old session contains Wet Only plus latent 50% blend, or Dry + Wet at 65%
- **THEN** the slider respectively shows 100% or 65%, opening/closing the editor does not rewrite either session, and playback does not start

#### Scenario: Missing dry at the wet endpoint
- **WHEN** verified committed wet is available but captured dry context is unavailable
- **THEN** 100% remains eligible under the existing labeled wet-recovery rules, while a value requiring missing dry is unavailable with its reason and is not silently padded

### Requirement: Transition edits shape only the wet contribution
Wet endpoint, fade-out, gap/overlap and tension edits SHALL affect wet audio only. They SHALL not apply a complementary dry fade-in, ducking, time shift or stretch to captured dry, or change normal live dry passthrough. Existing explicit monitoring blend, trim and disclosed contribution protection SHALL remain separate from transition shaping.

#### Scenario: Compare dry through transition edits
- **WHEN** the same captured event is auditioned with different wet endpoints and fades using identical monitoring settings
- **THEN** dry positions and pre-monitoring sample values are identical throughout each common context span, while only the wet envelope/support changes

#### Scenario: Wet-only monitoring
- **WHEN** Wet Only is selected after configuring overlap or a gap
- **THEN** audition uses the selected anchor's shaped wet window, including eligible overlapping wet neighbours unless isolated, without replaying dry or adding a dry listening tail

### Requirement: Contextual audition continues after the transition
Dry + Wet SHALL provide a saved, typeable post-roll preference from zero to five seconds, defaulting to two seconds when absent. Playback SHALL continue through the longer of the base source interval or the later of wet end/dry arrival plus post-roll. Post-roll edits SHALL prepare monitoring only, without convolution or render-asset changes.

#### Scenario: Listen beyond overlap
- **WHEN** wet ends 300 ms after arrival and post-roll is two seconds
- **THEN** Dry + Wet includes captured dry through at least 2.3 seconds after arrival, retaining any longer explicitly selected source interval

#### Scenario: Listen across a gap
- **WHEN** wet ends 200 ms before arrival and post-roll is two seconds
- **THEN** audition includes the gap, unchanged dry arrival and at least two seconds of captured dry after arrival

#### Scenario: Missing post-roll coverage
- **WHEN** the required listening tail is not fully captured or readable
- **THEN** the UI reports the missing context and needed range, retains the requested duration, and keeps independently verified Wet Only available without padding or silently shortening Dry + Wet

### Requirement: Monitoring protection and trim remain transparent
The aggregate wet branch, formed after per-swell user shaping/gain and wet-edge safety, SHALL retain the -6 dBFS peak guard without boosting quiet renders. The prepared audition sum SHALL receive additional attenuation only if needed to remain at or below -6 dBFS before user trim. Neither guard SHALL alter saved render samples. Saved -60 to 0 dB trim SHALL follow that sum and default to 0 dB when absent. A nonzero trim SHALL be visible and resettable from the audition strip.

#### Scenario: Hot correlated blend
- **WHEN** protected wet and captured dry combine above the audition ceiling
- **THEN** only the manual audition contribution is attenuated to the ceiling before trim, and Diagnostics identifies branch and blend attenuation separately

#### Scenario: Existing low trim
- **WHEN** a saved session contains -24 dB trim
- **THEN** it restores unchanged with a visible trim indication and a reset-to-0 action, without auto-starting audition

### Requirement: Audition changes do not alter rendering or capture
Scope, mode, blend and transport-independent manual Play/Stop SHALL change only audition monitoring. They SHALL not run convolution, rewrite IR/render assets or change dry passthrough or future scheduled wet gain. Dry capture SHALL precede all audition output. Selection changes and incompatible dependencies SHALL stop the previous voice; preparation and buffer retirement SHALL remain outside the callback.

#### Scenario: Change blend while processing
- **WHEN** blend is edited during audition and capture
- **THEN** changes ramp smoothly, render count and asset checksums remain unchanged, and captured input excludes both recorded-dry replay and wet audition

#### Scenario: Switch selection
- **WHEN** selection changes or an incompatible sample rate is detected during audition
- **THEN** old playback stops without leaking samples into the new selection or reinterpreting old buffers at the new rate

### Requirement: Monitoring preferences restore without playback
Audition scope (Isolate selected), mode, mixed percentage, post-roll and validated trim SHALL persist independently of selected-swell render dependencies. Restoring a project SHALL not start audition or use these settings as Song playback controls. Invalid saved values SHALL be rejected or migrated transactionally with an explicit diagnostic.

#### Scenario: Reopen mixed audition
- **WHEN** a session saved with Dry + Wet, 65% Wet, 1.5 seconds of post-roll and -3 dB trim reopens
- **THEN** preferences restore, asset validation proceeds separately, and no recorded dry or wet plays until manual audition

### Requirement: Overlapping-window audition is the default
Audition SHALL default to eligible wet swells intersecting the selected anchor's finite listening window. Saved Isolate selected SHALL restrict membership to that ID. Eligibility SHALL respect enabled/mute/global-solo policy; strict half-open intersections SHALL be evaluated against the anchor window once. Neighbours SHALL NOT recursively extend the window or replay the song. Only eligible intersecting members SHALL affect availability.

#### Scenario: Hear overlapping swells
- **WHEN** multiple eligible swells sound within the selected listening window and isolation is off
- **THEN** their wet contributions are mixed once at correct absolute positions, clipped to that window, and a swell outside it is excluded

#### Scenario: An overlap chain reaches beyond the window
- **WHEN** an included neighbour overlaps another swell entirely outside the selected window
- **THEN** that outside swell is excluded and audition still ends at the original window boundary

#### Scenario: Isolate selected without resetting monitoring
- **WHEN** Isolate selected is enabled and later disabled
- **THEN** membership switches between selected-only and eligible overlaps, the preference is saved, and mode, blend, tail and trim remain unchanged without convolution or autoplay

#### Scenario: Activity and solo gates
- **WHEN** a member is disabled, muted or excluded by global solo
- **THEN** it contributes no wet; muting the anchor does not independently exclude eligible neighbours, and empty wet membership is reported explicitly

### Requirement: Group monitoring preserves dry and relative wet levels
Overlapping wet SHALL retain each member's user gain/envelope and positioned sample offsets before aggregate wet protection. Captured dry SHALL be read/mixed once for the anchor's context, regardless of member count. Existing dry=1-wet and wet=wet blend, aggregate wet ceiling, combined-sum protection and saved trim SHALL remain separate from transition editing. No per-member upward normalization or dry duplication SHALL occur.

#### Scenario: Correlated overlapping wet and one dry branch
- **WHEN** two hot wet contributions overlap with captured dry at 0%, 50% and 100% Wet
- **THEN** a sample oracle matches the aligned wet sum and single dry branch with disclosed aggregate/sum attenuation, raw assets and live dry remain unchanged, and relative member gains are preserved

### Requirement: Group preparation is bounded and dependency complete
Monitoring SHALL prepare an immutable group keyed by anchor/window, scope, member IDs/placements/generations/assets and capture dependencies under the shared budget. Required unavailable members SHALL block playback with identity/reason; excluded members SHALL NOT block. Changes SHALL invalidate obsolete groups. Scope and pure placement SHALL reuse raw renders without convolution. Decoding, preparation and retirement SHALL remain off the callback.

#### Scenario: One required overlapping member is unavailable
- **WHEN** an eligible intersecting member is stale, missing, corrupt or incompatible
- **THEN** overlap audition reports that member and cannot start, while explicit isolation can remain available when the selected result independently verifies

#### Scenario: A group changes during preparation
- **WHEN** an included swell moves, changes shape/activity or loses a dependency before the group publishes
- **THEN** the obsolete group cannot become current, the latest membership is prepared one required member at a time under the shared 256 MiB budget, without keeping whole-song PCM resident, and no audio starts automatically

### Requirement: Monitoring boundaries use short click-prevention ramps
Each wet audition contribution SHALL receive a disclosed 2 ms safety ramp at its actual start/end, independent of musical fades. It SHALL preserve stored PCM, saved envelope values and positioned bounds, reach zero at boundary samples and proportionally shorten for tiny intervals. It SHALL NOT introduce a dry transition gain envelope. Outer replay boundaries SHALL follow the manual-playback boundary requirement.

#### Scenario: Zero fade ends inside a dry-context audition
- **WHEN** a wet crop with a nonzero final sample and zero user fade ends while captured dry continues
- **THEN** the wet contribution reaches zero through its short safety ramp, dry continues without a transition-specific gain ramp, and saved raw/shape edits remain unchanged

### Requirement: Manual playback boundaries ramp safely
Manual replay SHALL use disclosed 5 ms outer start/end ramps and smooth Stop, shortened for tiny buffers and reaching zero at boundary samples without changing interval length. Replacement SHALL preserve sequential cursor and smooth handoff. Live dry and stored PCM SHALL remain unchanged; unsafe invalidation MAY silence immediately. The callback SHALL remain allocation/lock/file-I/O free with retirement outside it.

#### Scenario: Natural completion and explicit stop
- **WHEN** audition starts, reaches a nonzero final stored sample, or receives Stop and is replayed
- **THEN** short monitoring ramps avoid an abrupt boundary step, replay restarts at the prepared window start, and live dry remains untouched; the outer ramp applies only to replay contribution (including recorded dry at window edges), with its interior unchanged

#### Scenario: Very short buffer and restoration
- **WHEN** an audition interval is shorter than the nominal ramps or an old zero-fade session reopens
- **THEN** ramp arithmetic stays finite/bounded, saved edits retain their exact values, boundary protection is disclosed separately and playback never starts on restore

### Requirement: Audition percentage and geometry agree
The slider's thumb centre SHALL use the usable travel after thumb insets, with filled track and exact percentage representing the same value. Pointer and numeric entry SHALL agree at endpoints and intermediate values. This presentation SHALL change audition monitoring only, preserving existing gain protection, ramps and live output preferences.

#### Scenario: Reference percentages
- **WHEN** the control is set to 0%, 50%, 65% and 100% at supported sizes/scales
- **THEN** its thumb centre occupies respectively 0, 0.5, 0.65 and 1 of usable travel, and typed and dragged values match within declared rounding tolerance
- **AND** live routing, live wet level, raw asset identity and convolution count remain unchanged

#### Scenario: Preview and Song playback priority
- **WHEN** audition is requested while enabled live Song playback has priority
- **THEN** existing priority and inhibition rules apply, Play/Stop remain labeled preview actions and neither requests host transport changes
