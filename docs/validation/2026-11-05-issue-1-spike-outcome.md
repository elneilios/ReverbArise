# Issue #1 spike outcome — 5 November 2026

**Outcome:** no reliable direct Song/Pattern or upstream-source-mute signal was established for ReverbArise's FL Studio VST3 effect. A script bridge demonstrated realtime mode delivery, but its setup and tested offline policy were unsuitable for shipping. The owner parked that route. Input-silence gating cannot transparently preserve anticipatory swells.

This closes out the investigation record for [PR #74](https://github.com/elneilios/ReverbArise/pull/74), referencing [issue #1](https://github.com/elneilios/ReverbArise/issues/1). It does not implement a fix or select a new product workflow.

## Findings

| Route | Result | Product implication |
| --- | --- | --- |
| Direct JUCE/VST3 timing and lifecycle signals | Inspected APIs and the native host probe established no reliable Song/Pattern or upstream-mute discriminator. Track notifications/reset events also occur during ordinary operation. | Do not infer mode or source mute from loops, timing offsets, resets, callback gaps or generic notifications. This is not proof that no private or future interface could exist. |
| FL controller-script bridge | An explicitly configured script delivered Song/Pattern context to two diagnostic instances in realtime, including reload and disposable save/reopen. Paired controller ports were needed in the tested setup. | Technical delivery worked, but seamless controller-independent onboarding was not established. The owner rejected the setup and parked connector development. |
| Bridge offline rendering | The enabled probe exported digital silence: its Song packet was already about 1,565 ms old at the first offline callback, beyond the 750 ms realtime lease. Both receivers processed 387 offline callbacks without consuming a fresh packet. | The tested realtime freshness policy failed offline. A longer timeout is not a validated fix. This is a diagnostic-policy failure, not evidence of a production #73 regression. |
| Input-activity gating | Synthetic fixtures reproduced identical silent input before a valid future note and a muted note. Closing the gate loses valid lead-ins; startup grace also admits unwanted lead-ins. Holds trade sparse-note preservation for delayed muting. | Activity detection is feasible, but is not reliable source-mute detection. Any optional activity-following effect would need accepted musical compromises. |
| Explicit host controls | In the owner's four-bar-kick probe test, Playlist and Channel Rack mute left generated tone audible; mixer mute and effect-slot disable silenced it. | Explicit output controls work at their respective routing levels. Mixer mute silences the whole path; effect bypass may restore dry in a wet-only workflow. Neither is a source-mute notification or automatically equivalent to a wet-only scheduling control. |

## Evidence and limits

- Investigation baseline: main `520a466`. Windows x64, FL Studio 26.1.7.5654, JUCE 9.0.4 / VST3 3.8.0; controller experiment used MIDI API 45 and embedded Python 3.12.1. The FL executable version was carried forward rather than independently reread for the final mute session.
- Separate diagnostic plugins were built and tested; native Release CTest passed 1/1 for each probe, and 18 Python connector fixture/wiring tests passed. Owner listening/native traces, exported WAV analysis and synthetic activity fixtures are distinct evidence, not a full production regression run.
- The clean Channel Rack mute interval retained 4,261 playing callbacks with enabled buses and no reset/bypass/prepare/release/layout event. One trace row was lost during baseline; no further loss was reported across the marked mute checks.
- Exact toggle latency/residual PCM leakage, other hosts/FL versions, seamless virtual-endpoint onboarding and a corrected bridge offline design were not established. No reliable general detection guarantee follows from these results. Historical captured-source loss remains unconfirmed.

## Closeout

Stop extending the investigated detection routes. Prepared swells can legitimately play through silent input; any change to playback permission, audition or render inclusion is a separate product decision. No new control, default, automation parameter, state migration or canonical specification is approved here. The spike leaves production code, plugin identities and capture/render assets unchanged; issue #1 remains open.

The final PR intentionally retains only this outcome and its documentation-index entry. The full experiment remains available at the [pre-closeout evidence commit `c1e97a5`](https://github.com/elneilios/ReverbArise/tree/c1e97a5fc79484dd4136e56a359dcfcef686c22e/docs/validation), including the [connector findings](https://github.com/elneilios/ReverbArise/blob/c1e97a5fc79484dd4136e56a359dcfcef686c22e/docs/validation/2026-11-05-issue-1-connector-findings.md), [host-signal results](https://github.com/elneilios/ReverbArise/blob/c1e97a5fc79484dd4136e56a359dcfcef686c22e/docs/validation/2026-11-05-issue-1-host-signal-probe.md) and [input-activity counterexamples](https://github.com/elneilios/ReverbArise/blob/c1e97a5fc79484dd4136e56a359dcfcef686c22e/docs/validation/2026-11-05-issue-1-input-activity.md). Probe sources, scripts, metrics and detailed reports are historical experiment material rather than maintained product tooling. Raw WAV/CSV snapshots were local outputs and were not committed.

This closeout consolidates existing evidence; it does not claim new native or host testing. Host transport-start control and any Song Audition/render-inclusion workflow were not established by this spike.
