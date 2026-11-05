## ADDED Requirements

### Requirement: Wet generation conditions source cut edges
New wet renders SHALL apply linked 1 ms raised-cosine ramps to a private source copy before convolution. Capture PCM, dry playback/reference samples, source coordinates and IR gain SHALL remain unchanged. Ramps SHALL shorten for small inputs without overlapping; inputs under four frames SHALL remain unchanged. This processing SHALL add no host latency or output limiting.

#### Scenario: Nonzero source cuts through a transient
- **WHEN** a stereo source with nonzero cut boundaries is generated
- **THEN** both channels receive the same edge gains, first and last samples reach zero for inputs of at least four frames, and samples outside the ramps remain unchanged

#### Scenario: Short or silent source
- **WHEN** an input is shorter than two nominal ramps or contains silence
- **THEN** generation remains finite, preserves its frame count and coordinates, shortens nonoverlapping ramps, and preserves silence

## MODIFIED Requirements

### Requirement: Reverse convolution retains exact length and alignment
For a source of N frames starting at absolute sample P and a prepared IR of M frames, the system SHALL produce the full linear convolution of the edge-conditioned wet source with the reversed IR, with N + M - 1 frames starting at P - (M - 1). It SHALL retain every output sample, including negative-time lead-in, without automatic output trimming, normalization, limiting, or fading.

#### Scenario: Leading output before song start
- **GIVEN** the source begins at song sample zero and the prepared IR has more than one frame
- **WHEN** generation completes
- **THEN** the result begins at a negative song position and retains its complete N + M - 1 frames through the source interval's end

#### Scenario: Full tail and numerical reference
- **WHEN** asymmetric synthetic sources and IRs are rendered, including lengths that cross processing boundaries
- **THEN** every result sample matches independent direct convolution of the conditioned source within the documented numerical tolerance, with no missing final samples or unreported latency shift
