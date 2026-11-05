# Song export composition

Issue #6 implementation in progress. `SongExportPlan` is immutable metadata,
published through `shared_ptr<const SongExportPlan>`; it contains no song PCM.
Its builder accepts verified **accepted** swell versions, never requested IR
intent. The engine must resolve a pending replacement before calling it. Required
wet that is not ready blocks the complete standalone composition; host-bounce
available-member policy is unchanged.

Identity includes source manifest/reference, stable ordered member definitions,
render identities, shape/placement/calibration, fixed song mapping, origin/range,
percentage, gain policy/revision and warning ranges. Perform selection, isolation,
trim, routing, current host cursor and names do not define the audio. Pure model
tests cover deterministic ordering, relevant invalidation and unchanged output
on rejected requests. Instance/session/request tokens own async publication; a matching content
identity alone never authorizes publication.

The file range begins at the verified song origin and includes the latest known
source extent or eligible positioned wet end, clipped only before origin.
Checked arithmetic rejects unsupported RIFF size before admitting work.
Asset-byte verification, warned source reading, protection and bounded writing
run on the existing Swell worker. A valid metadata plan alone is not a ready WAV
or proof that referenced files are readable. Callback preview integration and
the dedicated workspace are tracked separately.


## Saved Export preferences

Issue #51 state groundwork adds Export metadata version 2 with independent `dry`
and `wet` levels and per-file gain-law provenance. Unversioned legacy Export
metadata is validated before migration: its preference resets to 0% Dry / 100%
Wet, desired/current references clear, and all immutable history descriptors,
paths and checksums survive as gain-law 1. New descriptors default to gain-law 2.
The composition identity revision is `song-export-v2-independent-levels` and
includes both levels. This prevents matching an older crossfade identity.
The outer creative-state and VST3 identities remain unchanged. The worker uses
the independent pair; production uses the owner-reviewed preset selector and independent sliders.

Creative JSON v10 adds Export initialization/percentage, desired identity,
last completed path and up to 256 immutable file descriptors. Envelope v8 and
capture schema 1/2 remain unchanged. Supported v1–v9 states migrate to an
uninitialized Export preference. First use defaults to Dry 0% / Wet 100%;
subsequent edits/re-entry preserve the independent pair.
Missing, malformed, unsafe or excess reference metadata is rejected before
session replacement. Earlier binaries do not read v10; retain earlier FLP/cache
copies for binary rollback. No exported audio is embedded or deleted.

Export entry uses compatible ready wet independently of dry, with frozen-wet
recovery retaining its existing verification rules. After entry, choosing 0%
does not change mode or enable capture. Learn Stop still automatically enters
Perform under its existing successful-commit contract; #38 is future work.


## Warned dry reading

`ExportSourceReader` is worker-only and uses the existing verified managed chunk
reader. It retains at most one decoded source chunk, with budget checks covering
both planar decode scratch and returned interleaved data. The existing five-second
/ 1,920,000-frame limit remains in force. Pages are at most 4096 frames. Preparation
fixes exact gap/unavailable spans; captured silence adds no warning. Unreadable
source is a warned zero branch, while malformed metadata or insufficient working
budget rejects preparation. Wet-only avoids all source audio reads.

Warnings never rewrite the source manifest or weaken Perform's strict interval
reader. Availability is fixed for a revision; a later read failure on an admitted
asset fails the operation instead of changing an in-progress file into silence.


## Shared window mixer and measurements

`SongMix` pins verified accepted wet file readers, retains one bounded dry chunk
and uses 4096-frame decode/mix pages. The same kernel serves measurement and WAV
writing; arbitrary output partitions do not change member order or per-sample
arithmetic. Wet envelopes, accepted calibration and 2 ms true-boundary safety
ramps use the existing shaping helpers. The weighted sum is measured without
aggregate attenuation or limiting. Sparse zero spans are omitted
from measurement scans while remaining in the declared file interval.

Exact nonzero audible bounds and bounded dry/wet peak bins come from the final
transparent composition; there is no quiet-tail threshold. One shared reference
scale is used for both visual layers. Export does not consume Perform trim,
isolation or live wet level. The existing serialized Swell worker owns preparation and publication; combined
budget stress and final callback integration validation remain pending.

