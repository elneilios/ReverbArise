# Native UI theme and visual checkpoints

`Source/UI/WorkspaceTheme.h` centralizes the approved baseline palette and flat
native LookAndFeel. `WorkspaceLayout.h` calculates eight regions in six bands
from editor size only; changing selection/readiness or opening the interim
Settings overlay does not change their positions.

The numerical palette remains a proposed baseline adopted for this change:
background #0F1418, aqua #00EFFF/#00C2CC, timing #FFC700, text #F5F7F9/#C7CDD4,
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

The first-stage logical default is 1400x1000, minimum 1100x850 and maximum
2000x1400. Margins are 20 (16 below 1200 width); header/global/audition/overview/
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

New manual and automatic names use the one-based Playlist arrival bar, e.g.
Swell 18 for 34 seconds at 120 BPM/4/4. Same-bar names may repeat; UUIDs remain
independent. Existing/custom/saved names survive reanalysis, placement and reset.
Timing-unavailable naming falls back to Swell rather than a fictitious bar.
Marker sensitivity changes event selectivity; Regeneration sensitivity tolerates
performance variation; each is independent and saved. Initial mapping still
needs a first pass; already prepared wet audio is heard on subsequent playback.

This checkpoint is intermediate: existing recovery/settings controls remain
available from the gear in a temporary overlay which never moves the bands.
Legacy exact-bound and curve-preset tools remain there until the dedicated
Settings/diagnostics cleanup. That window and final audition/overview polish
remain unchecked tasks. Do not treat the temporary overlay as the final UI.

See [evidence and approvals](ui-alignment-evidence.md). Explicit checkpoint B
approval is required before proceeding to the final UI stage.
