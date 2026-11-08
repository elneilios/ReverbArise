# Loop capture in VCV Rack

Load ReverbArise in Host-FX and connect the stereo audio you want to capture. In Learn, enable **Loop** for the next take. Use one patch clock for the source, the beat and bar pulses, and the phrase downbeat. Start must rise on the same Host-FX audio sample as the phrase, bar and beat events. Route parallel signals through equivalent Rack module paths to keep their edges aligned.

| Host-FX Note Gate | MIDI note | Patch signal |
| --- | ---: | --- |
| D0 | 14 | Capture Start or capture gate |
| C0 | 12 | Phrase downbeat |
| G0 | 19 | Bar downbeat |
| A0 | 21 | Beat pulse |
| A♯0 | 22 | Clock reset, when available |

- **Fixed length** records from Start through the selected bars and additional beats. Let the clock continue until Learn finishes. Start gate release has no effect on length.
- **Follow capture gate** records from the rising gate. A falling gate requests Stop. The take ends before the first sample of the next bar. A fall on a bar ends at that bar.
- Click **Arm capture** after the clock is running. If Learn is waiting for gate low, let it fall once before sending Start. Learn counts the beat pulses between bar pulses before it accepts Start. Capture is limited to 15 seconds. A missing beat, missing bar or clock reset during capture cancels the take when observed. The last accepted take remains available.

**Show captured WAV** opens the verified file's location. Use a separate copy when moving the patch to another machine. Saved prototype sessions with the earlier gate-only setting show **Legacy gate** until you choose a clocked capture method. Their existing takes still open. Turn off Enable Loop before your next Song capture. The last accepted take continues to select the playback path until another take succeeds. Loop capture via DAW transport and Loop Export are outside this local Rack slice.

## Cue one swell against a fixed Rack phrase

Use one patch clock and reset for the source phrase, its capture gate and the recurring phrase-start pulse. At the selected rate, measure the capture gate's high span and the period between phrase markers. The two must match exactly. Route the audio into Host-FX's stereo inputs and map the control inputs in the module as follows.

| Host-FX Note Gate | MIDI note | Patch signal |
| --- | --- | --- |
| D0 | 14 | High for one full phrase while capturing |
| C0 | 12 | Short pulse at the start of every phrase |
| E0 | 16 | Independent Cue pulse |
| F0 | 17 | Independent Stop pulse |

Open Perform with an accepted Loop. Swells shows the prepared swell, and the Dry and Wet lanes show its position relative to Loop start. Drag either end of the highlighted source interval in Dry, or drag the waveform in Wet to move the swell. Use the zoom controls, then click or drag Overview to navigate. Fit returns to the whole phrase. Timing sets the source interval and placement, Envelope sets lead-in and fade out, and Sound selects a bundled IR. The rotary controls accept typed milliseconds. Reset returns placement and envelope settings to their defaults. Reset All also restores the source range and reverb. Audition plays the wet swell once and leaves the patch audio passing through. The reader checks the checksum of the exact WAV bytes it decodes and requires the saved stereo float format, native rate and frame count. The first captured frame is 0.

After two exact intervals and wet preparation, Loop Playback shows **Ready to cue**. Cue selects the next phrase start that leaves enough time for the entire lead-in. Once repeating, negative-position wet is mixed into the end of the cycle. A prepared render must fit within one phrase before and one phrase after zero to allow Cue. Edits made while the loop is queued or playing apply to the next Cue. Stop cancels queued playback or fades active wet over 5 ms.

A missing or shifted marker ends the launch and requires a new Cue after the reference returns. Stop before unplugging or suspending Host-FX. Host-FX can report an unplug as an ordinary Note Gate release and can suspend callbacks without notification. The current test setup uses an exact fixed period and no sample stretching. Reload opens Learn with the verified take and saved edit. Reopen Perform to reacquire the reference and Cue again.

To roll back the test, close the host and restore the previously installed VST3 bundle. Reopen the backup of the Rack patch or DAW project made before testing. Retain the new copy of the patch and any captured WAVs separately.
