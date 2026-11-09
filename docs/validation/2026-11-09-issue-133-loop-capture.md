# Issue #133 Loop capture validation

## Rack control and sequenced End Signal

Rack Free 2.6.6 ran Host-FX 2.1.0, Fundamental 2.6.4 and Impromptu Clkd 2.5.0 at 48 kHz. Clkd ran at 137 BPM. The Host-FX setting of 256 produced 128-frame callbacks in the trace probe. The probe measured zero sample offset between the Clock, sequencer commands and the corresponding electrical reference lanes. See the [fixture and mapping](issue-133-rack-fixture/README.md).

The headless Rack sequenced proof ran the Release VST3 with only Clock, Step 1 and Step 4 as control inputs. With Recapture Audio off it published exactly one revision across repeated sequencer cycles. The accepted source had four beats, 84,088 stereo frames at 48 kHz, and 42,531 frames in audio ranges. Independent verification checked contiguous coverage and the SHA-256 of each WAV. The proof bundle SHA-256 was `2bef9d4f26e61ea99d2610fe093069c76e0f54943807222eadd17a2f5285f94e`. The visible Duration probe also showed Step 4 cancelling before the configured end.

The final review bundle repeated the headless proof for End Signal and Duration. The Duration fixture disconnected Step 4 from Capture Stop so the take could reach its configured one-bar end. Both published a single 84,088-frame stereo revision with matching checksums. The final bundle SHA-256 was `cda799103466be6d5a0f0002881c3b506d43607cb1b3d164713e227150b11b6c`.

The audible manual and sequenced examples are local files in `out/issue-133/`. The sequenced example starts with Clkd stopped so the owner can choose End Signal before the first Step 1 trigger. The headless test verified the audio supplied to capture. Physical monitoring depends on the selected Audio 2 device.

The owner used the hosted Learn Arm toggle in the Clock-only proof with ASIO4ALL v2 active. Arm off accepted a three-bar Loop. Rack autosave pointed at revision 1 with 12 beats and 252,263 stereo frames at 48 kHz. The saved ranges were contiguous and the WAV SHA-256 values matched the manifest. The fixture contained only the Clock note mapping and stereo audio at the plug-in input. The owner armed a replacement with Recapture Audio on and stopped Clkd while recording. Rack autosave confirmed the clock was stopped, the accepted reference was unchanged and there was still just one published manifest.

## Native and owner verification

The Release processor test verifies UI and external Arm admission, variable callback sizes, End Signal finishing on the following downbeat, Duration Stop cancellation, Recapture Audio protection and replacement, clock-loss cancellation, and retained source identity after restore. The Learn native fixture verifies the existing Arm latch and Recapture Audio switch at 100% and 125% presentation scale. The owner approved the native End Signal preview on 2026-11-09. Physical Windows DPI beyond the preview remains unverified.

The warm Windows Release wrapper passed all 98 tests in 353.41 seconds on the final product sources. The preceding Release run passed 97 tests and reported the previously observed Windows foreground-focus failure in the Perform knob reset fixture. The full rerun passed that fixture. An incremental Release rebuild and focused processor test then passed additional assertions for Stop while waiting and callback suspension while End Signal was pending.

The Recapture Audio on replacement case used the native processor fixture with the same Step 1 and Step 4 MIDI roles. The Rack sequencer proof verified first capture and protection with Recapture Audio off. Other DAW integrations remain unverified.
