# Tasks

## 1. Implement and verify the realtime guard

- [x] 1.1 Add one allocation-free, zero-latency stereo guard. Verify its −1 dBFS sample ceiling, channel linking, unity idle gain, repeated peaks, recovery, reset and identical output across block partitions at 44.1, 48 and 96 kHz.
- [x] 1.2 Apply the guard at the realtime Song and Loop Dry + Wet mix points. Verify headroom and overload against known dry and wet inputs, stop and seek transitions, dry-only passthrough, fixed Loop cue timing, and unchanged Wet Only, Audition, capture and offline output. Update the guide and verify its review record.

## 2. Qualify live playback

- [x] 2.1 Compare Dry + wet Audition and live playback in FL Studio Song and Loop and VCV Rack Loop. Record peak and gain reduction for an ordinary passage and a coincident-transient stress case. Verify the full lead-in and dry attack by listening before accepting the default.
- [x] 2.2 Run the full Windows Release suite and strict OpenSpec validation. Record host coverage and any measured audible or sample-peak limitations for review.
