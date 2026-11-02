# M3 Regeneration

Implementation in progress. Source identity hashes canonical finite stereo samples
in exact interval order with rate and absolute bounds. Revision intersections are
dirty hints only; identical recapture retains content identity despite new chunks.
Signed zero and covered silence have the same sample identity as zero-valued audio.
The streaming visitor decodes each intersecting bounded capture chunk once and
never includes unrelated boundary samples or treats uncaptured gaps as silence.

Tests cover identical/changed/unrelated recapture, split chunks, zero-valued audio
versus captured silence, exact interval fragments, missing coverage and cancellation.

Each swell owns an input generation, render dependency and state: empty/importing,
checking, queued, rendering, ready, stale, error or restoring. Changed overlap
temporarily disables audition while source content is checked. Identical content
returns to Ready without reconvolution. Name, navigation and shape edits never
change raw render identity. Stale output is inspectable but cannot audition.

Regenerate Selected explicitly rebuilds selected confirmed definitions. Regenerate
All processes missing/stale confirmed definitions, including muted/disabled ones,
and reuses current results. Failures are per swell; deleted/edited/old-session jobs
cannot publish. Auto-regenerate is off by default, saved as a preference, and only
reacts to committed source changes. Pending work coalesces per swell.

Recapture & Regenerate arms one absolute source interval. Move FL Studio's playhead
there and press Play manually. Passive mapping is suspended; only intersecting dry
samples enter capture, with block offsets preserved. Progress counts fresh committed
coverage, never the old map or queued/dropped/failed blocks. Stop/seek gaps can be
filled on subsequent passes. Complete coverage disarms and refreshes affected changed
swells even when ordinary auto-regeneration is off. Cancel retains actual committed
capture but abandons the arm/follow-up. Reopen never restores an active arm.

One render worker and a shared 256 MiB working budget account for source, prepared
IR, FFT/decode scratch, resident raw/selected shaped audio, waveform summaries and
pinned retired voice buffers. Inactive full buffers unload on the worker; immutable
disk audio remains. Requests that cannot fit fail without truncation. The coordinator
test restores over 256 MiB of logical render references with resident data below
32 MiB, and rejects oversized requests before full source allocation.
