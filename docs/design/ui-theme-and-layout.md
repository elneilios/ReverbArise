# Native UI theme and visual checkpoints

`Source/UI/WorkspaceTheme.h` centralizes the approved baseline palette and flat
native LookAndFeel. `WorkspaceLayout.h` calculates eight regions in six bands
from editor size only; changing selection/readiness or opening the separate
Settings/Diagnostics windows does not change their positions.

The numerical palette remains a proposed baseline adopted for this change:
background #0F1418, aqua #00EFFF/#00C2CC, timing #FFC700, text #F8FAFC/#E1E6EC,
rules #4A5058 and control outlines #6B737C. The first stage replaces the large
boxed header/list/editor/inspector surfaces with rules. Control/selection outlines
are allowed; only the Overview waveform viewport has a major rounded outline.
Its title and ruler sit above that viewport.

The header embeds only `resources/logo/svg/reverbarise-wordmark-dark.svg` and
`resources/logo/source/InterVariable.ttf`, rather than loading resources from
disk in the callback or embedding the preview contact sheet. The supplied SVG
keeps its aspect ratio, integrated A, alpha and clear space. Regular native
control text uses the supplied Inter typeface; bold and italic roles explicitly
use real system Segoe UI faces while variable-font style-axis support is not
verified on this backend. The Inter OFL notice is copied into the VST3 bundle's
Contents/Resources alongside the existing WavPack notice. Native visual inspection raised main headings/tagline to 21 logical pixels and the empty headline to 24 for clearer hierarchy; these are proposed checkpoint metrics, not immutable raster measurements. No font downloader or
logo-export build dependency is introduced.

The current logical opening default and minimum are both 1100x850, with maximum
2000x1400. The former 1400x1000 opening size remains a supported reference size.
The smaller default and brighter off-white text were explicitly requested by the user on 2026-11-02. Margins are 20 (16 below 1200 width); header/global/audition/overview/
information bands are 70/66/82/142/44 pixels. Workspace gets the remaining height.
The left list is 210 (180 at minimum width); inspector is 340 (320 at minimum).
Twenty-pixel column separation preserves the large central waveform editor.
The inspector and list scroll independently. Screenshot scaling alone does not
prove a physically scaled host window fits every monitor work area.

Checkpoint A was explicitly approved by the user. Checkpoint B now has eleven
paired knob/value cells in a two-column, six-row inspector: Target/Move Wet,
Source Start/End, Pre-roll/End Offset, Fade In/Out, Curve In/Out, and Level.
The inspector scrolls independently; Enabled/Solo/Freeze, the current IR and
split Reset action occupy its initial view. Every cell keeps the selected UUID
and exact initial sample value for the duration of a gesture. Timing knobs use
checked integer deltas over the current editor span (fade knobs use current wet
length); absolute samples never pass through a floating absolute slider value.
Typed input is never grid-snapped. Return/focus loss commits valid owned input,
Escape cancels, invalid input restores the canonical value and reports why.
A selection/deletion or superseding input/timing change cancels the old gesture.

Absolute Target/Source fields use Playlist bar.beat.fraction by default; Settings
can select exact integer samples. Move Wet uses signed milliseconds or exact
samples under that same option. Pre-roll and End Offset use signed seconds;
fade lengths use milliseconds, Level -60..+12 dB, and independent tensions
-1..+1. Pre-roll is measured from the dry target to the positioned wet start;
positive End Offset overlaps the original note, negative makes a gap. Both
invert wet placement with checked arithmetic when editing the logical shape.
Neither changes live dry audio. The source selection has matching vertical
lines and circular grips with subtle neutral shading; the yellow dashed target
and upper marker stay separate from its range. Missing/unmapped coverage remains
visible. Wet handles are outlined circles and keep genuine time positions,
centre-line start/end and existing stacking/gesture rules.

The IR dropdown contains the real bundled Preverb Hall and, when different,
the swell's current custom IR. Load file supports the existing WAV/WavPack
backend, pins the opening swell ID/generation and imports on the worker. This
is independent of the future-default IR setting. Frozen swells cannot assign
an IR/change source until unfrozen; shape/gain/placement remain editable within
prepared support.

