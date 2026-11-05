# Swell source-edge conditioning

New wet generation applies a linked stereo 1 ms raised-cosine taper to each end
of the worker's private extracted source copy before reverse convolution. This
reduces cut-edge excitation of sharp IR reflections when several swells overlap.
It changes neither captured PCM nor dry playback/calibration reference samples.
No limiter, lookahead, extra host latency or output gain change is introduced.

At rate R, the nominal ramp is round(R * 0.001) frames (44/48/96 frames at
44.1/48/96 kHz). Cap each ramp at floor(N/2); for L >= 2 use
`0.5 - 0.5*cos(pi*i/(L-1))` at each end, mirrored and shared by both channels.
Inputs under four frames, or rates yielding a ramp under two frames, retain
their samples. Ramps do not overlap; all other samples are unchanged. Existing
readers validate source PCM. Cancellation discards the worker copy. The generic
convolver still computes exact full linear convolution with its supplied input.
Result length, absolute source coordinates and negative-time lead-in are unchanged.

New managed render dependencies are `source-edges-1ms-v1:` plus the original
source fingerprint; this participates in existing content identity hashing.
Asset engine/codec and session schema are unchanged. Verification accepts an
exact legacy source dependency or the current prefixed dependency, with the
same source/IR/timing checks. Unknown processing prefixes do not match. Original
source fingerprints still guard publication against recapture races.
Automatic generation's fallback for metadata without a separate generation
fingerprint strips the known processing prefix before comparing source content;
unchanged capture does not become a spurious regeneration request.

Opening an existing project preserves its verified saved wet sound. Use
**Regenerate All** once to upgrade older unfrozen swells using captured source;
no recapture is needed. Current ready renders are skipped by All. Regenerate
Selected explicitly rerenders selected unfrozen swells. Frozen swells retain
their saved audio until explicitly unfrozen and regenerated. Shape, placement,
identity and user gain edits survive regeneration; musical calibration uses
original dry samples against the new wet render. Previous complete assets are
retained if an upgrade fails or is superseded.

Native coverage: `preverb_source_edges_test` checks rates, short/silent inputs,
linked gains, no helper allocations, cancellation, independent convolution,
legacy restore, explicit upgrade selection, freeze, persistence and original
capture preservation. Generic reverse-convolution tests remain unchanged.
An optional local review invocation takes a stereo source WAV, IR WAV and a new
output WAV path; CTest uses no private audio inputs.
