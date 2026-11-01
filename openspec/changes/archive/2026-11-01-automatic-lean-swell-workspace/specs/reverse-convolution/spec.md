# Spec Delta

## MODIFIED Requirements

### Requirement: Generation is asynchronous and publishes complete results
The plugin SHALL automatically prepare eligible swells and retain Load IR and diagnostic Generate recovery controls with honest loading, progress, ready, stale and error states. Source reading, IR preparation and convolution SHALL run outside the callback. Only a complete result for current dependencies SHALL become ready; superseded, failed or cancelled work SHALL not replace it.

#### Scenario: Generate during processing
- **WHEN** an automatic or manual render runs while the host processes audio
- **THEN** live dry passthrough and mapping remain responsive and no convolution, file I/O, blocking wait or result-buffer allocation occurs in the audio callback

#### Scenario: Request changes before completion
- **GIVEN** a render is in progress
- **WHEN** source selection or IR changes, or a newer request supersedes it
- **THEN** the old completion cannot become the current ready result

#### Scenario: Render fails or exceeds resources
- **WHEN** a read, allocation or render fails, or exceeds the displayed supported resource budget
- **THEN** the plugin reports an error and retains the last completed result without describing it as current; it never silently truncates the request

#### Scenario: Automatic first use
- **WHEN** detection accepts a target with complete source and a valid default IR
- **THEN** generation queues and commits automatically without a main-view Generate action

### Requirement: Audition is a manual sequential wet preview
The plugin SHALL provide manual Audition and Stop anchored to the selected committed shaped swell, defaulting to eligible overlapping wet with optional Isolate selected, with Wet Only or aligned contextual Dry + Wet as defined by audition-monitoring. It SHALL play the complete prepared audition interval once independently of Song position. Wet Only SHALL honor the selected anchor's positioned wet window and clip eligible neighbours to it. Short disclosed monitoring safety ramps SHALL follow audition-monitoring. Live dry SHALL pass unchanged. Audition SHALL never change host transport or start automatically.

#### Scenario: Audition while transport is stopped
- **GIVEN** a verified render and a host that continues processing the effect
- **WHEN** the user presses Audition with transport stopped
- **THEN** the selected prepared result plays without requiring host seek or Play; Wet Only starts at its audible start and Dry + Wet includes its aligned captured listening context and configured tail

#### Scenario: Host seeks during audition
- **WHEN** the host seeks or loops during manual audition
- **THEN** the preview continues sequentially without retriggering or joining at a Song-relative offset

#### Scenario: Stop and replay
- **WHEN** the user presses Stop and subsequently Audition
- **THEN** the first audition ends and the next begins at the prepared interval's first sample, respecting audible edits and mode without muting live dry input

#### Scenario: Audition is not captured
- **GIVEN** mapping is enabled with zero dry input
- **WHEN** the plugin adds wet or captured-dry preview output
- **THEN** capture contains only zero dry input and never generated or replayed preview audio

#### Scenario: Cached wet with unavailable source
- **WHEN** a previously committed wet asset and its saved identity verify but current dry source is unavailable
- **THEN** the UI retains an explicit source-unavailable state and permits only labeled last-committed Wet Only audition, without claiming the render is current or permitting Dry + Wet

### Requirement: Audition has independent protected level control
The plugin SHALL attenuate the safety-ramped aggregate wet branch above -6 dBFS without quiet boost or raw-sample changes. It SHALL expose raw, shaped, attenuation and predicted monitoring peaks, plus saved -60 to 0 dB trim defaulting to 0 dB when absent. Changes SHALL ramp over 5 ms without callback allocation or blocking. Restore SHALL preserve validated saved trim. Dry + Wet SHALL additionally follow audition-monitoring protection.

#### Scenario: Loud imported IR
- **WHEN** the safety-ramped wet branch peak is +7.2 dBFS and new-instance trim is 0 dB
- **THEN** wet branch attenuation is -13.2 dB and Wet Only peaks at -6 dBFS, while stored render samples and capture remain unchanged

#### Scenario: Quiet render and live adjustment
- **WHEN** the safety-ramped aggregate wet branch peaks below -6 dBFS and the user changes audition level while playing
- **THEN** no automatic boost occurs, the level change ramps smoothly without regeneration, and live dry input is unchanged

#### Scenario: Combined output level
- **WHEN** live dry audio or diagnostic tone accompanies audition
- **THEN** the displayed monitoring peak describes only the prepared audition contribution, and its ceiling does not bound combined plugin output

