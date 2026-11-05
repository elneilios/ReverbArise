# Issue #54 capture failure investigation

## Scope and baseline

Investigation baseline: `939f656`, branch `codex/54-learn-capture-diagnostics`.
Windows x64, Visual Studio 18 2026, bundled CMake 4.3.1, JUCE 9.0.4.
This worktree starts with a cold build tree and reuses the pinned JUCE/WavPack
source directories from the main checkout through ignored local CMake toolchain
configuration. No CMake cache or compiled objects were copied between trees.

The owner's original FL Studio failure remains unobserved: its exact error,
host block size, timing changes and disk load were not recorded. Tests inject
known failures; they cannot identify that original cause by matching a symptom.

## Executed transaction regression

`out/build/windows-msvc/Debug/preverb_capture_failure_test.exe` (native MSVC,
normal Windows permissions) passed both empty-session and prior-source cases.
The fixture stages two five-second chunks, observes provisional peaks, waits
for an acknowledged held consumer and fills all 32 queue slots before rejecting
the pass. Both cases clear provisional progress and accept a later retry.
The prior-source case compares every earlier asset byte-for-byte, preserves the
accepted reference/revision/context and restores the older immutable snapshot
to verify exact PCM. Output: `retained=0 staged-files=4` and
`retained=2 staged-files=6` after retry. Counts include newly staged/retry files;
rejected immutable chunks remain on disk without being accepted source.

Observed code behavior: `progressSnapshot()` deliberately discards failed-pass
telemetry; the accepted map is separate. Thus a failed first capture can clear
all visible audio without deleting any earlier committed capture. This is a
plausible explanation of the reported disappearance, not a root-cause finding.

The first sandboxed configure failed at Windows SDK registry access. A sandboxed
fixture also could not establish usable managed assets; the same binary passed
with normal Windows permissions. The initial cold Debug wrapper build was
intentionally stopped after focused target availability to incorporate source
changes before building unrelated targets. It is not a full build/CTest pass.

## Focused diagnostics and ownership results (5 November continuation)

Native Debug focused builds and direct runs passed:

- `preverb_capture_failure_mailbox_test`: capacity refusal plus 100,000 concurrent
  coherent publications; unread evidence is never overwritten.
- `preverb_capture_failure_test`: 14 worker fault cases (empty/prior source)
  covering nonfinite data, rate mismatch, blocked segment writes, blocked manifest
  publication, exceptions during staging and Stop validation, and staged corruption.
  Later queue-failure symptoms do not overwrite the first worker error; failure-time
  zero drops and final seven drops remain distinct. All cases retry successfully.
- Four cancel/restore cases across held staging and held validation fence obsolete
  work. Repeated Stop cannot accept twice; failure injected while validation is held
  rejects the pass. Context-pin rejection and a 2,048-character thrown error verify
  retry and the 512-character evidence bound.
- `preverb_workflow_processor_test`: all existing assertions plus 18 new fault
  cases (empty/prior source) cover missing sample/PPQ/tempo/meter, tempo change,
  8,193-frame input, overflowing sample range, rate change and unavailable playhead.
  Each verifies exact first-failure counters, retained source, dry passthrough,
  zero callback allocations/deallocations and retry. The existing incompatible-rate
  guard is preserved: prior-source rate-change recovery explicitly restores the
  earlier state at the original rate, retiring that epoch's diagnostics.
- `preverb_swell_ui_test`: existing component/native binding assertions and new
  empty/prior-source failure notice checks pass, including one-line notice fitting
  at 1100/1400/1600 logical widths and no current error after successful retry.
- `scripts/check-ui-standards.ps1`: passed (49 files; 61 bounded existing occurrences).

The repeated native UI suite also passed with screenshots enabled. Inspected
`out/ui54/workflow-failed.png` and `out/ui54/capture-failure-evidence.png`:
accepted waveform remains visible, provisional totals are zero, the concise error
and retry text fit, and the full pass/source/timing/counter record is readable.
Component assertions verify measured diagnostic height and scroll-content bounds.
No new visual exception or control-family review is needed. These software native
images do not establish untested physical DPI or FL Studio host coverage.

Focused targets were built with the wrapper's `Enter-BuildLock` held and no wrapper
active. Target builds are not claimed as a completed all-target wrapper/CTest run.
Disposable fixtures simulate write/publication failures and corruption, not physical
disk exhaustion. The original incident has still not been reproduced in FL Studio.

## Integrated long-duration matrix

