# Perform workspace

Perform uses the owner-approved revision-4 compact composition and production
standards revision 3. `PerformLayout.h` owns its scoped geometry; Learn and the
shared header retain their approved comfortable measurements.

The chronological Swells list, dry/wet editor and Selected Swell inspector form
three columns. Full-width horizontal rules separate the header, audition,
Overview and information bands. Column content and vertical rules are inset from
these boundaries. The obsolete marker/sensitivity band is omitted; saved detection
preferences and Learn controls retain their existing behavior.

`Controls/SwellList` retains ID/epoch-bound sorting, multiple selection and scroll.
The inspector edits the primary swell only, using existing IR, Enabled, Solo,
Freeze and exact-value bindings. It scrolls vertically; long captions truncate
with full tooltips rather than shrinking production text. There are no bulk
inspector actions. Existing `KnobValue` owns pointer/keyboard/exact-entry lifecycle.

One row below the detail canvas contains shared Zoom − / slider / Zoom + / Fit,
a latching Snap button and no-wheel rhythmic selector. Height above the lanes
scales waveform display amplitude only; it never changes audio gain. The Overview
shows the same horizontal span and navigates without seeking the host.

Snap uses Ctrl for temporary bypass and Shift for fine drag; Ctrl+Shift combines
them. Ctrl displays Snap Off without changing its saved request. Exact numeric
entries remain unsnapped. Unsupported fixed host timing is disclosed, with Ctrl
or Snap Off available for valid sample edits. See [navigation and state contracts](../development/perform-navigation.md).

The inspector timing knobs follow the same absolute-anchor grid policy as canvas
handles. With Snap Off, wheel/arrows use fixed time increments independent of zoom;
typed fields retain full valid precision. Position labels use musical units and
duration/movement labels use s/ms. Samples remain internal/diagnostic. Overflowing
status and IR helper lines ellipsize at their approved font proportions, with full
tooltips. Knob focus and accessibility expose accepted domain values, including
when a candidate is snapped or rejected.

Shared controls contain presentation and intent callbacks only. Perform owns
editable source/wet gestures and grid policy; Learn owns captured dry inspection;
Export owns read-only mix preview, transport and files. No generalized waveform
viewer or Export behavior is added to the shared controls.

The audition row uses Play/Stop and the existing latching action family for
**Wet only**. New instances default Off: captured dry at unity plus saved Live
Wet gain, with existing disclosed wet/mix protection. On suppresses captured dry.
Play loops the finite prepared listening window until Stop or existing safety/
workflow/selection/DAW-priority guards stop it. It does not seek or start the host.
The percentage slider and row Trim reset are removed. Legacy project balance and
trim restore unchanged until explicit toggle; the retained legacy trim is shown
in Settings and inactive in live-level mode. Status uses Ready, Looping, Preparing
or Unavailable with accepted mode and membership; overflow ellipsizes at the
approved helper font size, and full reasons/protection details remain in tooltip.
The refined row's actual native checkpoint is directly open for review; final
host audition listening is pending. Existing layout, Snap and knob approvals
remain valid.

See [composition approval evidence](../validation/2026-11-04-issue-46-perform-composition.md)
and [integration validation](../validation/2026-11-04-issue-46-integration.md).
