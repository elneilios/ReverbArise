# Design

## Context

See [the proposal](proposal.md) for the issue scope.

`PluginProcessor` reads Loop observations in the audio callback. The existing note decoder and Phrase clock describe measured Rack behavior. `LoopLiveReference` admits recurring phase and `LoopPlayback` schedules prepared wet. Learn capture and the live reference already have separate authority. Perform exposes the existing Cue, Cancel and Stop command and reports why Cue is unavailable.

The FL Studio test-install workflow provides a Release VST3 in the system scan location. Installation proves discovery only. The host qualification must observe actual callback data and routing from an FL Studio project.

## Goals / Non-Goals

**Goals**

- Establish a repeatable FL Studio capture route and let the musician start live wet playback with a button or MIDI note. Record each qualified command route and its limits.
- Preserve the accepted Loop on an interrupted candidate and require fresh Cue after a live discontinuity.
- Deliver a minimal Playlist fixture and observed evidence, plus automated checks for any newly supported route.

**Non-Goals**

- New screens, speculative controls, a change to Song Mode, or #128 Loop export.
- Re-creating missed event-driven commands after a seek or stretching the accepted audio to a new tempo.

## Decisions

1. **Measure before selecting a route.** Use the supported test installation and a minimal Playlist with a bright transient. Record the FL Studio and wrapper versions, sample rate, callback conditions, audio and transport observations, wrapper ports and channels, actual MIDI note numbers, offsets, note-on and release behavior, and duplicates. Probe D5 and C5 as candidates. Select native timing, measured MIDI, or a documented fallback separately for each purpose.
2. **Align Cue to a bar.** Reuse the wet scheduler. FL Studio uses Playlist position and the host bar origin by default. A Phrase note received in standby sets an offset for the four-beat playback grid at the note's sample. Clear that offset on a transport interruption. Rack measures beat intervals from Clock and counts four beats per bar starting at the first pulse; a Phrase pulse aligned to a beat can set the downbeat while idle. Cue uses the first future downbeat with room for the full lead-in. Both hosts repeat at the accepted sample length. A legacy Phrase-only route retains its recurrence checks.
3. **Route commands through existing lifecycle handling.** Feed qualified external commands to the same capture and performance logic as their UI equivalents. Keep the two command roles separate. Verify seeks, resets, and late cues with the existing discontinuity and fresh-Cue rules. A verified repeat preserves the queue and playback cursor. Probe offline and arbitrary-start behavior separately, and document a position-based route only when reproducible.
4. **Reuse existing feedback and approval gates.** Report unavailable routes with actionable reasons through the current Learn or Perform status. Reuse existing controls. If a visual change proves necessary, show a native preview for approval before rollout.
5. **Capture starts at a Playlist bar line.** An armed take may capture from the first supported playing block if that block starts on a downbeat. The next callback must confirm continuity before the candidate can proceed. An Arm partway through a bar waits for the next downbeat. Duration uses its configured span. End Signal may be driven by MIDI Start/Arm and Stop, with Stop finishing at the next verified downbeat.
6. **Finish the current pass.** The first Stop Loop click excludes future cycles from the wet mix. The second click uses the existing immediate fade. MIDI Stop requests a finish once, and repeating it leaves the scheduled finish in place. MIDI Stop Now uses the short fade immediately. Both hosts share channel 1 note numbers: Phrase 12, Cue 16, Stop 17, Stop Now 18, Capture Start/Arm 24 and Capture Stop 25. The host-specific clock routes remain separate.
7. **Use the same Rack downbeat for Duration and Cue.** Observe Rack Clock while Learn or Perform has a Loop in use. Duration waits for an observed downbeat, including the first pulse of a new run. The selected Bars or Beats value sets the recording length. End Signal keeps its sequenced or manual start rule.
8. **Read Loop position in Perform.** Keep the approved transport readout and derive Bar and Beat from the same Loop playhead drawn on the waveform. Use the accepted source's tempo and meter. Before beat one, while waiting, or after stopping, position is unavailable. Song Mode continues to show the host observations.
9. **Keep the Loop running through swell edits.** A playing Loop keeps a complete compatible mix while the worker prepares an edit. During repeating playback, replace it at the current position using a short crossfade. Keep a queued first entry tied to its original lead-in. A silent mix from intentionally excluding every swell still advances the playhead. Keep the existing fades for invalid source, timing and playback authority. Use the shared status and hover help to explain disabled Audition and Cue, including the required capture rate.
10. **Recognize FL Studio's declared repeat in Learn and Perform.** Accept a wrap only when consecutive playing callbacks match the same declared range and musical mapping, and their samples meet the exact end and start of that range. Learn counts elapsed capture beats across the wrap and can settle at its first beat. Perform applies the same boundary check to its own live observations and keeps the independent scheduler position.
11. **Make automatic repeat capture opt-in.** Add End on repeat to the existing capture method selector. Start on a verified range start in FL Studio, or on a Phrase pulse aligned with Clock in Rack. Close on the next verified repeat boundary after whole bars of audio. If armed in the middle of a pass, wait for a complete pass. Reset, a premature seek, a changed repeat range or manual disarm cancels the take.
12. **Describe capture the same way across hosts.** Learn explains when each capture method starts and ends. The guide appendices explain how to supply timing and external commands in each host.

## Risks / Trade-offs

- Wrapper delivery can differ from Piano Roll notation and from Rack note gates. A controlled host trace determines the admitted mapping and the configuration documented in the guide.
- A callback gap can prevent a safe end boundary. Preserve the prior loop and record the verified Duration fallback.
- Offline rendering may omit or reorder event-driven commands. Document the observed behavior and keep Loop export scoped to #128.
