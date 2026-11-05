# Perform workspace

Current: the owner approved the complete revised native presentation on 4 November 2026.
Production now adopts the 52-pixel audition strip, grouped/full Source inspector,
compact readouts with exact editing, shared ruler, aligned headings and normal-font
bottom hover-help footer (standards revision 5). Issue #68 adds the owner-approved
stable Bar, Beat, BPM and meter readout (standards revision 6), using independent
font-measured slots, right-aligned numbers and anchored separators above hover
help. Values and availability updates do not move the group. Oversized values
retain full accessible/help text. See the [readout policy](../development/transport-readout.md).
The temporary preview-only switch
is removed; native tests exercise the ordinary production editor. Earlier review
chronology below is retained; the latest approval supersedes pending-adoption wording.

The reserved header message row now retains a warning-level Perform playback
caution when no higher-priority notice is active. It explains that swells can
remain audible during DAW playback even if the source is silent or muted.
Errors, actionable warnings and transient preparation messages temporarily
replace it. This reuses the existing NoticeBar typography, warning icon/ink and
geometry; it adds no control family, token, layout exception or audio policy.


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

Subsequent owner feedback requests a single-row 52-pixel strip with status to
the right and equal 8-pixel action gaps. That native metric proposal is review-only
until approval; production retains 72 pixels meanwhile. Overview now uses the
established Inter SemiBold section-heading role. See the
[slim-strip checkpoint](../validation/2026-11-04-issue-46-audition-strip.md).

The subsequent [Perform clarity checkpoint](../validation/2026-11-04-issue-46-perform-clarity.md)
preserves that 52-pixel preview strip and three columns, while making audition
mode explicit, composing the shared ruler, grouping the inspector, shortening
annotations into context help and aligning footer/headings. Its metric/compact-value
profile remains review-only pending actual visual approval; existing roles and
tokens are reused without a new visual exception.

The owner's subsequent hover-help revision replaces floating tooltips with one
shared bottom footer below the musical position. All existing control and graph
help, including nested fields and owned Settings/Diagnostics controls, routes
there at the normal Inter Regular 14 helper role. The existing 20-pixel feedback
height and bottom margin contain it without a graph overlay. Long messages
ellipsize without shrinking; full text remains in the accessible description.
Hovering changes no musical, navigation or saved state. See the
[bottom-help checkpoint](../validation/2026-11-04-issue-46-hover-footer.md).

See [composition approval evidence](../validation/2026-11-04-issue-46-perform-composition.md)
and [integration validation](../validation/2026-11-04-issue-46-integration.md).

Pre-roll and End Offset depend on the selected swell's available raw wet bounds.
Before those bounds exist their existing disabled value cells remain blank;
song origin is not a substitute wet endpoint. Once preparation publishes, the
same cells show their normal sample-derived values. Saved shape values and the
existing control family, geometry, interaction and visual tokens are unchanged.
Explicit manual insertion and input/IR edits request background preparation even
with automatic policies off; readiness and Audition still require verified audio.
See [manual preparation policy](../development/workflow-modes.md#explicit-manual-preparation-in-perform).

An empty dry-only session can create its first target by double-clicking the dry
lane with Snap on. Until that first accepted target pins session timing, the grid
uses current valid host timing at the captured sample rate. Established timing
and missing host-position guards still apply; creating a target with '+' first
is unnecessary. This is a binding correction in the existing timeline control.
