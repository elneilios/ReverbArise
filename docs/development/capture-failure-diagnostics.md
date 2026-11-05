# Learn capture failure diagnostics

Learn records a provisional pass until DAW Stop validates and accepts one coherent
source/context revision. Failed-pass peaks are discarded; they are not accepted
audio. Prior committed source and older immutable snapshot dependencies remain
authoritative. A failed first pass can therefore clear all visible peaks without
deleting an earlier capture. Temporary/staged files are not automatically adopted
as a recoverable pass, and working cache assets must not be purged as routine data.

## Failure evidence ownership

`CaptureFailureEvent` is fixed-size metadata. The callback is the sole producer of
a 64-slot SPSC mailbox and emits at most one event per pass/epoch. Release/acquire
indices protect copies and slot reuse; a full mailbox refuses publication rather
than waiting or overwriting an unread event. Independently, the failed-pass fence
still rejects incomplete capture. Callback work uses lock-free x64 integer atomics,
bounded copies and no file access, formatting, heap work, locking or logging.
Mailbox exhaustion increments an explicit omitted-event counter in Diagnostics;
an unavailable current cause is never replaced by an older pass's failure reason.

Worker failures use the same monotonic observation sequence. The worker chooses
the earliest observed cause for the active pass, preserving its bounded detail
(512 characters) before abort clears session error state. An observation sequence
records detection order, not proof that a later-observed condition began later.
Pass/epoch and current workflow ownership fence restore, cancel and late completion.

Evidence includes attempted start/count (without calculating an overflowing end),
queued/dropped counts at failure, queue occupancy/capacity, available host timing,
and the initial timing baseline. Sample/PPQ/tempo/meter validity is explicit;
unavailable values are never presented as genuine zeroes. Worker-side string data
adds the accepted project/instance/reference/revision pinned to the pass. Final
drops are separate from the first failure's counters. No PCM is retained by diagnostics.

The most recent failed-pass record survives normal snapshot refresh and successful
retry; Diagnostics labels it historical after the failed pass is no longer current.
Explicit clear/restore retires records from the previous epoch. None of this adds
fields to saved plugin state or capture manifests. Ordinary recoverable storage
exceptions reject the pass and leave the worker available for retry; process crashes
and catastrophic allocation exhaustion are outside that guarantee.
The first cause is published as soon as the worker observes it, even before Stop;
discarding provisional peaks cannot leave a healthy-recording message visible.

## Native harness

Build through `scripts/build.ps1`; the native `preverb_capture_failure_test` uses
disposable asset roots, production queues/workers and deterministic holds. Default
mode exercises rollback/retry and focused injected failures. `--matrix` exercises
accelerated 3-/5-minute song spans; `--realtime` separately paces actual storage
against wall-clock input. The standalone `preverb_capture_failure_mailbox_test`
checks bounded capacity and concurrent metadata copying.

Run native managed-directory fixtures under normal Windows permissions: sandbox
ACL restrictions are not a simulated physical disk failure. For measured commands,
results, skipped cases and host limitations, see the dated issue #54 validation
record in `docs/validation/`. Test fault hooks default to off and are never enabled
by the product UI. No hook should be used against an owner's working cache.

After building, from the repository root:

```powershell
& ./out/build/windows-msvc/Release/preverb_capture_failure_test.exe
& ./out/build/windows-msvc/Release/preverb_capture_failure_test.exe --matrix
& ./out/build/windows-msvc/Release/preverb_capture_failure_test.exe --realtime
```

The matrix covers both rates and all three block sizes, a late injected overflow,
and a selected sparse/loop/silence control. Pacing waits for actual consumption in
accelerated correctness runs; their wall times are not DAW throughput results.
The real-time mode separately runs three minutes of steady input with bounded
five-millisecond staging delays, followed by an overflow injected after 61 seconds.
Output records high-water occupancy, queued/dropped counts and maximum staging,
chunk write-plus-verification, and final manifest validation/publication latency.
Write/verification is measured as one atomic asset-store operation, not attributed
to separate physical disk devices or filesystem flush guarantees.

Optional `--probe-small-blocks` paces 64-frame input at both rates for up to three
minutes, stopping on any naturally observed overflow. It verifies rejection and
retry if pressure occurs, or complete accepted coverage otherwise, without injecting
a worker hold. This probes a possible native pressure path; matching that path to
an unknown FL Studio incident still requires failure-time evidence from the host.

Failure feedback reuses the existing NoticeBar, Learn status/helper typography
and Diagnostics text. The capture diagnostic label measures its content height;
the existing scroll surface grows with it. No control family, token, density or
interaction variant was introduced. Historical evidence is not shown as a current
error after successful retry.
