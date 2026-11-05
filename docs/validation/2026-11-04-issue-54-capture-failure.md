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

## Remaining evidence

Long-duration/throughput runs and final native Debug/Release validation are pending. FL Studio and
additional DPI coverage have not been run. No user project or source asset was
opened or modified.
