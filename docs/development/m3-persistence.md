# M3 Persistence

Implementation in progress. Basic persistence targets the same PC and Windows
profile; a saved FL Studio project does not pack the working audio assets.

The existing project/instance cache keeps capture manifests unchanged. New immutable
`irs/` and `renders/` files are float32 WAV with an adjacent JSON descriptor. IRs
retain their decoded channel count; renders retain full stereo convolution audio.
References contain rate, frames, checksum, content identity, engine version (1),
absolute bounds and render dependencies. Canonical decoded sample identity uses
SHA-256 of little-endian rate/channel/length/sample values, normalizing signed zero.
WavPack import is decoded once into canonical managed audio, so its original name,
external location and optional correction file are not needed after successful import.

Writes use unique temporary files and atomic publication before references become
committed. A WAV without its valid matching metadata is unavailable. No existing
asset is overwritten; deleting a definition, Clear, and RAM eviction do not delete
stored audio. Earlier saved snapshots retain their exact references. Cleanup and
Pack Project/Relink remain future work.

The asset tests cover lossless float samples above unity, mono/stereo, equivalent
WAV/WavPack identities, Unicode input paths, original immutability/moves, checksums,
invalid paths, missing publication metadata, blocked writes and earlier references.

Plugin XML state v3 contains metadata format v1 plus existing capture/tone state.
Saving serializes compact records and committed references, without PCM, disk I/O
or waiting for background rendering. Import/render/recapture in flight cannot create
a saved reference to an uncommitted file. Existing assignments remain saved until a
new import commits. Unsupported/malformed state is rejected before replacing valid
definitions or capture references.

Restoration stops audition and clears mapping/arms/jobs. It retains source ranges,
arrival/shape/activity/name/IR/selection preferences, saved fixed timing, analysis
and auto-regeneration preferences, and audition level. First wait for the referenced
capture and compatible host timing; validate assets in the background. Load selected
audio on demand. Missing/corrupt/old-engine render caches rebuild from valid source
and managed IR without recapture. Missing source/IR or incompatible timing is shown
per swell without silently substituting audio; unaffected swells remain available.
Restore never starts audition. General relinking and packing remain M5.

Legacy v1/v2 retain their capture/tone semantics with an empty swell collection and
-12 dB audition default; previously unsaved M2 previews cannot be recovered. An M2
binary cannot read v3 editing state, so retain a pre-M3 project for binary rollback.
No M1 capture-format conversion or destructive cache migration occurs.

Processor tests independently exercise v1/v2, edited v3 reopen at -24 dB, saving
while work is queued, moved external IR, render rebuild, rate mismatch, unsafe state,
older snapshot recovery after Clear, capture isolation and callback buffer retirement.

**Copy asset info** copies the primary swell's exact working directory, IR and render
paths for validation/backup. It does not pack or modify files. Use a fresh test instance
for missing-asset checks and temporarily move files rather than deleting sole copies.
