# Design

## Context

See `proposal.md` for motivation and the `song-timeline-audio` delta for the behavior contract. Investigation baseline is `939f656`.

Observed in code, not yet newly executed: `CaptureWorker::progressSnapshot()` returns no provisional display when `failedPass` matches the active pass. `LearnWorkspace` paints accepted overview and provisional ranges separately. `CaptureSession` stages chunks privately and accepts one prepared source/context snapshot after Stop and ownership rechecks; abort drops staged metadata without deleting committed assets. Thus a failed first pass can lose all visible progress without deleting accepted source. This explains a possible symptom, not the original incident.

`PluginCaptureQueue` has 32 block slots, each accepting up to 8192 frames. The callback currently combines queue-full, bad timing/mapping, channel and block admission failures into one boolean and calls `failPass(pass)`. Storage staging uses five-second batches on the consuming worker. A stall while writing/checking a chunk can exhaust a queue measured in blocks, not seconds. Queue pressure is a hypothesis; no measured incident throughput or exact host error exists.

Existing `capture_transaction_test` covers invalid samples/rate/ranges, staged corruption, publication failure and immutable snapshot restoration. `workflow_processor_test` deliberately holds the worker, overfills the queue, checks overlay invalidation and preserves the accepted reference. Extend these mechanisms instead of building a separate capture implementation.

## Goals / Non-Goals

