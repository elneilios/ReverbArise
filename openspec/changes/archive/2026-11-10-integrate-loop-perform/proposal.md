# Proposal

## Why

[Issue #126](https://github.com/elneilios/ReverbArise/issues/126) brings accepted Loop audio into the production Learn and Perform workflow. #133 delivered capture and #125 delivered the headless scheduler. Musicians need to generate and edit swells from the accepted phrase, see whether live phase is ready, and use Cue and Stop in Rack.

## What Changes

- Preserve the owner-approved Learn controls and keep accepted source identity and length separate from the next capture preference. Use the accepted Loop's markers for explicit Generate Swells.
- Extend the shared Perform editor, Swells list, inspector, Overview and Audition with phrase-local editing and negative wet lead-in.
- Add one command beside Audition that offers Cue Loop, Cancel Loop or Stop Loop, with separate playback status. The incoming phrase reference defines wet playback origin once its recurrence verifies automatically.
- Verify live Phrase pulses as soon as a Loop is accepted in Learn and carry verified timing into Perform while the same reference and source remain valid.
- Open the native preview for owner approval with side-by-side Song and Loop evidence at 900×600 and 100% and 125% scale. After approval, verify and hand over a simple audible VCV patch for local Cue and Stop.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `workflow-modes` — enable Perform from a verified accepted Loop without requiring Song transport or live Loop phase.
- `swell-workspace` — use the existing editor for phrase-local swells and the Loop playback row.
- `loop-phrase-capture` — generate swells from the accepted Loop's eligible markers while preserving Learn capture policy.
- `loop-host-admission` — let the verified live phrase route declare wet origin independently of dry alignment.
- `transport-swell-playback` — grant Cue from verified recurrence automatically and place wet downbeats at the routed markers.

## Impact

Use the existing editor, source and render owners, and #125 scheduler. Use the [approved mockups](https://github.com/elneilios/ReverbArise/issues/116#issuecomment-6707861945) for hierarchy and the current production standards for the controls. #133 remains authoritative for Learn capture interaction. Preserve Song behavior and saved project identities. FL Studio routing belongs to #127, Loop WAV export to #128, and broader asset persistence and recovery to #129.
