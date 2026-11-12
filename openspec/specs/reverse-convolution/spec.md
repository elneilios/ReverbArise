# reverse-convolution Specification

## Purpose

Generate a complete, precisely positioned reverse-reverb result from a committed source interval and an imported impulse response, with visual inspection and manual Wet Only or contextual Dry + Wet audition inside the plugin.

## Requirements

### Requirement: Wet generation conditions source cut edges
New wet renders SHALL apply linked 1 ms raised-cosine ramps to a private source copy before convolution. Capture PCM, dry playback/reference samples, source coordinates and IR gain SHALL remain unchanged. Ramps SHALL shorten for small inputs without overlapping; inputs under four frames SHALL remain unchanged. This processing SHALL add no host latency or output limiting.

#### Scenario: Nonzero source cuts through a transient
- **WHEN** a stereo source with nonzero cut boundaries is generated
- **THEN** both channels receive the same edge gains, first and last samples reach zero for inputs of at least four frames, and samples outside the ramps remain unchanged

#### Scenario: Short or silent source
- **WHEN** an input is shorter than two nominal ramps or contains silence
- **THEN** generation remains finite, preserves its frame count and coordinates, shortens nonoverlapping ramps, and preserves silence

### Requirement: Impulse responses have defined preparation
The system SHALL accept non-empty mono or stereo WAV and lossless WavPack PCM/float impulse responses, convert them to the source capture sample rate, and retain their leading silence and gain. Mono IRs SHALL apply independently to both source channels; stereo IRs SHALL apply left to left and right to right without crossfeed. Unsupported, unreadable, or non-finite inputs SHALL report an error.

#### Scenario: Prepare a mono IR at another sample rate
- **GIVEN** a mono WAV IR has a different sample rate from the selected stereo capture
- **WHEN** the user loads it for generation
- **THEN** its duration is preserved to the nearest output sample and its prepared response is applied independently to both source channels without automatic normalization or silence trimming

#### Scenario: Stereo channel isolation
- **GIVEN** a stereo IR with different left and right responses
- **WHEN** a stereo source is rendered
- **THEN** each source channel is convolved only with its corresponding IR channel

#### Scenario: Invalid IR
- **WHEN** an empty, corrupt, multichannel beyond stereo, or non-finite WAV/WavPack IR is selected
- **THEN** the plugin reports the reason and does not replace a valid result with partial output

### Requirement: WavPack detection does not depend on filename extension
The system SHALL recognize WavPack content by its header and allow selection of `.wv` and `.wav` IR files. Equivalent decoded WAV and lossless WavPack inputs SHALL produce equivalent prepared samples without gain normalization. Truncation, integrity failures, and hybrid streams without their required lossless correction data SHALL report an error instead of publishing altered or partial IR audio.

#### Scenario: WavPack content is named as WAV
- **GIVEN** a mono or stereo lossless WavPack IR has a `.wav` filename
- **WHEN** the user loads that IR
- **THEN** it is decoded as WavPack and becomes available for generation without renaming or external conversion

#### Scenario: Integer and floating-point preservation
- **WHEN** equivalent WAV and lossless WavPack PCM/float IRs are loaded
- **THEN** they retain channel order, rate, frame count, leading silence, and decoded gain, including finite float values above unity

#### Scenario: WavPack validation failure
- **WHEN** a WavPack IR is corrupt, truncated, exceeds the preview memory budget, or requires missing correction data
- **THEN** loading reports a specific error and preserves the last complete IR/result without silent lossy fallback

### Requirement: Source selection uses absolute Playlist bars
The plugin SHALL provide separate source start/end bar controls, with an exclusive end, distinct from diagnostic tone controls, and sample-accurate source editing for each selected swell. Bar entries SHALL resolve to absolute samples using fixed host tempo/meter; precise edits SHALL not be rounded to whole bars. Invalid ranges, unavailable timing, or incompatible capture/host sample rates SHALL prevent generation with an explanation.

#### Scenario: Select after seeking
- **GIVEN** the host is stopped at a later Song location with valid timing
- **WHEN** the user selects an earlier start/end bar interval
- **THEN** generation uses those absolute Playlist bars rather than offsets from the current playhead or capture-chunk start

#### Scenario: Unavailable timing
- **WHEN** the user requests generation without sufficient host timing or with an incompatible capture sample rate
- **THEN** the plugin reports that the bar interval cannot be resolved and preserves existing capture assets

#### Scenario: Edit a sub-bar source
- **WHEN** a selected swell's source handle is moved between grid lines with snap off
- **THEN** its exact sample interval is rendered without rounding it to the bar-entry controls

### Requirement: Rendering reads a complete committed source interval
The system SHALL render from one identified committed capture revision, preserving covered silence and exact stereo sample order across chunk boundaries. Uncaptured gaps, missing assets, and invalid samples inside the requested interval SHALL cause generation to fail rather than become silence. New capture commits SHALL not alter an already running render's source.

#### Scenario: Source crosses chunk boundaries
- **GIVEN** a fully covered selected interval spans several contiguous chunks and captured-silence ranges
- **WHEN** the interval is rendered
- **THEN** its samples match the committed source in exact order, including zeros for covered silence

#### Scenario: Source contains an uncaptured gap
- **WHEN** any part of the selected interval is uncaptured or references unreadable audio
- **THEN** generation reports the unavailable interval and does not publish a partial swell

#### Scenario: Capture changes during rendering
- **GIVEN** generation has accepted a committed source revision
- **WHEN** capture publishes a newer revision
- **THEN** the running render remains internally consistent with its accepted revision

