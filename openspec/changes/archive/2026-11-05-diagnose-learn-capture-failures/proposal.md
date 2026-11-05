# Proposal

## Why

GitHub issue: [#54](https://github.com/elneilios/ReverbArise/issues/54).
An intermittent real-song Learn failure cleared the visible waveform after approximately a minute; the original error and trigger are unknown. Existing rollback separates provisional progress from accepted source, but generic callback failure reporting cannot explain a recurrence or prove what happened in the owner's session.

## What Changes

- Preserve a bounded first-failure diagnostic record per pass, including reason, ownership, accepted source reference/revision, attempted frame range, queue/drop counts and available host timing, after provisional progress disappears.
- Explain failed provisional capture and retained committed coverage through existing Learn notices and developer diagnostics, including the case where no source had yet been committed.
- Extend the native capture/processor harness with deterministic queue pressure, invalid timing/data, write/publication failures, Stop/cancel/restore fences and successful retry checks. Exercise multi-minute synthetic input and delayed workers without touching the owner's assets.
- Record observed versus inferred behavior, staged-file recoverability, callback safety, native Release results and remaining FL Studio uncertainty. Make only a narrow recovery correction if a defect is demonstrated; require further review for a lifecycle/storage redesign.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `song-timeline-audio`: retain actionable failure evidence independently of provisional display, truthfully report durable coverage, and verify long-pass failure/retry safety.

## Impact

Expected areas: `Source/Capture/CaptureWorker.*`, `AudioCaptureQueue.h`, capture session/store error paths, `Source/Plugin/PluginProcessor.*`, existing Learn/notice/diagnostic bindings and focused native tests. Update living capture guidance and dated validation evidence. Preserve existing immutable assets, source/context transaction semantics, saved identities and real-time safety; no persistence schema change, checkpoint/resume, new control family or independent #53/#38/#46/#8/#25 work.
