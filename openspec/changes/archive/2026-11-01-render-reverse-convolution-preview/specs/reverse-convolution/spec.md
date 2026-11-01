# Spec Delta

## Purpose

Generate a complete, precisely positioned reverse-reverb result from a committed source interval and an imported impulse response, with visual inspection and manual wet-only audition inside the plugin.

## ADDED Requirements

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
The plugin SHALL provide separate source start/end bar controls, with an exclusive end, distinct from the diagnostic tone controls. It SHALL resolve them to one absolute sample interval using host sample/PPQ timing at a fixed tempo and meter. Invalid ranges, unavailable timing, or incompatible capture/host sample rates SHALL prevent generation with an explanation.

#### Scenario: Select after seeking
- **GIVEN** the host is stopped at a later Song location with valid timing
- **WHEN** the user selects an earlier start/end bar interval
- **THEN** generation uses those absolute Playlist bars rather than offsets from the current playhead or capture-chunk start

#### Scenario: Unavailable timing
- **WHEN** the user requests generation without sufficient host timing or with an incompatible capture sample rate
- **THEN** the plugin reports that the bar interval cannot be resolved and preserves existing capture assets

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
For a source of N frames starting at absolute sample P and a prepared IR of M frames, the system SHALL produce the full linear convolution of the source with the reversed IR, with N + M - 1 frames starting at P - (M - 1). It SHALL retain every output sample, including negative-time lead-in, without automatic trimming, normalization, limiting, or fading.

#### Scenario: Leading output before song start
- **GIVEN** the source begins at song sample zero and the prepared IR has more than one frame
- **WHEN** generation completes
- **THEN** the result begins at a negative song position and retains its complete N + M - 1 frames through the source interval's end

#### Scenario: Full tail and numerical reference
- **WHEN** asymmetric synthetic sources and IRs are rendered, including lengths that cross processing boundaries
- **THEN** every result sample matches independent direct convolution within the documented numerical tolerance, with no missing final samples or unreported latency shift

### Requirement: Generation is asynchronous and publishes complete results
The plugin SHALL provide Load IR and Generate controls plus visible loading, rendering progress, ready, stale, and error states. Source reading, IR preparation, and convolution SHALL run outside the audio callback. Only a completed result for the current request SHALL become ready; superseded, failed, or cancelled work SHALL not replace it.

#### Scenario: Generate during processing
- **WHEN** a render runs while the host processes audio
- **THEN** live dry passthrough and mapping remain responsive and no convolution, file I/O, blocking wait, or result-buffer allocation occurs in the audio callback

#### Scenario: Request changes before completion
- **GIVEN** a render is in progress
- **WHEN** the source selection or IR changes, or another generation request supersedes it
- **THEN** the old completion cannot become the current ready result

#### Scenario: Render fails or exceeds resources
- **WHEN** a read, allocation, or render fails, or the request exceeds the displayed supported resource budget
- **THEN** the plugin reports an error and retains the last completed result without describing it as current; it never silently truncates the request

### Requirement: Preview shows source and wet result on one timeline
The plugin SHALL display the generated wet waveform alongside the source overview on a shared absolute Song scale, extending the view for negative-time lead-in. It SHALL show source bounds, result bounds, frame count, and sample rate, with Playlist bar references when supported. It SHALL distinguish a current result from an outdated preview.

#### Scenario: Compare source and leading swell
- **WHEN** generation produces a result beginning before its selected source interval
- **THEN** the wet preview starts at the correct earlier position on the shared scale and its entire length remains visible

#### Scenario: Render exceeds full scale
- **WHEN** raw wet samples exceed full scale
- **THEN** waveform drawing uses the labeled display-only scale `1 / max(1, renderPeak)` to preserve the visible envelope without changing rendered samples or audition level

### Requirement: Audition is a manual sequential wet preview
The plugin SHALL provide Audition and Stop controls for the current ready result. Audition SHALL start from its first sample, advance once through the full wet buffer using processed frames, and stop at its end independently of host Song position. The contribution SHALL contain no recorded dry source; live dry input SHALL continue passing through unchanged. Audition SHALL never request host transport changes or start automatically.

#### Scenario: Audition while transport is stopped
- **GIVEN** a ready render and a host that continues processing the effect
- **WHEN** the user presses Audition with transport stopped
- **THEN** the complete wet result plays from its first sample without requiring the host to seek or play

#### Scenario: Host seeks during audition
- **WHEN** the host seeks or loops during manual audition
- **THEN** the preview continues sequentially without retriggering or joining at a Song-relative offset

#### Scenario: Stop and replay
- **WHEN** the user presses Stop and subsequently Audition
- **THEN** the first audition ends and the next begins at the buffer's first sample, without muting live dry input

#### Scenario: Audition is not captured
- **GIVEN** mapping is enabled with zero dry input
- **WHEN** the plugin adds audible preview output
- **THEN** captured coverage contains only the zero dry input and never the generated preview

### Requirement: Audition has independent protected level control
The plugin SHALL attenuate wet audition peaks above -6 dBFS without boosting quieter renders or modifying rendered samples or dry input. It SHALL expose automatic attenuation, raw render peak, predicted wet audition peak, and a session-local -60 to 0 dB level control defaulting to -12 dB. Level changes SHALL ramp over 5 ms without callback allocation or blocking. Project restore SHALL reset the level to -12 dB.

#### Scenario: Loud imported IR
- **WHEN** the rendered wet peak is +7.2 dBFS and the level control remains at -12 dB
- **THEN** automatic attenuation is -13.2 dB and the wet audition peak is -18 dBFS, while stored render samples and capture remain unchanged

#### Scenario: Quiet render and live adjustment
- **WHEN** a render peaks below -6 dBFS and the user changes audition level while playing
- **THEN** no automatic boost occurs, the level change ramps smoothly without regeneration, and dry input is unchanged

#### Scenario: Combined output level
- **WHEN** live dry audio or diagnostic tone accompanies audition
- **THEN** the displayed audition peak describes only the wet contribution; the -6 dBFS ceiling does not bound the combined output

### Requirement: Preview validity follows its inputs
The system SHALL identify each result by its source dependencies, selected sample interval, prepared IR, and render sample rate. Changes to the selection or IR, overlapping source recapture, capture clear/restore, or a host sample-rate/tempo/meter change SHALL invalidate affected work, stop audition, and prevent outdated output from being auditioned as current. Unrelated recapture SHALL not invalidate an unchanged source dependency.

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

### Requirement: M2 preview is session-local
The system SHALL retain existing saved capture references and tone settings across project restoration without embedding preview PCM in plugin state. Imported IR selection, M2 source controls, and rendered preview SHALL be session-local in this milestone; reopening SHALL require explicit selection and regeneration and SHALL never start audition automatically.

#### Scenario: Reopen an M2 project
- **WHEN** a saved project containing an M2 instance is reopened
- **THEN** existing captured-source state and tone settings restore as before, while the preview is empty and silent until explicitly regenerated and auditioned
