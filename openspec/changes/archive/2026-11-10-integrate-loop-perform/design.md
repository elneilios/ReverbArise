# Design

## Context

The integrated #133 Learn route owns capture intent and atomic acceptance. #125 exposes headless Loop Standby, phase, preparation, Cue and Stop. The editor currently routes Perform readiness and placement through Song timing. The shared worker can prepare Loop wet in headless Live, while selected source wave, audition, and Learn generation still have Song-specific gates.

## Goals / Non-Goals

**Goals**

- Keep one production Learn and Perform workspace and one shared multi-swell renderer.
- Expose the scheduler's independent phase, wet preparation and playback states through the approved controls.
- Provide an owner-reviewed native preview before handing over an audible Rack patch.

**Non-Goals**

- FL Studio route qualification, Loop WAV export, project format changes and broader asset recovery belong to #127–#129.
- The spike's separate single-swell workspace remains reference material.

## Decisions

### Use accepted-source coordinates for editing

Use the existing accepted-source snapshot and managed revision at the editor and worker boundary. Derive a phrase-local edit grid from the Loop's verified timing and exact extent. Apply it to timeline, Overview, source range, exact fields, marker generation and finite Audition. Wet placement remains signed. Leave the Song host mapping and live scheduling callers on their existing timing path. A phase outage can block Cue while the same accepted phrase remains editable.

Route mode entry through existing workflow guards. Reuse the shared Generate transaction for explicit Loop generation and bound its automatic source windows at the exclusive phrase end. Pin the candidate, render and gesture to the accepted source and session. Keep the current marker overlay available while recapture is pending. Reuse #133 capture controls and preserve Song's Generate on Stop behavior. Use the existing render, source-window, IR and shape owners for the editor and audition.

### Derive live origin from the input route

Feed the #125 scheduler from the supported Host-FX Note Gate profile with declared 4/4. In Rack, expect a Phrase interval of one, two or four bars based on the accepted length. Verify the expected cadence against actual offsets under the #123 timing profile. Compare the route to the accepted rate, tempo, meter and rounded extent. Retain two matching intervals, epoch bounds, invalidation and fresh-Cue rules.

Observe live Phrase pulses for an accepted Loop in Learn as well as Perform. The scheduler retains both partial and completed verification when the same source and callback stream continue across the mode switch. Learn accepts no playback commands and emits only dry after any existing stop fade. A replacement source, interrupted stream, reset or invalid cadence revokes the observations. Perform still needs a coherent wet render and a fresh Cue.

The verified marker declares where the wet phrase lands. A producer may route dry audio with a different origin. The plug-in neither compares dry samples nor asks for a confirmation. Capture Clock and capture commands retain their existing roles. Host-facing copy describes the required reference and reason for an unavailable Cue using host-neutral terms. The Rack guide supplies the concrete Note Gate mapping.

### Bind a native preview before handing over the patch

Use one shared ActionButton with the existing neutral role for Cue Loop and the destructive role for Cancel Loop and Stop Loop. Keep the command in the same position and show the state beside it: Ready, Waiting for phrase, Playing lead-in, Playing loop or Fading out. Disable Stop during the fade. Provide action-specific hover and accessibility help. Preserve the existing focus behavior, separator geometry and Song row. Show actionable status when phase or wet preparation blocks Cue.

Extend the existing native Perform review harness with real accepted Loop data, at least two swells and the #125 scheduler bindings. Open the review window for the owner, with side-by-side evidence at 900×600 and 100% and 125%. The owner approves the actual row and interaction before the Rack handoff. A new visual variant or standards exception requires specific approval and associated standards and tests.

Use an audible Rack patch with a snare hit each bar. CLKD sends Clock for Learn and a four-bar Phrase pulse for Perform. Keep both connected from capture through playback. Use the same sample offsets for the sequencer's Clock and command triggers. Step 1 sends Arm and Phrase, and the last step sends capture Stop. Verify 4-, 8- and 16-step captures and live playback at the corresponding phrase lengths. Validate Cue, Cancel, repeating wet, Stop with dry continuing, and loss of phrase recurrence while Clock continues.

## Risks / Trade-offs

- The capture grid and live phase can differ. Use separate admission and include a shifted dry test, a capture-only case, and a mismatched phrase case.
- Replacing the accepted source during asynchronous work can expose stale PCM or gestures. Use the existing identity and generation fences and test replacement, restore and interrupted capture.
- Rack callback and cable paths can differ. Verify the named test configuration in the host and record which path, scale and device setup were exercised. Keep the patch local and reproducible with a small tracked generator or fixture description.

## Migration Plan

Keep current VST3 identifiers and managed asset references. Older projects retain Song. Reopened Loop projects recover the accepted source under existing guards, wait for fresh live observations, and enter Standby without queued wet. Keep verified evidence and the issue branch reviewable before merging into `feature/loop-mode`.
