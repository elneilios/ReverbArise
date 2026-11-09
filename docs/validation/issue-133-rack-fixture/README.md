# Rack examples for Loop capture

Use Rack 2 with VCV Host-FX, Fundamental and Impromptu Modular. These patches point Host-FX to a local Windows VST3 bundle. They use the existing Host-FX Note Gate mapping on MIDI channel 1.

| Control | MIDI note | Manual patch | Sequenced patch |
| --- | ---: | --- | --- |
| Clock | 23 | Clkd master clock | SEQ 3 clock, driven by Clkd |
| Start/Arm | 24 | Learn Arm on | SEQ 3 Step 1 |
| Stop | 25 | Learn Arm off | SEQ 3 Step 4 |

The step gates are qualified by clock high to avoid an initial held step producing a command without a beat. The VCA channels give control and pulsed audio equal Rack path lengths. Green cables carry audible audio. Amber cables carry timing. The sequencer's next Step 1 supplies the closing beat after Step 4.

In the sequenced patch, Clkd starts stopped so you can select Loop Mode and End Signal before the sequencer sends its first arm. Press Run after selecting the method.

Generate the examples with the Release ReverbArise bundle. Choose the speakers or headphones in Audio 2. If you use a specific ASIO device, supply its exact Rack device name with `--asio-device`.

```powershell
$bundle = 'out/build/windows-msvc/preverb_plugin_artefacts/Release/VST3/ReverbArise.vst3'
python -B docs/validation/issue-133-rack-fixture/make-patches.py manual --bundle $bundle --output 'out/issue-133/Loop Capture - Manual.vcv'
python -B docs/validation/issue-133-rack-fixture/make-patches.py sequenced --bundle $bundle --output 'out/issue-133/Loop Capture - Sequenced.vcv'
```

## Host input measurement

Use the same generator with `probe` and the Release [Rack Host Admission Trace bundle](../issue-123-rack-fixture/README.md). Run Rack at 48 kHz in a disposable profile with the licensed Host-FX, then inspect the trace. This variant sends separate copies of Step 1 and Step 4 to the two dry lanes to measure alignment. It duplicates two events on the legacy Beat and Perform Stop notes for comparison.

On Rack Free 2.6.6, Host-FX 2.1.0, Fundamental 2.6.4 and Clkd 2.5.0, the 137 BPM trace contained 22 Clock rises and five rises each for Start/Arm and Stop. All ten command rises shared the sample of their corresponding Clock and dry reference. The five Start/Arm and five Stop falls generated note-offs with no repeated note-ons while each gate was held high. Legacy Beat and Perform Stop were observed as distinct note numbers. Actual callbacks were 128 frames at 48 kHz. The earlier #123 unplug test measured a note-off with the source gate still high. The #133 capture command decoder accepts positive edges only.