Final integration base includes approved main `1f47acf` (#38); source revision
`9e1dc22` preserves its generation snapshots/settlement handler and Learn Stop
policy. Harness success checks accepted source, not the superseded automatic
Perform transition. JUCE source tag `9.0.4` and WavPack commit
`5803634a030e2a11dba602ba057b89cc34486c67` were verified directly.

Native Release `preverb_capture_failure_test.exe --matrix` passed all 19 cases:
3-/5-minute audio spans at 44.1/48 kHz and 64/512/8192 frames, six overflows injected
after at least 61 seconds of song input, and one sparse/loop/silence control.
Thirteen nonfault cases had zero dropped blocks and complete accepted coverage;
six fault cases retained diagnostics, rejected incomplete source and passed retry.
All accepted chunks respect the five-second/1,920,000-frame limits. There were no
skipped matrix combinations. Raw measurements are in
[the matrix JSON](2026-11-05-issue-54-capture-matrix.json).

At 64 frames the provisional summary reaches its 65,536-peak ceiling, clears its
display ranges and reports preview limitation while source capture continues;
the three-/five-minute commits still succeed. This is a second tested explanation
for disappearing provisional peaks, separate from failed-pass invalidation.
The ceiling is 4,096 ranges/65,536 peaks, below 4 MiB of metadata, with no retained
full-song PCM. Neither mechanism proves the original FL Studio incident's cause.

These runs used bounded producer pacing and synthetic song positions, not DAW
real-time throughput. Native compilation was active on a Ryzen 5 7600X (12 logical
processors). Maximum observed staging/write times therefore describe this run's
load only; the 32-slot queue's time budget varies with host block size. The first
sleep-paced exploratory case passed, then that exploratory matrix was intentionally
stopped; it is not counted as a completed matrix or used for a speedup claim.

## Wall-clock input and unforced small-block pressure

Native Release `--realtime` passed: 48 kHz/512-frame steady input accepted a complete
three-minute capture with zero drops, observed queue high-water 5/32, 36 stages,
maximum staging 19.6753 ms, maximum chunk write/verification 15.6577 ms and final
validation/publication 127.525 ms (180.145 seconds wall time). Bounded five-ms
staging delays were enabled; the timer excludes that explicit pre-stage wait.
An acknowledged worker hold after 61 seconds then produced one queue-full drop,
rejected the incomplete pass and passed retry (61.3516 seconds wall time).
See [raw timed results](2026-11-05-issue-54-capture-realtime.json).

Additional `--probe-small-blocks` runs, with no injected hold, observed:

| Paced input | Observed outcome under concurrent Debug compilation |
| --- | --- |
| 44.1 kHz / 64 frames | Queue full at sample 222592, 3478 queued blocks, one drop; 5.11299 s wall time. First staging/write-verification maxima were 92.4894/88.8948 ms, exceeding the approximately 46.4 ms queue duration. |
| 48 kHz / 64 frames | Queue full at sample 182400, 2850 queued blocks, one drop; 3.8637 s wall time. No storage batch had yet completed, so attributing this case to disk writes would be unsupported. |

Both probes passed rejection, retained-diagnostic and successful-retry assertions;
**their captures failed**, rather than being reported as successful captures.
See [raw probe results](2026-11-05-issue-54-capture-small-blocks.json).
The harness uses Windows `sleep_until` producer pacing, not an FL Studio callback;
producer wake-up jitter, worker scheduling, summary construction and competing
compilation can contribute. This proves a native small-block pressure path capable
of producing the symptom, not the original reported episode or a disk-only cause.
The timed harness supplies current synthetic timing; its manual injected events
leave baseline timing explicitly unavailable. Processor fixtures separately verify
the production callback's pinned timing context and unavailable-field behavior.

No queue resize, storage-thread redesign, partial-pass recovery or checkpointing
was introduced. Broader throughput/retention changes require separate review.
The narrow corrections here preserve first-cause evidence through refresh/retry,
show it before Stop, fence late worker failures from newer callback failures,
and contain recoverable staging/validation exceptions without losing accepted data.

## Final validation status

| Check | Result / scope |
| --- | --- |
| Final native Release wrapper | `scripts/build.ps1 -Configuration Release -Test`: **65/65 passed**, 229.83 s CTest; production/test revision `4d2d098`, evidence working tree dirty. Logs: `out/build-logs/Release/20261105-005135-e03a3685/`. |
| Native Debug wrapper | `scripts/build.ps1 -Configuration Debug`: build passed; wrapper testing was not requested. Logs: `out/build-logs/Debug/20261105-003545-39fd0f46/`. |
| Focused native Debug CTest | **9/9 passed**, 161.55 s: workflow processor, capture transaction/failure/mailbox/context, Learn generation/model UI, swell UI and persistent capture. `out/debug-focused54.log`; this is not a full Debug suite. |
| Final native UI supplement | Debug `preverb_swell_ui_test` passed after the scroll-stability correction and regenerated capture screenshots. `out/debug-ui-scroll54.log`. |
| Long harness | All 19 accelerated matrix cases, two wall-clock cases and two unforced small-block probes passed their specified outcome/retry invariants. Probe captures failed as explicitly reported above. |
| UI policy / OpenSpec | `scripts/check-ui-standards.ps1` passed: 50 files, 61 bounded occurrences, unchanged debt inventory. `openspec validate diagnose-learn-capture-failures --strict` passed; `git diff --check` passed. |

Final screenshots: [Learn failed pass](2026-11-05-issue-54/learn-failed-pass.png)
and [capture diagnostics](2026-11-05-issue-54/capture-failure-diagnostics.png).
Native assertions cover numeric queue evidence, no erroneous current notice after
retry, before-Stop failure feedback, provisional generation snapshot invalidation,
text height and scroll position across periodic diagnostic refreshes.

An intermediate cold Release invocation passed 59/60 before main integration:
the new before-Stop regression exposed the worker object compiled before that
fix during iterative work. The subsequent integrated gate passed 65/65, and the
final gate above rebuilt the scroll correction and passed 65/65 again. No failed
intermediate run is presented as final validation. Dependency sources were warm;
the initial worktree native compilation was cold, later builds incremental.
Build-wrapper command-double fixtures were not run (wrapper unchanged).

Completed staged WAVs remain individually verifiable in the disposable fixtures,
but a complete rejected pass/context was not published; automatic orphan adoption
or resumable recovery is not established. The owner's original cache was not
examined, and no user project, captured source or exported WAV was modified.

**FL Studio smoke and physical host/DPI coverage remain untested.** One brief
owner smoke check can cover normal capture/Stop/retry and retained source in the
host; no repetition of the injected automated edge cases is requested. The exact
original error/cause remains unknown. The review PR uses `Refs #54`, leaving the
reliability issue open for recurrence evidence and separately reviewed throughput
or checkpoint design if needed.
