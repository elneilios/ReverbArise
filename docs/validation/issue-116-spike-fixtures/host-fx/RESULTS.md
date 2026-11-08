# Host-FX input spike for ReverbArise #116

7 November 2026. Rack Free 2.6.6, Host-FX 2.1.0, Fundamental 2.6.4, Windows x64. Host-FX worker enabled. The test used a separate Rack profile and standalone probe builds. ReverbArise source checkout was `be002a399ffc0550c511614583234e885663b564` and the shared JUCE source was `3fb5cbc0340758596dac39f7c8de47d562b4896e`.

## Findings

Host-FX **Note Gate** delivered every observed 1 ms pulse edge and held-gate edge at the same sample as the corresponding audio input in the 48 kHz comparison. It delivered both note-on and note-off even when both fell within one VST3 callback. The two Note Gate mappings can distinguish a capture gate and a phrase-start pulse by MIDI note number.

CV mapped to ordinary VST parameters delivered all observed held-gate transitions, but their apparent positions moved to callback boundaries. Many short pulses disappeared at larger callback sizes. The pinned JUCE VST3 wrapper takes the final ordinary parameter point in each queue and exposes a block value to the processor. These measurements describe the processor-visible path, not Host-FX's raw parameter queue. The negative offsets are relative to audio inside the callback, not negative end-to-end host latency.

## Clean 48 kHz comparison

The approximately eight-second held gate and approximately 1 ms pulse were each split to a control input and an audio channel of the same VST3. The pulse train used 9.97 Hz to vary phase relative to audio blocks. The aligned phrase case used a VCV Gates rising-edge output driven by the same held gate. Its reference rose one sample after the held-gate reference on all five observed rises, including startup. Each MIDI edge matched its own reference.

| Input route | Host setting / actual callback frames | Held-gate edges matched | Short-pulse edges matched | Gate offset in samples | Pulse offset in samples | Trace prefix |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Parameter | 16 / 8 | 9/9 | 1478/1478 | −14 to −1 | −15 to 0 | `25b33eb2` |
| Parameter | 128 / 64 | 9/9 | 1110/1476 | −63 to −13 | −63 to 0 | `eaae211e` |
| Parameter | 256 / 128 | 9/9 | 556/1476 | −127 to −77 | −127 to 0 | `2c7f4d51` |
| Parameter | 512 / 256 | 9/9 | 278/1476 | −255 to −89 | −255 to 0 | `687c727e` |
| Parameter, aligned phrase | 256 / 128 | 8/8 | 6/8 | −127 to −78 | −117 to −12 | `1060e787` |
| Note Gate | 256 / 128 | 9/9 | 1476/1476 | 0 | 0 | `c521d453` |
| Note Gate | 512 / 256 | 9/9 | 1476/1476 | 0 | 0 | `853e0f2b` |
| Note Gate, aligned phrase | 256 / 128 | 8/8 | 8/8 | 0 | 0 | `7bb80940` |

At 44.1 kHz the automation stress cases delivered 2264/2264, 1582/2262 and 380/2262 pulse edges at actual callback sizes of 8, 64 and 256 respectively. All 42 held-gate edges matched across those three cases. The aligned automation case at 128 frames delivered 6/12 pulse edges and 13/13 held-gate edges. Note Gate was measured at 48 kHz. With this no-audio-device Rack profile, selecting 48 kHz during the run left the processor at 44.1 kHz. A Rack restart initialized the processor at 48 kHz.

The paired MIDI gate durations matched the audio durations exactly. No unmatched extra edges or trace queue drops were reported in the comparison windows. The parser passed a synthetic case with two matched edges, signed offsets and a missing 48-sample pulse. The separate note probe and automation probe both built in native x64 Release using Visual Studio 18 2026, MSVC 19.51 and SDK 10.0.26100.0.

- Automation probe binary SHA-256 `D8476B3A3158783EADC93F88D04053F06390DF0EDC7031C704797747E19FE090`
- Note probe binary SHA-256 `A4ECA316B0332FB1D5671A1AD3039E01B38E65C1DC1E03E4D5E1B2403B1972E0`

## Scope of the result

Note Gate is the candidate for sample-positioned Loop commands in a ReverbArise prototype. Rack emitted note-on during patch load when the source started high, so the prototype needs an explicit arming rule and a fresh rise after low. A repeatable synthetic fixture and raw CSVs are here alongside [results.json](results.json) and the [run instructions](RUN.md).

This run did not verify production capture, tempo and meter mapping, an actual musical sequencer over a long session, disconnect and bypass recovery, stop/reset during a take, or FL Studio MIDI and export. Gate 1 remains open for agreed limits and those host checks.