Reset wet shape restores the current creation defaults: automatic raw start,
zero placement/gain/fade-in/tensions, Linear in curve, 20 ms positive endpoint
and 20 ms Smooth release (sample-rounded, with valid short-render clamping).
It preserves name, target/source, IR, activity and freeze. The adjacent dropdown
explicitly offers Reset all swell settings: preserve UUID/name/current target,
rebuild the source interval using current captured coverage/next target, assign
the current future-default IR and restore wet/activity defaults including
unfreezing. The worker validates actual source/default audio before committing;
failed/superseded transactions do not partially unfreeze or change the swell.
A failed Reset all remains visible in the header until the next edit/reset/IR
action. Full response remains a separate Settings action rather than a reset.

New manual and automatic names use the one-based Playlist bar-and-beat arrival,
e.g. Swell Bar 18.1 for 34 seconds at 120 BPM/4/4. A fractional beat is included
when needed, e.g. Swell Bar 18.1.25 at 34.125 seconds. Repeated identical labels
receive (2), (3), etc. so new names remain unique within the session. UUIDs stay
independent. Existing/custom/saved names survive reanalysis, placement and reset;
existing names are not migrated. Timing-unavailable naming falls back to Swell
rather than inventing a position. The same formatter supplies inspector positions.
Marker sensitivity changes event selectivity; Regeneration sensitivity tolerates
performance variation; each is independent and saved. Initial mapping still
needs a first pass; already prepared wet audio is heard on subsequent playback.

The gear opens a separate, editor-owned Settings window with Playback,
Processing, Swell, Audition and Recovery tabs. Settings uses labelled form rows;
no controls are painted or allocated over the main workspace. The two global
sensitivities and the selected source/shape/Freeze/Solo controls stay in place.
Playback holds live enable/routing/wet level; Processing holds capture/master
policy, minimum separation and future-default IR; Swell holds mute, independent
curve presets, exact-unit choice, bar-range convenience and Full response;
Audition holds isolation, listening tail and monitoring trim; Recovery holds
explicit generate/suggest/confirm/recapture/Clear/suppression/test-tone actions.
Live-off/nondefault live routing/level and critical warnings remain in the main
status with full tooltip text. A reset error cannot hide an export warning.

Recovery > Open diagnostics opens a second editor-owned window. Copyable absolute-bound observations remain there as read-only diagnostics.
All source/shape/placement edits use the identity-pinned inspector controls;
there is no second writable developer editing surface. Diagnostic text formats
only while that window is open, at most five times per second, and refreshes
immediately on reopen. Both windows hide on Close and are destroyed before the
editor's LookAndFeel. Opening/closing them performs no engine action.

The row three-dot/right-click menu is positioned at the event's screen point,
with the row component retained as its scale/lifetime target. The whole list's
rectangle is never used as its popup anchor, including after list scrolling.
Rename/Delete continue to address the captured stable swell UUID.

Final audition/overview/diagnostic consolidation and Release/physical-host
acceptance remain tracked separately. Native software screenshots alone do not
prove FL Studio popup scaling or physical Windows DPI behaviour.

See [evidence and approvals](../validation/ui-alignment-evidence.md). Checkpoint B, Settings for this iteration, the compact opening size and brighter
text have recorded user approvals. The final integrated checkpoint C remains
separate from automated evidence and physical FL Studio/DPI acceptance.


The Overview now draws cached real wet peaks weighted by each swell's current
envelope, gain and placement; it no longer draws generic quadratic swell icons.
Those summaries are scaled for visibility and do not change audio levels. A
waiting result without cached wet data is a position marker, not a fake waveform.
The map spans actual capture/render/requested extents, including negative wet
lead-in, rather than imposing an arbitrary eight-second tail on captured audio.
Its shared content/highlight rectangles drive both paint and navigation. Only
editor navigation changes; DAW position is never sought. Captured silence,
unmapped gaps and unavailable sections stay distinct. A retained source-detail
cache is clipped to currently available audio ranges, so stale data cannot paint
over a current gap or missing asset.

A negative wet endpoint creates a gap before the unchanged dry target; positive
end offset overlaps it. Negative-time wet audio remains visible, but output is
not possible before the DAW actually processes those positions. Pre-roll/end
entry includes placement and inverts it with checked integer arithmetic.
Shift-drag uses JUCE's fine velocity mode on inspector knobs; typed positions
retain full precision independent of Snap and displayed rounding. The main
preview status discloses active wet/sum safety attenuation and saved trim stays
visible/resettable. The test-tone armed state is visible in the header with its
Settings > Recovery Off path.

See [current diagnostic dispositions and final host checklist](ui-diagnostics-and-acceptance.md).
