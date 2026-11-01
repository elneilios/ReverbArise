# Proposal

## Why

M3's editing engine needs a usable automatic workflow and clearer monitoring before signoff. The delivered automatic baseline works in FL Studio, but host feedback exposes excessive transient markers, difficult envelope manipulation and missing wet-to-source transition context that this iteration must address.

## What Changes

- Enable progressive dry capture and background detection, stable automatic target creation and dependency-driven rendering, using a genuinely bundled default IR. Preserve manual edits and independent per-swell IR assignments.
- Replace the dense editor with a left swell list, central aligned source/wet timeline, right inspector, audition strip, lower song overview and collapsible Diagnostics, following the handoff's supported controls.
- Add aligned captured-dry plus shaped-wet audition with a persisted percentage blend. Default audition trim to 0 dB for new settings, retain wet peak protection, and preserve saved trim choices.
- Retain manual analysis, generation, recapture, sample addresses, tone and asset tools in Diagnostics.
- Put one prominent, typeable Marker sensitivity knob and an adjacent Auto markers switch in the main workspace. Lower sensitivity favours stronger attacks in busy material; the switch defaults on and pauses automatic marker creation/reconciliation independently of capture, manual editing and existing-render refresh. Save both per plugin instance, coalesce background reanalysis, and preserve manual edits and deletion suppression.
- Refine attack detection so a decaying event does not create multiple automatic swells while distinct real attacks remain detectable; protect existing manual edits and identities.
- Add responsive on-curve endpoint/fade handles and continuous tension control. Let the wet endpoint move independently before or beyond the fixed dry arrival, with an independently adjustable wet fade-out creating a gap or overlap.
- Prepare sufficient real wet render support for the requested overlap, showing any required additional captured source context. Transition edits never fade, duck, move or stretch dry audio; existing explicit audition blend/trim/protection remain separate.
- Extend Dry + Wet audition through the transition and a configurable listening tail defaulting to two seconds. Missing captured context is reported instead of silently shortened or replaced with silence.
- Keep fade-duration controls directly grabbable from zero/initial state, with labelled collision layouts linked to their true envelope anchors.
- Let a whole-swell body gesture translate wet audio and its complete envelope together using a saved placement offset, preserving duration, fades, tension, base extraction and fixed dry arrival without reconvolution.
- Prevent abrupt wet/audition cuts with short disclosed monitoring edge ramps. New manual and automatic swells start with 20 ms wet overlap and a 20 ms wet fade-out; saved edits retain their values.
- Default manual audition to all eligible swells intersecting the selected listening window, with a saved Isolate selected option. Sum aligned wet once and captured dry once, respect mute/solo, protect the combined contribution and never recursively extend the window through neighbouring swells.
- Defer transport-synchronized swell playback, including replay/seek/loop scheduling and silent-insert wet scheduling validation. Automatic generation produces inspectable and manually auditionable audio in this change; it does not play swells with Song transport.

## Capabilities

### New Capabilities

- `automatic-swell-processing`: Progressive capture-to-detection-to-render coordination, attack-aware acceptance, stable target provenance, default IR ownership, bounded work and restoration policy.
- `swell-workspace`: Single-screen editing, responsive on-curve/tension controls, wet endpoint gap/overlap editing, musical timing and accessible diagnostics.
- `audition-monitoring`: Captured-dry/wet alignment, listening context after the transition, blend law, prepared playback buffers and saved monitoring preferences.

### Modified Capabilities

- `reverse-convolution`: Automatic preparation, genuine wet support for overlap, selected-window overlapping shaped audition with optional isolation and contextual dry playback, and neutral default monitoring trim.

## Impact

Extends the implemented `edit-and-persist-target-swells` baseline in Core/SwellSession, DSP/SwellEngine and AuditionVoice, Capture workers, PluginProcessor, PluginEditor and UI timelines. Adds a redistributable generated default IR, state migration, headless/integration/UI tests and a separate FL Studio validation guide. Reuses existing convolution and WAV/WavPack import; no new reverb algorithm or external library is planned.

The first automatic baseline had 39 completed implementation tasks. The first feedback implementation now has 56/63 tasks complete and a passing 23-test suite; seven host checks remain open. Preserve those completed tasks as historical evidence. New unchecked tasks cover zero-fade grabbing, wet-audio/envelope placement, monitoring boundary ramps, gentle new-swell transitions, overlapping-swell audition and main-view marker sensitivity/enable controls; the previous test pass does not validate these new additions. The user's clarification that dry remains untouched supersedes the earlier complementary wet/dry crossfade suggestion. New transition parameters are prepared and manually auditioned now; Song-synchronized playback remains deferred.

M3 remains active with host acceptance incomplete. This change may be implemented before M3 signoff; acceptance must cover both, then sync/archive M3 before this successor so its reverse-convolution deltas win. No implementation is authorized by creating this proposal. Requirements are derived from `docs/Preverb_Lean_Functional_UI_Handoff_v1.2.md`, with the user's explicit exclusion of transport sync taking precedence. The wireframe is a layout reference, not authorization for presets, undo/redo, extra DSP or DAW transport controls.
