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

Shared controls contain presentation and intent callbacks only. Perform owns
editable source/wet gestures and grid policy; Learn owns captured dry inspection;
Export owns read-only mix preview, transport and files. No generalized waveform
viewer or Export behavior is added to the shared controls.

See [composition approval evidence](../validation/2026-11-04-issue-46-perform-composition.md)
and [integration validation](../validation/2026-11-04-issue-46-integration.md).
