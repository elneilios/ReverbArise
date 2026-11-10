# Rack examples for Loop capture

Use Rack 2 with VCV Host-FX, Fundamental and Impromptu Modular. These patches point Host-FX to a local Windows VST3 bundle. They use the existing Host-FX Note Gate mapping on MIDI channel 1.

| Control | Host-FX Note Gate | MIDI note | Manual patch | Sequenced patch |
| --- | --- | ---: | --- | --- |
| Clock | B0 | 23 | Clkd master clock | SEQ 3 clock, driven by Clkd |
| Start/Arm | C1 | 24 | Learn Arm on | SEQ 3 Step 1 |
| Stop | C#1 | 25 | Learn Arm off | SEQ 3 Step 4 |

Use MIDI numbers in hosts with a different octave naming convention.

The step gates are qualified by clock high to avoid an initial held step producing a command without a beat. The VCA channels give control and pulsed audio equal Rack path lengths. Green cables carry audible audio. Amber cables carry timing. The sequencer's next Step 1 supplies the closing beat after Step 4.

In the sequenced patch, Clkd starts stopped so you can select Loop Mode and End Signal before the sequencer sends its first arm. Press Run after selecting the method.

Generate the examples with the Release ReverbArise bundle. Choose the speakers or headphones in Audio 2. If you use a specific ASIO device, supply its exact Rack device name with `--asio-device`.

```powershell
$bundle = 'out/build/windows-msvc/preverb_plugin_artefacts/Release/VST3/ReverbArise.vst3'
python -B docs/validation/issue-133-rack-fixture/make-patches.py manual --bundle $bundle --output 'out/issue-133/Loop Capture - Manual.vcv'
python -B docs/validation/issue-133-rack-fixture/make-patches.py sequenced --bundle $bundle --output 'out/issue-133/Loop Capture - Sequenced.vcv'
```

## Loop Perform example

Use the `perform` variant to try the #126 Cue, Cancel and Stop commands. It uses
the measured 137 BPM, 48 kHz, four-bar live profile in Host-FX. Clkd triggers
a short snare-like hit once per bar. Two timing cables reach Host-FX.
**Input 1 (B0, MIDI 23)** gets one pulse per beat for Learn.
**Input 2 (C0, MIDI 12)** gets the /16 Phrase pulse for Perform.
Leave both connected. Green cables carry audio into Host-FX and out to Audio 2.

```powershell
$bundle = 'out/build/windows-msvc/preverb_plugin_artefacts/Release/VST3/ReverbArise.vst3'
python -B docs/validation/issue-133-rack-fixture/make-patches.py perform --bundle $bundle --output 'out/issue-126/Loop Perform - Clock and Phrase.vcv'
```

Select your Audio 2 device at 48 kHz, open the Host-FX eye, and capture four bars
using Learn's **Loop Mode**, **Duration**, and **Arm**. Review the markers and click
**Generate Swells**. In Perform, use **Dry + Wet** routing and wait for **Ready**.
Cue and cancel one queued start, then cue again and let the swells repeat. Click
**Stop Loop** and listen for the dry hits to continue. To exercise sync loss,
unplug the phrase cable while the clock and audio continue, then reconnect it.
After **Ready** returns, Cue again to resume wet playback.

### Sequencer with End Signal

Use `perform-sequenced --steps 4`, `--steps 8` or `--steps 16` in the same
command to generate the three snare patches. They use Impromptu GateSeq64
in 4×16 mode with one pulse per step. Track 3 sends Clock each step. Track 1
sends Step 1 to Start/Arm and Phrase, and Track 2 sends the last step to
capture Stop. Track 4 plays the snare on the first beat of each bar.

Select **Loop Mode → End Signal** before starting Clkd. Press **Run** on Clkd and
wait for Step 1 to arm Learn. Clicking **Arm** in Learn starts a manual take on
the next beat, so leave the button to the sequencer in this test. The last step
requests Stop and the next Step 1 closes the take. The 4-, 8- and 16-step patches capture
one, two and four bars respectively. Leave **Recapture Audio** off to preserve
the first accepted take. All three lengths can be cued in Perform when the
Phrase signal repeats at the captured length. Leave Clkd running after the
take is accepted. The plug-in verifies two matching Phrase intervals while
you work in Learn, and Cue becomes available in Perform once both the
reference and wet audio are ready.

On 10 November 2026, headless Rack Free 2.6.6 with Host-FX 2.1.0,
Fundamental 2.6.4 and Impromptu 2.5.0 delivered 128-frame callbacks at
48 kHz. The timing probe observed two complete phrase intervals per
patch. The manual patch's Phrase edges shared their Clock sample. In
each sequencer patch Step 1's Arm and Phrase shared the Clock sample,
and capture Stop shared the last step's Clock sample.

The production Release plug-in also saved fresh End Signal takes in
headless Rack using the same sequencer wiring:

| Steps | Accepted bars | Accepted frames at 48 kHz |
| ---: | ---: | ---: |
| 4 | 1 | 84,088 |
| 8 | 2 | 168,175 |
| 16 | 4 | 336,350 |

Each result was revision 1 and contained audio. The native Perform fixture
checks Cue, Cancel, Stop and recovery when only Phrase is interrupted.
Audible desktop verification of the simplified patch is a separate check.

## Host input measurement

Use the same generator with `probe` and the Release [Rack Host Admission Trace bundle](../issue-123-rack-fixture/README.md). Run Rack at 48 kHz in a disposable profile with the licensed Host-FX, then inspect the trace. This variant sends separate copies of Step 1 and Step 4 to the two dry lanes to measure alignment. It duplicates two events on the legacy Beat and Perform Stop notes for comparison.

On Rack Free 2.6.6, Host-FX 2.1.0, Fundamental 2.6.4 and Clkd 2.5.0, the 137 BPM trace contained 22 Clock rises and five rises each for Start/Arm and Stop. All ten command rises shared the sample of their corresponding Clock and dry reference. The five Start/Arm and five Stop falls generated note-offs with no repeated note-ons while each gate was held high. Legacy Beat and Perform Stop were observed as distinct note numbers. Actual callbacks were 128 frames at 48 kHz. The earlier #123 unplug test measured a note-off with the source gate still high. The #133 capture command decoder accepts positive edges only.
