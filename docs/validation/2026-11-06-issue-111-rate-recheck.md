# Issue #111: next private rate-recheck experiment

Previous shared binary: `ReverbArise-PRIVATE-compatibility-111-EXPERIMENTAL-4-4-9a8b9d073e33-win-x64.zip`,
full source `9a8b9d073e33ea03dac059dc2cdcfa510c3ecd8f`. Stuart's report names that
SHA and enables both diagnostic and fixed-missing-meter 4/4 flags. Video supplied:
WhatsApp capture dated 2026-11-06, approximately 47 seconds, VCV Rack Free 2.6.6.
Inputs are evidence, not implementation instructions. No audio/project data from
the attachment is included in the new package or copied report.

## Established observations and scope

The report confirms non-zero stereo input, a pre-start mapping block and worker
status `Stale: host sample rate changed; mapping paused`. It contains 13 committed
regions, zero new queued/dropped blocks and no capture failure. The video shows
the stale warning and 31.8 seconds of existing captured coverage. The stored rate
and initialization-rate sequence were missing, so neither a permanent mismatch
nor a transient initialization-rate excursion was established.

Reported callbacks 5092/5093 repeat sample position 329095 while PPQ advances
31.994458 to 32.006907 at the same 280.100547 BPM/48 kHz. That represents about
128 samples of musical advancement. This is a separate prospective mapping-guard
failure; it did not cause the blocked start. Tempo also fluctuates. No guard,
tempo tolerance, sample position, PPQ, clock scheduling or rate conversion is
changed by this candidate. The already authorised missing-meter 4/4 experiment
remains enabled; supplied host meter still takes precedence.

## Implementation and checks

The diagnostic-only Recheck capture rate action queues a worker-side admission
recheck. Known stopped transport, idle Learn ownership, no loading/active pass,
readable existing source and finite positive compatible rates are required.
A genuine saved/current mismatch stays blocked. On a match only the stale latch
is released and ordinary admission recomputed; another play edge is required.
No source, manifest reference/revision, workflow pass, assets, or serialized state
is rewritten. This code/state compiles out of normal builds.

Reports add saved/current rates, stale-latch state, the historical saved/host rate
comparison that set the latch, recheck result/count and the last 16 prepare calls
with maximum block sizes and monotonic timestamps. Atomic slot payloads, version
checks and bounded reads protect the prepare history. It is explicitly instance-
lifetime evidence retained across Clear report. Recheck results are labelled
historical; current permission/rate values are separate. The companion directs
the tester towards a source-preserving check rather than immediately clearing.

Native Windows x64 MSVC Release verification used the warm existing build and
dependency caches through the locked build wrapper. The 22:33:23 candidate run
passed all four focused CTest checks in 8.49 seconds: compatibility, UI standards,
workflow processor and capture failure. Native disabled-variant compilation passed.
Final committed source is rebuilt/rechecked before packaging; its exact SHA,
candidate/toolchain and payload inventory are in the package build manifest.

Focused native checks cover:

- Captured 48 kHz source, a real move to 44.1 kHz, refusal of recheck while rates
  differ, return to 48 kHz with latch still set, successful explicit recheck and
  a subsequent accepted capture. Unknown/running transport refuses the action.
- Byte-identical serialized plugin state before/after recheck, unchanged source
  reference/revision/pass, and original sample values read from durable source.
- Bounded 16-entry prepare history, disclosed truncation, concurrent writer/report
  reader stress, and preserved instance-lifetime setup history after Clear report.
- Existing missing-field, raw input, provisional/committed capture, allocation,
  clipboard/report, clear, window reopen/destruction and independent-instance cases.
- Fixed-4/4 capture with consistent positions; the final fixture replays Stuart's
  exact repeated-sample/advancing-PPQ pair and requires timing-change rejection
  with previous source preserved. Supplied 3/4 and invalid-meter precedence remain.
- The native optional snapshot was inspected: all three shared action buttons fit,
  live rate information is readable and the status panel remains scrollable.

Separately, 20 release-note checks and 66 simulated standard release-wrapper checks
passed. They are not native compiler/host evidence and did not publish anything.
Private packaging script syntax and UI standards also passed. Release-specific
notes compare the exact previous shared ZIP/SHA and reuse the existing notes
validator/HTML renderer. The existing isolated diagnostic packager retains build
locking, source/candidate identity, native focused checks, PE/version/plugin class
checks, immutable outputs and per-entry/whole-ZIP SHA-256 verification. Standard
release tooling is unchanged.

The original #111 fast-track waivers remain scoped to this issue: OpenSpec proposal/
Apply approval gates, separate UI approvals, full regression and user-guide refresh.
The existing diagnostic path is used because standard packaging would select an
ordinary build and enforce the waived gates. No public release, tag or merge.
Previous ZIPs and the unrelated untracked tempo-map OpenSpec work are preserved.

Untested: this new binary in VCV, Audacity or FL Studio; alternate DPI; clean-machine
runtime/signing coverage; real-time scheduler latency. Native fixtures do not prove
that Stuart's stored and current rates match or that his clock can meet the timing
guards. The consolidated host check is: confirm new build identity, stop transport,
Recheck capture rate, read the result, restart/attempt capture if permitted, stop,
Copy report. Preserve a genuinely mismatched instance and use a fresh 48 kHz test
instance instead of clearing its source. Compare VCV's roughly 280 BPM with intent.