Float32 stereo WAV writing streams from the same measured kernel. Verification
checks finite decoded samples, rate/channels/length and cancellable SHA-256 before
atomic non-overwriting managed publication. Save makes a verified staged copy
and requires explicit permission to replace an external destination. Previously
published managed history is never overwritten. These file operations run only
off the realtime callback; no whole-song sample buffer is constructed.


## Request ownership and recovery

Issue #51 adds an atomic `setExportLevels(dry, wet, session, pass)` request. Both
finite levels are validated before either changes, and one changed pair causes
one request-generation increment. Unchanged pairs do not dirty state. Composition
identity includes both levels; new WAV descriptors and names retain both levels
and the v2 gain-law provenance. Old-law history cannot satisfy a new identity.
Production binds the reviewed independent controls. The old crossfade setter
remains a test/backward-call adapter; it is not a production control.

At 0/0 the engine reports an explicit Silent idle state, stops preview, clears
current prepared/detail authority and disables preview/seek/delivery. Historical
files remain referenced and verified. A nonzero pair with cancelling audio can
still produce a truthful silent file; zero pair intent deliberately offers none.
Dry reads/warnings depend on Dry>0, and wet preparation depends on Wet>0.

The v2 worker sum is `dry * capturedDry + wet * acceptedWet`. Per-swell
calibration, Level, shape and boundary edge treatment remain in accepted wet;
aggregate wet and combined attenuation are removed. Measured peaks may exceed
unity and Float32 WAV samples retain those values. Display normalization is
separate and uses the weighted contributions on one shared scale. Preview
start/stop/seek ramps remain monitoring-only. The 2e-6 sample oracle compares
independent layers and their composite, including unity, intermediate and zero
pairs; previous antiphase, sparse, overlap and file-partition checks remain.

`SwellEngine::prepareExport` snapshots accepted members on the existing worker.
Pending IR replacements resolve to the independently verified retained version;
requested imports never enter an accepted composition mid-job. Eligibility keeps
enable/mute/global-solo policy. Publication checks instance IDs, session epoch,
request generation, workflow pass, source reference, accepted live revision and
compatible timing. Mix gestures coalesce; Stop/mode/restore cannot start preview.
No-op requests and transient cursor/status changes do not dirty creative state.

Desired identity is accepted independently of the last completed reference, so a
save during preparation retains the requested percentage without relabelling the
old WAV. Independently verified matching history may be reused. Missing/corrupt
completed files rebuild to a new unique path when inputs survive, retaining all
historical descriptors. No editor instance is needed for worker publication.
Copied engines share immutable readable history but own future requests.

The initial format is stereo IEEE float32 WAV at the verified source rate.
Unsupported RIFF ranges are rejected before writing (conservatively at
`(UINT32_MAX - 1024) / 8` stereo frames); RF64 and resampling are not implemented.
File frame zero is verified song origin, including a late first event's leading
silence. Negative wet lead-in is clipped without shifting later audio. Exact
nonzero analysis retains quiet tails; an all-zero mix still writes its declared
nonempty interval but cannot be auditioned as an audible region.


## Preview/file equivalence

Export preview decodes the current verified float32 WAV into a sixteen-page
4096-frame stereo ring. The callback never decodes, reads files, owns reference
counts, frees pages or waits for the worker. Only explicit Play intent can arm
prefetched pages. Stop revokes delayed arms; exhausted pages stop preview and
report starvation rather than advancing over missing audio. Mode exit and
restore revoke continuation. A mix change stops/rebuffers explicitly; completion
does not resume playback automatically.

The cursor is an absolute song sample. By default preview starts at the exact nonzero
mixed bound, while the WAV retains its full song-origin interval, lead-in and
declared tails. Interior PCM equals the decoded WAV; only the existing 5 ms
manual preview start/end/Stop ramps differ. The 2 ms wet member-boundary ramps
are already part of the shared composition and file. No page boundary applies
another ramp. Wet aggregate and weighted-sum attenuation are fixed for the whole
revision and use the saved policy; no live wet level or Perform trim is baked in.

Engine callback oracles cover 0%, 50%, 65% and 100% and compare all preview frames
to decoded WAV plus the declared monitor ramps. The reusable metadata/mixer and
paged-voice seam can support future #5, but Perform retains its selected-window
audition behavior in this change. Normal Export DAW processing remains dry
passthrough; internal preview neither starts/seeks the DAW nor captures itself.


## Working-budget accounting