Integration note (5 November): final validation merges approved main `1f47acf`
(#38). Keep its provisional generation snapshot and settlement callback, compact
Learn/event presentation, and accepted-source Stop policy. Failure fences invalidate
both provisional waveform and generation snapshots. Harness success checks use
`Result::accepted`; they do not require the old automatic Perform transition.
No #38 behavior is redesigned by this change.

**Goals:** Make recurrence diagnosable; deterministically reproduce relevant failure paths and the waveform symptom; prove accepted source/history and retry safety with isolated disposable assets.

**Non-Goals:** Recover or silently accept a prefix of a rejected pass; add checkpointing, persistence formats, queue resizing or a new thread architecture; recreate the owner's song; claim the injected cause was the real incident. Do not require the owner to reproduce the intermittent problem.

## Decisions

### 1. Keep first-failure metadata separate from progress and display status

Introduce typed failure reasons at the failing boundary. Distinguish queue capacity from invalid block size/range, unavailable or incompatible host timing/rate, admission/mapping rejection, nonfinite data, asset write/validation, manifest publication, generation-context pinning and recoverable worker exceptions. Existing detailed storage errors remain attached on the worker rather than replaced by generic text.

Use fixed-capacity, trivially copyable callback evidence with a bounded producer publication protocol: first writer claims a pass-tagged slot, writes plain fields, then release-publishes readiness. Consumers acquire readiness before copying; neither side spins waiting for the other. Keep callback and worker failure latches independently owned and choose the earliest observed failure outside the callback using bounded ordering metadata. Never reset/reuse a slot while its producer can still access it; use pass/epoch tags and ownership tests to fence cancel/restore/retry. Verify atomics are lock-free on supported Windows x64. An unbounded compare/exchange loop or a seqlock retry loop in the callback is not acceptable.

Capture pass/epoch, attempted start/count with overflow-safe range handling, total admitted/queued and dropped blocks at failure, queue depth/capacity, sample rate, sample/PPQ availability and values, BPM/meter, playing/loop state and the pinned timing baseline. Unavailable fields carry validity bits. Preserve accepted source reference/revision in worker-owned metadata pinned to that pass; do not copy strings or inspect assets in the callback. Record final drop totals separately from failure-time values. Bound worker error text and keep only the latest failed-pass record rather than per-block logs or full audio.

Retain the latest completed failure across retries until a newer failure replaces it or explicit session reset/restore retires it. Present it as historical when a new pass succeeds. Failure details must be copied before abort clears session error state, and must not be overwritten by routine snapshot refresh. Process crashes and catastrophic allocation exhaustion are outside recoverable exception guarantees. Catch ordinary worker/storage exceptions at a pass boundary, latch their cause, abort safely and leave the worker available for retry.

Alternative: retain the provisional overlay. Rejected peaks imply usable source and still lose failure context, so retain metadata instead. Alternative: logging each callback block would add pressure and violate callback constraints.

### 2. Keep the existing transaction boundary and truthful error presentation

Continue rejecting incomplete passes and discarding their provisional overlay. Reuse the current `WorkspaceNotice`/shared `NoticeBar`, Learn helper/status roles and capture Diagnostics text. State that the pass was not accepted, whether prior committed source exists, a readable cause and Stop/retry guidance. Developer text carries detailed identities and counters; product copy does not expose implementation details. Existing accepted coverage stays visible. No control family, visual token, geometry, density or interaction variant is proposed. Include component/binding tests, supported-size text fitting and `scripts/check-ui-standards.ps1`; any necessary visual exception requires separate scoped owner review and a prepared native preview before rollout.

Inspect staged `.tmp-*`/immutable segment outcomes in disposable fixtures and document what remains verifiable after rejection. Do not auto-delete, adopt or promise recovery of orphaned files. Saved projects, accepted references and historical manifests remain authoritative.

### 3. Reproduce paths with controlled native harnesses

Reuse `TestDirectory`, synthetic playhead/audio, worker-hold hooks and existing failure fixtures. Use barriers/explicit drain acknowledgments and bounded deadlines instead of timing-dependent sleeps to decide when the failure occurs. Production builds must not enable test fault injection by default. Every failure fixture runs both without prior source and with previously accepted source, including explicit Recapture where relevant; compare reference/revision, context, PCM, asset hashes and restoration of an older saved snapshot. Verify rejected progress is absent, diagnostics survive, then remove the fault and complete a successful retry.

| Case | Injection / assertions |
| --- | --- |
| Queue overflow after progress | Stage at least two chunks, confirm provisional progress, hold consumer, fill beyond 32 slots; retain queue-full context and original reason despite subsequent drops. |
| Timing/block/data | Missing sample/PPQ/tempo/meter, incompatible timing/rate, oversized block, invalid range and nonfinite PCM; assert distinct causes, no invalid memory access and unchanged source. Ordinary valid seeks/loops remain supported and cannot falsely settle. |
| Storage and validation | Block chunk destination or manifest directory inside disposable root, corrupt only newly staged data, inject a recoverable worker exception; retain detailed cause and preserve prior asset hashes. Explicitly label fault simulation rather than claiming physical disk-full coverage. |
| Ownership and settlement | Repeated Stop, cancel or restore during staging/validation, stale pass completion and retry; no partial publication or evidence attached to another session. |
| Multi-minute success/late failure | Feed 3- and 5-minute synthetic song spans at 44.1/48 kHz with representative 64/512/8192-frame blocks, audio/silence transitions and selected seeks/loops; use bounded pacing to avoid accidental queue pressure in correctness cases, then inject a fault after at least 60 seconds of song position. |
| Sustained pressure | Run a separately timed native 3-minute steady-input case at 48 kHz/512 frames with actual storage and calibrated bounded worker delays. Record queued/dropped counts, queue high-water mark, stage/write/validation latency and wall time, separately from accelerated song-duration cases. No hardware-independent throughput claim. |

Keep the full matrix in a dedicated focused harness target or explicit long-test mode if runtime is unsuitable for normal CTest; default regression tests still cover each failure category and retry. Record all skipped combinations. Disable unrelated automatic swell work when isolating capture; retain a bounded processor integration case with normal production bindings so isolation does not conceal callback/worker interactions.

### 4. Require evidence before recovery changes

Land diagnostics and regression coverage even if the original trigger remains unknown, with an honest investigation result. If a test demonstrates a narrow error-propagation, failure-handling or retry defect within this contract, correct it with a before/after regression. A change to admission semantics, durable publication or partial-pass recovery must return to owner review rather than being absorbed into this work.

## Risks / Trade-offs

- Diagnostic races or stale identity -> test publication coherence, concurrent cancel/restore/retry and bounded callback work; read accepted identity only on the worker.
- Accelerated input overwhelms a healthy queue -> distinguish intentionally injected pressure from correctness pacing; label simulated duration separately from real-time throughput.
- Long captures stress provisional summary construction, asset budgets or validation -> record limits/latencies and exact failing branch without assuming a one-minute limit; keep any wider performance migration out of scope.
- Notices overflow existing text roles -> use concise copy and component sizing checks; retain detail in the existing diagnostic surface.
- Synthetic failures differ from FL Studio -> report code observations, executed harness results, hypotheses and untested host paths separately; use one brief final host smoke check only.

## Migration Plan

No project or asset migration. Implement on the dedicated #54 branch, validate focused Debug and Release wrapper runs and required UI checks, then submit one PR for owner review. Revert source/diagnostic changes if necessary; neither deployment nor rollback modifies accepted capture assets. Preserve dated validation evidence and update the living capture guide and documentation index. Do not independently close #54 or claim its real-host root cause is resolved without evidence.

## Open Questions

The original host block size, exact error, timing changes and storage load were not recorded. The harness can establish behavior under known injected conditions; only recurrence with retained diagnostics can tie an unknown host incident to a specific cause.
