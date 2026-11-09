# Rack timing fixture for #123

This fixture adapts the #116 trace observer to log the seven proposed Note
Gate roles, MIDI channel, sample offsets, callback size and sample rate.
It drains the trace on a worker so it also works in headless Rack. Probe
sources are retained here. Keep licensed Rack modules, profiles, generated
patches, build output and raw traces in local ignored output.

1. Build `rack_admission_VST3` in Release with native x64 MSVC. Point
   `JUCE_SOURCE_DIR` at the repository's pinned JUCE checkout and select a
   dedicated build directory such as `out/issue-123/rack-probe`.
2. Use the Release `Rack CLKD Test Controls.vst3` built from the
   [#116 fixture](https://github.com/elneilios/ReverbArise/tree/47cbed2/docs/validation/issue-116-spike-fixtures).
   Run `python -B make-patch.py --probe <admission-bundle> --control <control-bundle> --output <patch.vcv>`
   with absolute bundle paths. Add `--mask-phrase` and a separate output
   directory for the missing-reference variant.
3. Use a fresh disposable Rack profile at 48 kHz with Rack Free 2.6.6,
   licensed Host-FX 2.1.0, Fundamental 2.6.4 and Impromptu CLKD 2.5.0.
   Start Rack with `--headless` and the generated patch, and set
   `RACK_USER_DIR` to the test profile. Keep standard input open for the
   run. Send Enter to stop and drain the trace. Run one Rack process at a
   time. Use at least 175 processed seconds for the extended run and at least
   100 for the missing-reference variant. Verify processed time from the
   trace, since wall time and engine time can differ.
4. Run `python -B check-trace.py <trace.csv> --tempo 137` or
   `--tempo 280` as appropriate. Match the 137 BPM lane to the actual
   128-frame callback trace and 280 BPM to 256 frames. Pass
   `--mask-phrase` when analyzing the variant. A loss, overflow or invalid
   event invalidates the result.
5. Run `python -B check-admission.py` for the contract scenarios. To
   replay a continuous measured epoch, add `--trace <trace.csv>` and
   `--period 46080000/137` or `--period 46080000/280`. For the masked
   trace add `--mask-phrase` to apply its measured path offset and
   expect one loss and a fresh Cue after recovery.

The dry inputs are electrical copies of CLKD phrase and bar pulses for
edge comparison. The earlier #116 cue fixture used audio and matched
paths through the VCA. In the masked variant the phrase MIDI route alone
passes through an additional VCA, so dry leads MIDI by one sample where
the mask is open. The controller sends reset near 36 seconds, closes the
phrase mask near 61 seconds and reopens it near 68, and sends Perform Stop
near 94. The emitted MIDI sample numbers are the measurement.
