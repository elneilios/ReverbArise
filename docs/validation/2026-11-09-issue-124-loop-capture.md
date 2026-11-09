# Issue #124 Loop capture validation

The baseline is `2c3322a` plus the #124 working change. Tests ran on Windows x64 with Visual Studio 2026 and the repository's pinned JUCE source.

## Hosted capture and reopen

VCV Rack Free 2.6.6 ran the Release VST3 with Host-FX 2.1.0 and Impromptu CLKD 2.5.0. The disposable patch used the 137 BPM, 48 kHz, 4/4 lane and the Host-FX setting of 256. The #123 observer measured 128 actual frames for that setting. This capture run did not trace the callback size separately. CLKD drove the shared MIDI reference and stereo dry clock pulses without an audio interface. Use the 137 BPM lane of the #123 alignment patch, point Host-FX at the #124 Release bundle, and select Duration and one Bar in Learn. [Timing fixture and wiring](issue-123-rack-fixture/README.md)

The hosted editor reached Clock ready. Switching Arm on displayed the waiting state. It then accepted a 1.8-second Loop and cleared Arm. The Rack project saved the Loop preference and its exact managed source reference. Independent verification matched the project ID, instance ID and revision to the schema 3 manifest. The record contains 84,088 stereo frames at 48 kHz, 137 BPM and four beats of 4/4. One 140-frame 32-bit float WAV and the contiguous silence span make up the phrase. The verifier checked the WAV header, frame count, nonzero data and SHA-256 against the manifest. Restarting Rack with the saved patch restored the same take with Arm off. Clock readiness returned from fresh markers.

- Release VST3 binary SHA-256: `c13bf4a995fbeb22e67d2ba973c15f2b454aee40e3341a26e04f17e0ec5df8f9`.
- Saved patch SHA-256: `78f3c4b58de67c626601297e027c79c66470b2dee6e53636c426969252a96d0f`.
- Captured WAV SHA-256: `475cbda63b7bc5b3b862d77f689ada3954cda3ba80f8569bd068ca15a60aa8f1`.
- Local test patch, verifier and JSON evidence are in `out/issue-124/rack-validation/`. The Rack license and generated profile are local.

## Native checks and owner review

The native processor fixture exercised the two external clock profiles and a host playhead declaring 121.5 BPM and 4/4 at 44.1 kHz with 512-frame callbacks. It checked exact sample bounds, saved metadata, capture budget, a tempo change during replacement, and preservation of the prior accepted take. The native Learn test exercised the toggle bindings, accepted source, phrase-local automatic markers, and the minimum 900 × 600 layout at 100% and 125% user scale. The owner approved keeping the Loop Mode and Arm toggles and requested host-neutral help. Physical display scaling beyond the hosted window and other DAWs remain unverified.

The host-neutral timing path requires a host that delivers the stereo input and the Loop MIDI markers at their actual sample offsets. The Rack run verifies the external clock route. The declared host timing path has native fixture coverage. FL Studio Loop routing and coverage for other DAWs remain under #127. Raw gate release cannot qualify End signal. Loop playback and export remain separate work.

The full Windows wrapper, `scripts/build.ps1 -Configuration Release -Test`, passed configuration, build and all 98 CTest cases in 394.99 seconds. This included the Song capture and playback regressions and the Perform readability case that timed out on the previous baseline. The completed run reused the build output and used four build jobs. Logs are retained in `out/build-logs/Release/20261109-201126-8614f6a5/`.

## PR #132 review fixes

The native working-folder regression saves and reopens a Song project with a Loop preference, then saves and reopens an accepted Loop in the same working folder. It verifies the source identity, timing, preference and disarmed restore. It also rejects duplicate, unknown and malformed XML children with valid snapshot digests. The capture transaction regression checks cancellation before acceptance and rejection of cancellation after commit.

The revised Release wrapper configured and built successfully. CTest passed 97 of 98 cases in 419.57 seconds, including both review regressions. The only failure was `preverb_perform_knob_reset_test`. The fixture had a native peer and could not acquire Windows foreground focus at `PerformKnobResetCases.h:59`. An isolated retry reproduced the same failure. The earlier working-folder validation recorded the same class of focus failure. This revision has no change to that test or the Perform knob UI. Rack hosting was verified before these fixes. [Earlier focus evidence](2026-11-06-issue-8-working-folder.md)

Logs are retained in `out/build-logs/Release/20261109-204259-8820c3ce/` and `out/issue-124/review-knob-focus-retry.log`.
