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
**Input 1 (B0, MIDI 23)** gets one Clock pulse per beat for Learn and Perform.
The first pulse starts the four-beat count. Green cables carry audio into Host-FX and out to Audio 2.

```powershell
$bundle = 'out/build/windows-msvc/preverb_plugin_artefacts/Release/VST3/ReverbArise.vst3'
python -B docs/validation/issue-133-rack-fixture/make-patches.py perform --bundle $bundle --output 'out/issue-127/fixture/Loop Perform - Clock Only.vcv'
```

Select your Audio 2 device at 48 kHz, open the Host-FX eye, and capture four bars
using Learn's **Loop Mode**, **Duration**, and **Arm**, then press **Run** on Clkd.
To retake while Clkd is running, enable **Recapture audio** and click **Arm**
mid-bar. Recording starts at the next downbeat. Review the markers and click
**Generate Swells**. In Perform, use **Dry + Wet** routing and wait for **Ready**.
Cue and cancel one queued start, then cue again and let the swells repeat. Click
**Stop Loop** to finish that pass without the next lead-in. Click **Stop Now**
while finishing to fade immediately. To exercise sync loss, unplug Clock.
Reconnect it and wait for **Ready**, then Cue again.

To sequence these actions, add Host-FX Note Gate inputs on channel 1:
E0 (16) cues, F0 (17) finishes the current pass, and F#0 (18) stops with a
short fade. Keep the Clock input connected. Perform's footer shows position
within the captured Loop, with the captured BPM and meter.

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
the first accepted take. Leave Clkd running while you generate swells, then Cue
in Perform when Ready. Step 1's optional Phrase pulse can align the bar count
to the sequencer while playback is idle. Clock supplies the running timing.

### Use a clock to automate End Signal

A beat clock by itself does not issue Start or Stop. Use a clock with
phase-offset divided outputs, or drive a resettable pulse counter from the
same clock. For a two-bar phrase in 4/4:

| Host-FX Note Gate | Send |
| --- | --- |
| B0 (23), Clock | Every beat |
| C1 (24), Learn Start/Arm | The first beat of the phrase |
| C#1 (25), Learn Stop | During the last bar, before the closing downbeat |

For example, pulse Start with beat 1 and Stop with beat 8. Keep the beat
clock running; beat 9 closes the take. Leave the Learn Arm button off so
the patch controls the start. Leave Recapture Audio off to keep the first
accepted take. A Phrase cable is optional and only sets the playback origin.
For a known fixed length with only a beat cable and manual Arm, use Duration.
With the full [Impromptu Clocked module](https://marcboule.github.io/ImpromptuModular/#clockedclkd),
the equivalent two-bar setup is master Clock, a /8 output with no delay
for Start/Arm, and another /8 output delayed by 3/4 of its cycle for Stop.
This sends Stop on beat 7, still in time for beat 9 to close the take.
Reset the clock before starting, with its stopped outputs set to low.
Clkd lacks the sub-clock delay control. On 10 November 2026, this three-cable
Clocked setup accepted an eight-beat, 168,176-frame take at 48 kHz and 137 BPM
in Rack Free 2.6.6 with Host-FX 2.1.0. It used no sequencer or Phrase cable.
The local patch, host log and manifest are in `out/issue-127/clocked-end-signal/`.

Generate a fresh copy without saved plug-in state or asset references:

```powershell
python -B docs/validation/issue-133-rack-fixture/make-patches.py clocked-end-signal --bundle $bundle --output 'out/issue-127/clocked-end-signal/Clocked End Signal - Clean.vcv'
```

In the fresh instance, turn on Loop Mode and choose End signal in Learn.
Leave Arm off, then start Clocked.

### End on repeat with two control cables

With the current worktree VST3, generate a fresh test patch:

```powershell
python -B docs/validation/issue-133-rack-fixture/make-patches.py clocked-end-on-repeat --bundle $bundle --output 'out/issue-127/fixture/Loop End on Repeat - Clock and Phrase.vcv'
```

Clocked Master goes to Host-FX **B0 (23)** for Clock. **CLK 1** is set
to divide by 8 with zero delay and goes to **C0 (12)** for Phrase.
Choose **End on repeat** in Learn, click **Arm**, then start Clocked.
The first coincident Clock and Phrase starts capture and the next
closes it after two bars. For a sequencer, use a Step 1 pulse from the
same clock instead of the divided output.

### Sequencer fixture evidence

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