Composition jobs share the existing 256 MiB engine allowance with resident
transport/monitor data. The serialized worker reserves the remaining capacity
for composition and 1 MiB of publication scratch; Save/drag verification reserve
1 MiB independently when admitted. Preview's fixed ring is below 600,000 bytes,
with reader/page scratch counted separately. Completed summaries are counted;
no whole-song PCM is retained. This is tracked working-storage accounting, not
a claim about complete process RSS or arbitrary third-party allocator overhead.

Focused native stress covers all 4096 supported wet members at maximum overlap:
67,222,352 tracked bytes, below its 128 MiB test allowance. A one-hour sparse
interval retains 373,664 tracked bytes with 4096 bounded summary bins and no
hour-sized buffer. A 150,000-frame concurrent producer/consumer test wraps the
ring repeatedly, preserves sample/edge-ramp oracles, and reports zero callback
allocations/deallocations. A held engine preparation remains within the shared
budget; Stop cancels delayed continuation without affecting the WAV.

Save pins epoch, workflow pass and exact completed file. External destination
publication rechecks ownership under the engine lock at atomic replacement;
older/mode-roundtrip chooser intents are rejected. Cleared source does not delete
Export history or delivered files. Restore with missing inputs retains history
and reports unavailable preparation; missing completed output with surviving
inputs rebuilds to a new path. No broad cache/Playlist cleanup is implemented.

## Read-only inspection and last-good presentation

The accepted audible bounds define the initial Fit, with modest visual margins;
the file's song-origin bounds remain separate. Fit, pointer-anchored wheel zoom,
Shift-drag pan and keyboard navigation reuse `SwellViewport`. Navigation is ephemeral
and never changes audio, export percentage or saved state. A newly accepted mix
fits once; ordinary refresh and detail publication preserve manual inspection.

During preparation or failure, the previous accepted waveform remains explicitly
labelled as the previous percentage. Playback, Save and drag still require the
exact current published revision. Current/previous summaries and detail requests
are ownership-checked and counted under the shared working budget, including a
conservative 1 MiB allowance for editor-retained metadata.

Viewport detail is coalesced after a 100 ms quiet interval, capped at 4096 bins
and produced by the existing measured mixer with accepted fixed gains. It does
not reconvolve, rewrite the WAV or change render identities. Superseded range,
session, pass or mix results cannot publish. Preview prefetch is serviced during
detail work; the coarse accepted summary remains usable until detail is ready.

## Preview position (issue #6 refinement)

Unmodified left-click/drag parks an absolute sample within the current accepted
audible range. A stopped seek does not play; Play starts at the parked point.
A playing seek replaces the existing prefetch ticket and continues after worker
verification/refill, using the same 5 ms start/outgoing ramps. Rapid requests
supersede prior work; Stop revokes delayed starts. Old-ticket cursors are hidden
while a new target is preparing. Stop parks its current cursor; natural completion
returns the display to the chosen start. Right-click never seeks.

Position is transient engine metadata, cleared with mix/session/mode changes;
it does not dirty the project, change export/file identity, regenerate wet or
seek the DAW. The callback still consumes only fixed pages. The worker reads the
same immutable WAV from the selected sample, with no new audio engine. Pending
last-good plots cannot seek. Wheel/Shift-wheel, right-drag and Fit remain viewport
actions, independent of preview position. Shared #46 controls are integrated.

PR #49's shared Zoom/Fit row and musical ruler now provide the Export controls;
the earlier deferral is fulfilled, with native owner approval recorded. The adapter exposes accepted span limits and keeps complete-audible Fit.
The row owns only intents; Export guards their session/pass/composition and maps
the logarithmic scale without duplicating limit calculations. The ruler paints
accepted absolute timing, never DAW seeking, Snap or an independent clock.

An unavailable coherent worker timing read waits, as existing transport
preparation does; it must not create a spurious invalid revision and revoke Play.
This does not substitute timing: callback validity/compatibility gates still
apply, and valid but incompatible host mapping remains an error.

If final worker publication cannot confirm the capture identity/timing, it does
not expose Ready or advance the completed reference. One private, request-keyed
ExportFile reference allows a later valid pass to independently verify/reuse the
already published immutable WAV instead of writing a duplicate. Retries re-run
composition/ownership checks; changed keys, restore/Clear or leaving Export discard
the private reference. It contains bounded file metadata, no PCM, and is not saved
or exposed for delivery before compatibility succeeds. Previous accepted files
and last-good display remain intact. No waiting loop blocks the worker.