#### Scenario: Restore audition preference
- **WHEN** a project saved at -24 dB audition trim is reopened
- **THEN** that trim is restored, visibly indicated and resettable, and no audition starts automatically

#### Scenario: Legacy state without trim
- **WHEN** supported legacy state has no saved audition trim
- **THEN** it uses 0 dB; an explicitly stored old -12 dB value remains -12 dB

## ADDED Requirements

### Requirement: Requested overlap has genuine rendered wet support
The system SHALL prepare sufficient real wet support for an editable endpoint beyond dry arrival. Where current support is insufficient, it SHALL disclose and verify effective captured render-source extension while preserving the base selection and dry timing. It SHALL not claim padding or stretched old audio as newly rendered wet. Missing coverage or resource limits SHALL retain the request with an explanation.

#### Scenario: Endpoint fits existing raw support
- **WHEN** an endpoint, fade or tension edit fits the verified raw render
- **THEN** only shaped wet preparation changes, with no new convolution or raw render modification

#### Scenario: Endpoint requires more support
- **WHEN** the requested wet endpoint extends beyond the current raw end
- **THEN** the system retains the base selection, shows effective additional render-source context, and renders from that complete committed input using the existing exact length/alignment convention

#### Scenario: Extension input is not captured
- **WHEN** any required effective render-source samples are unavailable
- **THEN** the requested endpoint remains visible as pending or unavailable, and neither a shorter old render nor invented wet samples are described as current support

#### Scenario: Extended dependency changes
- **WHEN** recapture changes samples inside the effective extension while the base selection is unchanged
- **THEN** the dependent wet render is invalidated and refreshed under the automatic policy, while unrelated swells and dry source placement remain unchanged

### Requirement: Wet transition shaping preserves raw convolution
Wet bounds, fades and tension SHALL shape verified raw support nondestructively, honoring exclusive bounds and neutral legacy curves without changing dry or IR identity. Saved placement SHALL translate wet audio/envelope together while preserving logical crop and raw PCM identity. Support checks SHALL invert placement; convolution SHALL occur only for changed raw dependencies or additional genuine support.

#### Scenario: Independent overlap and fade duration
- **WHEN** an endpoint is moved beyond arrival and fade-out duration is edited separately
- **THEN** the prepared wet uses both requested settings, preserves the fixed dry arrival, and does not replace overlap duration with fade duration

#### Scenario: Save and restore shape intent
- **WHEN** a session with a gap or overlap, nondefault fade tension and pending render extension is reopened
- **THEN** requested edits survive validation/restoration, ready status requires genuine support, and no audition begins automatically


### Requirement: Wet placement reuses immutable convolution
Pure translation of a swell's wet audio/envelope SHALL preserve the logical crop, raw asset checksum, IR, base source and exact N+M-1 convolution result. Positioned output and logical support SHALL use distinct checked coordinates. Extra support SHALL only be requested when an independent crop extension needs new real input; translation SHALL NOT be interpreted as a source extension, padding, resampling or time stretching.

#### Scenario: Move wet without regenerating
- **WHEN** a complete wet swell is translated across or before its fixed dry arrival
- **THEN** its samples and shape retain their order/duration at the new position, render count/checksum remain unchanged and dry source coordinates stay fixed

#### Scenario: Extend an endpoint after moving
- **WHEN** a translated swell's endpoint is extended beyond its positioned raw support
- **THEN** the support request uses the inverse placement transform, preserves the base selection and placement, and requires genuine captured input with explicit waiting/error rather than fabricated wet samples

### Requirement: New swells have a gentle initial wet release
New manual and automatic swells SHALL start with zero placement, a 20 ms wet overlap after fixed dry arrival and a 20 ms Smooth wet fade-out at neutral tension, resolved to valid integer samples at the compatible sample rate. Defaults SHALL use real support and disclose waiting if needed. Existing saved swells and edits SHALL NOT be reset by restore, reanalysis or regeneration; monitoring click prevention SHALL remain separate from musical fades and immutable raw identity.

#### Scenario: Create a new swell
- **WHEN** a manual or automatic swell is first created
- **THEN** its initial wet release extends 20 ms past dry arrival and fades over that release, with valid short-interval duration clamping and unchanged dry level/timing

#### Scenario: Reopen or regenerate an existing zero-fade swell
- **WHEN** an older or edited swell is restored, reanalyzed or regenerated
- **THEN** its saved endpoints/fades/tensions/IR and placement remain intact (legacy placement zero), and only disclosed monitoring boundary protection is added to manual replay
