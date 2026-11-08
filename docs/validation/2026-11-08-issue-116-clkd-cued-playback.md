# Issue #116 live CLKD cue and repeat spike

Windows x64, VCV Rack Free 2.6.6, Host-FX 2.1.0, Impromptu CLKD 2.5.0 and Fundamental 2.6.4 at 48 kHz. Two local Release VST3 observers captured four bars of gated oscillator audio and prepared a test swell from the capture. An independent control source delivered Cue and Stop through Host-FX, reset CLKD, and suppressed a section of the phrase marker route while the dry patch kept running. Bar and phrase MIDI and the dry reference used equivalent Rack paths.

The probe accepted adjacent phrase lengths within one sample of its captured phrase. It started the 2,048-sample lead-in early enough for the earliest expected marker, held the final lead sample for up to two samples when necessary, and used the actual marker to place the wet downbeat. Reset and an overdue marker cleared sync. The next launch required a fresh Cue after reacquisition. The candidate policy exists only in the isolated probe.

| Nominal BPM | Actual callback frames | Capture frames | Observed regular phrase intervals | Aligned wet downbeats |
| ---: | ---: | ---: | --- | ---: |
| 137 | 128 | 336,350 | 336,350–336,351 | 4 |
| 280 | 256 | 164,571 | 164,571–164,572 | 11 |

All 15 launches and repeat downbeats landed on the matching audible dry downbeat and MIDI marker at zero sample offset. The complete negative lead-in played at each tested entry and wrap. Ten lead-ins held the last sample for one or two samples, and the remaining five completed without a hold. The largest difference between the actual mixed callback samples and the independently reconstructed wet sum was 2.98e-8. The dry right channel passed through unchanged throughout both 96-second traces. The 137 BPM startup cue before lock emitted no launch. In both lanes reset and a dropped phrase marker cleared sync, and the next launch occurred after a fresh Cue. The second run used a fresh isolated profile after the earlier rerun did not produce two complete traces.

This supports testing a marker-based production policy for stable CLKD running at fractional samples per phrase. The test swell used a single short reverse of the captured oscillator, with a 256-sample positive tail. Overlapping production swells, tails across the corrected boundary, late cue, longer-term jitter, rate/tempo changes, physical monitoring, FL Studio and Stuart's full patch remain separate checks.

## Reproduction

Under \`out/loop-mvp-spike/rack/clkd/\`, \`make-cue-fixture.py\` generates \`clkd-cued-playback.vcv\` for the standalone Release \`rack_clkd_cue_VST3\` and \`rack_clkd_control_VST3\` targets. Open the patch with the listed plugins and allow at least 96 seconds of VST3 processing. \`check-cue.py\` compares the emitted output with the captured dry and the Host-FX marker offsets. \`cue-results.json\` identifies the two complete binary traces in the adjacent \`traces\` folder.

- Patch SHA-256 \`333fa86f1ba51083895df7c918e5e9b9b60e8c7c623db65d93644e878a6ed21f\`
- Cue probe SHA-256 \`8081909eca660de783d5632490d41d68734899bde1c42692f47272c88984b0ed\`
- Control probe SHA-256 \`871139f77368084159b189e00d5b2f480dfb545599f752a6abbf430fe9baf32f\`