### Requirement: Reverse convolution retains exact length and alignment
For a source of N frames starting at absolute sample P and a prepared IR of M frames, the system SHALL produce the full linear convolution of the edge-conditioned wet source with the reversed IR, with N + M - 1 frames starting at P - (M - 1). It SHALL retain every output sample, including negative-time lead-in, without automatic output trimming, normalization, limiting, or fading.

#### Scenario: Leading output before song start
- **GIVEN** the source begins at song sample zero and the prepared IR has more than one frame
- **WHEN** generation completes
- **THEN** the result begins at a negative song position and retains its complete N + M - 1 frames through the source interval's end

#### Scenario: Full tail and numerical reference
- **WHEN** asymmetric synthetic sources and IRs are rendered, including lengths that cross processing boundaries
- **THEN** every result sample matches independent direct convolution of the conditioned source within the documented numerical tolerance, with no missing final samples or unreported latency shift

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

### Requirement: Preview shows source and wet result on one timeline
The plugin SHALL display the generated wet waveform alongside the source overview on a shared absolute Song scale, extending the view for negative-time lead-in. It SHALL show source bounds, result bounds, frame count, and sample rate, with Playlist bar references when supported. It SHALL distinguish a current result from an outdated preview.

#### Scenario: Compare source and leading swell
- **WHEN** generation produces a result beginning before its selected source interval
- **THEN** the wet preview starts at the correct earlier position on the shared scale and its entire length remains visible

#### Scenario: Render exceeds full scale
- **WHEN** raw wet samples exceed full scale
- **THEN** waveform drawing uses the labeled display-only scale `1 / max(1, renderPeak)` to preserve the visible envelope without changing rendered samples or audition level

### Requirement: Audition is a manual sequential wet preview
Manual Audition SHALL play the selected swell's complete prepared interval sequentially, with overlapping wet or Isolate selected and Wet Only or contextual Dry + Wet per audition-monitoring. Wet Only SHALL clip neighbours to the positioned anchor window. Disclosed safety ramps SHALL apply; live dry SHALL pass unchanged. Audition SHALL never start automatically or change host transport. Enabled Song playback SHALL take priority per transport-swell-playback.

#### Scenario: Audition while transport is stopped
- **GIVEN** a verified render and a host that continues processing the effect
- **WHEN** the user presses Audition with transport stopped
- **THEN** the selected prepared result plays without requiring host seek or Play; Wet Only starts at its audible start and Dry + Wet includes its aligned captured listening context and configured tail

#### Scenario: Host seeks during audition
- **WHEN** the host seeks or loops during manual audition with live playback disabled
- **THEN** the preview continues sequentially without retriggering or joining at a Song-relative offset

#### Scenario: Stop and replay
- **WHEN** the user presses Stop and subsequently Audition while Song playback does not have priority
- **THEN** the first audition ends and the next begins at the prepared interval's first sample, respecting audible edits and mode without muting live dry input

#### Scenario: Audition is not captured
- **GIVEN** mapping is enabled with zero dry input
- **WHEN** the plugin adds wet or captured-dry preview output
- **THEN** capture contains only zero dry input and never generated or replayed preview audio

#### Scenario: Cached wet with unavailable source
- **WHEN** a previously committed wet asset and its saved identity verify but current dry source is unavailable and it was not explicitly frozen while ready
- **THEN** the UI retains an explicit source-unavailable state and permits only labeled last-committed Wet Only audition, without claiming the render is current or permitting Dry + Wet

#### Scenario: Song playback takes priority
- **WHEN** live playback is enabled and Song transport starts during manual audition
- **THEN** manual output transitions out within 5 ms, subsequent Audition requests are inhibited while Song is playing, and the plugin mixes Song-positioned wet without sustained duplication

#### Scenario: Explicitly frozen wet with unavailable source
- **WHEN** a swell was explicitly frozen while ready and its managed wet verifies at the original rate/tempo/meter
- **THEN** it remains eligible for live playback and Wet Only audition without current source or IR
- **AND** Dry + Wet audition still requires genuine captured dry context

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

### Requirement: Preview validity follows its inputs

The system SHALL identify each result by source content dependencies, sample interval, prepared IR, and render rate. Selection changes, overlapping recapture that changes source content, capture Clear, or host sample-rate/tempo/meter changes SHALL invalidate affected work and stop audition. Unrelated or sample-identical recapture SHALL preserve validity. Restored results SHALL be validated against saved dependencies before becoming current. Compatible IR replacements SHALL follow the accepted audition handover in swell-editing.

#### Scenario: Overlapping recapture invalidates a preview
- **GIVEN** a ready result depends on a captured source interval
- **WHEN** committed recapture modifies that interval
- **THEN** the preview is marked stale and must be regenerated before audition

#### Scenario: Unrelated recapture preserves a preview
- **WHEN** committed capture changes only outside the result's source interval
- **THEN** an otherwise unchanged result remains current

#### Scenario: Host rate or musical mapping changes
- **WHEN** the host sample rate, tempo, or meter changes after generation
- **THEN** audition stops and the old result is not reinterpreted at the new rate or silently remapped to new bars

#### Scenario: Unchanged recapture
- **WHEN** recapture changes storage revisions but the selected source samples are identical
- **THEN** a result with otherwise unchanged dependencies stays current

#### Scenario: Pending replacement has no accepted preview
- **WHEN** an IR selection is pending and there is no compatible accepted preview
- **THEN** the candidate remains unavailable to Audition until its complete render and monitoring mix are ready

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
