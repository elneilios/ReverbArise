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
Wet Only SHALL apply accepted starting gains, Level and Live Wet to the positioned wet sum. Dry + Wet SHALL add captured dry at unity. Both routes SHALL preserve finite above-unity float samples and disclose measured overload without changing gain.

#### Scenario: Hot correlated blend
- **WHEN** the audition sum of captured dry and wet exceeds full scale
- **THEN** the original float sum is played and overload is identified as a mixed output peak

#### Scenario: Existing low trim
- **WHEN** older pre-release state contains a legacy audition trim
- **THEN** the first audition after reopening uses Live Wet and unity captured dry in Dry + Wet
- **AND** the stored legacy trim and blend do not affect playback

#### Scenario: Level rises below protection onset
- **WHEN** a user raises a swell by 6 dB in Wet Only
- **THEN** its wet contribution increases by the requested ratio even above full scale, with any overload reported

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
Overlapping wet SHALL retain each member's saved starting gain, Level, envelope and positioned offset. Captured dry SHALL be mixed once at unity in Dry + Wet. Wet Only SHALL exclude captured dry. Live Wet SHALL set wet gain independently of the shape and listening window.

#### Scenario: Correlated overlapping wet and one dry branch
- **WHEN** two hot wet contributions overlap in Dry + Wet or Wet Only
- **THEN** the wet samples match the aligned sum of both accepted contributions
- **AND** Dry + Wet includes captured dry once and original assets retain their bytes

#### Scenario: Audition and export compare equal membership
- **WHEN** equal membership is auditioned and exported Wet Only at corresponding unity controls
- **THEN** the wet samples agree within tolerance, apart from documented monitoring boundary ramps

#### Scenario: Matching stays fixed across audition scope
- **WHEN** a source-matched swell is auditioned with different isolation, routing or listening tail
- **THEN** its accepted starting gain and Live Wet preference are retained

#### Scenario: Energy calibration does not follow monitoring loudness
- **WHEN** Live Wet changes for energy-calibrated swells
- **THEN** their source-energy measurements, starting gains and Level values remain fixed

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
Manual replay SHALL use disclosed 5 ms outer start/end ramps and smooth Stop, shortened for tiny buffers and reaching zero at boundary samples without changing interval length. Loop wraps SHALL restart the same finite window without callback allocation or buffer republishing. Explicit Stop and existing invalidation/priority guards SHALL terminate looping. Replacement SHALL preserve sequential cursor and smooth handoff. Live dry and stored PCM SHALL remain unchanged; unsafe invalidation MAY silence immediately. The callback SHALL remain allocation/lock/file-I/O free with retirement outside it.

#### Scenario: Natural completion and explicit stop
- **WHEN** audition starts, reaches a nonzero final stored sample, or receives Stop and is replayed
- **THEN** short monitoring ramps avoid an abrupt boundary step, replay restarts at the prepared window start, and live dry remains untouched; the outer ramp applies only to replay contribution (including recorded dry at window edges), with its interior unchanged

#### Scenario: Very short buffer and restoration
- **WHEN** an audition interval is shorter than the nominal ramps or an old zero-fade session reopens
- **THEN** ramp arithmetic stays finite/bounded, saved edits retain their exact values, boundary protection is disclosed separately and playback never starts on restore

### Requirement: Perform audition loops its finite window
Manual Perform audition SHALL repeat its prepared selected listening window until
Stop or existing selection/session/dependency/timing/workflow/DAW-priority guards
stop playback. Loops SHALL retain boundary ramps, sample alignment and bounded
allocation/lock/I/O-free audio processing, without republishing or accumulating
buffer ownership on each repeat. Restore SHALL NOT start playback. This SHALL NOT
extend the window or implement whole-song arrangement audition.

#### Scenario: Repeat and stop
- **WHEN** manual audition crosses the prepared window end
- **THEN** playback resumes at its start, including multiple wraps within one block and tiny buffers, with bounded position and unchanged stored audio
- **AND WHEN** Stop or an incompatible dependency interrupts playback
- **THEN** existing smooth-stop or immediate-invalidation behavior terminates it and releases buffers outside the callback

### Requirement: Perform audition exposes a Wet only toggle
Perform SHALL provide a latching button naming the selected mode, with Dry + wet when Off and Wet only when On. New instances SHALL default to Off. Existing status roles SHALL disclose readiness, monitoring ramps and overload. Audition routing SHALL remain independent of Export and live routing.

#### Scenario: Toggle live-level listening
- **WHEN** Wet only is toggled with an unchanged valid invocation
- **THEN** accepted pressed state, prepared sound and status agree without autoplay, convolution, asset edits or Export mix changes
- **AND** stale or disabled invocations cannot change a different session or workflow

#### Scenario: Restore old and new preferences
- **WHEN** a session saved under the generated-headroom policy reopens
- **THEN** its accepted Wet only choice restores without starting playback, and malformed metadata is handled transactionally

#### Scenario: Monitoring protection and unavailable source
- **WHEN** the user auditions Dry + Wet with unavailable captured dry
- **THEN** the reason is disclosed and dry is neither padded nor silently replaced with Wet Only

### Requirement: Audition strip uses one compact aligned row
Following owner approval of actual native metrics, Perform SHALL put status to
the right of Play/Stop/Wet only with equal gaps between those buttons and a slim
strip derived from existing row/inset tokens. Overview SHALL use the same shared
section-heading font role as the other section headings.

#### Scenario: Slim row remains readable
- **WHEN** the approved strip is displayed at supported sizes/scales, including empty/dense states
- **THEN** controls and status share a row with equal action gaps, full contained bounds and continuous rules, helper overflow ellipsizes without shrinking, and Overview typography agrees with other section headings
