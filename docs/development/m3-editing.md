# M3 Editing

Implementation in progress; host acceptance is separate from automated tests.

Confirmed targets have stable UUIDs and independent source, shape and IR settings.
Unconfirmed onset candidates do not create swells. Batch selection has one primary
swell for inspection and manual audition. Delete removes a definition, never assets.
Muted/disabled definitions remain editable. When any solo is set, only enabled,
unmuted soloed definitions can audition.

Coordinates are integer samples at capture rate. Bar entry uses the fixed host
sample/PPQ anchor and nearest-sample rounding; sample edits never round through bars.
New source starts at arrival and ends at the earliest of a quarter note later, the
next known arrival, or contiguous coverage end. Later suggestions never resize it.
Arrival moves translate source and audible bounds together; source handles keep the
arrival inside the source interval. Gaps are never silently filled.

Shape defaults: raw start, exclusive arrival+1 end, zero fades, linear curves,
0 dB gain. **Full response** restores complete raw bounds; **Reset shape** restores defaults.
Explicit crops remain visibly invalid if a new IR makes them exceed raw bounds.
Fade lengths clamp individually to audible length minus one; overlapping fades
multiply, zero fade means no fade. Gain is -60 to +12 dB. Shape edits do not change
raw audio or its identity.

Metadata format v1 (inside plugin state v3) uses exact decimal strings for int64
positions, typed finite parameters and checked bounds. Limits are 4096 swells and
8 MiB serialized metadata; malformed state is rejected before replacing a session.
Names are limited to 256 characters. Selection must reference existing unique IDs.
Active audition, capture arms and pending jobs are never serialized.

Onset analysis streams contiguous committed sections with a Hann FFT window: the
smallest power of two at least 21 ms (minimum 256, maximum 8192 samples), hop one
quarter window. Positive spectral flux is combined across channels, thresholded
against recent novelty, then peak-suppressed. Sensitivity is 0..1 (default 0.5),
minimum separation 1..2000 ms (default 80 ms). Gaps/unavailable chunks reset history.
These are editable hop-resolution suggestions, not polyphonic note transcription.
Fixtures cover plucks at 44.1/48/96 kHz, soft attacks, dense noisy input, silence,
negative coordinates, cancellation and chunk splitting. Onset tolerances allow one
analysis window; manual targets remain sample-accurate.

For audible sample k in [A,B), fade-in uses (k-A)/fadeIn, fade-out uses
(B-1-k)/fadeOut; each coordinate clamps to 0..1. Curves are u, 3u²-2u³, and
(exp(4u)-1)/(exp(4)-1). Zero fade means unity. Multiply both fades and per-swell
gain; no time shift or stretching occurs. Independent numerical tests check the
complete result including singleton crops and overlapping fades.

Selected audition uses the complete edited interval, adds wet audio after dry capture,
and stays independent of FL Studio seeks/loops. Its shaped peak is attenuated to
-6 dBFS if necessary, never boosted, then the saved -60..0 dB audition control is
applied (new-session default -12 dB). Live level changes ramp over 5 ms. Shape or
selection/activity changes stop audition and require explicit restart. Raw and shaped
peaks, display fit and predicted wet audition level have distinct meanings; the
wet ceiling does not limit live dry input or diagnostic tone. Restore never auditions.

The source lane supports double-click to add a target; double-click an onset
suggestion to confirm it. Alternatively use **Suggest onsets**, the visible-suggestion
selector and **Confirm**, or **Add at playhead** after positioning FL Studio.
Confirming/reanalysis never resets existing confirmed edits. The target list supports
multiple selection; one primary target supplies the inspector and audition.

The overview, detail scrollbar, Prev/Next, 1/2/4/8/16-bar span, Zoom +/- and Fit
controls share absolute sample coordinates. Ctrl+wheel zooms; wheel scrolls. Snap
uses a sixteenth-note grid; turn it off for free dragging or use exact sample fields.
Source handles are at the bottom of the source lane; arrival markers are at its top.
Audible bounds are square handles at the wet lane's top; fade handles are orange at
its bottom. The inspector exposes the same operations numerically, with fade lengths
in milliseconds. **Set source from bars** explicitly replaces the selected source
range; merely navigating or focusing a field does not round precise edits.

Full raw response stays visible behind the shaped overlay; excluded samples are
dimmed. Over-full-scale peaks use a labeled display fit without flattening. Previous
results are labeled stale; pending shapes do not masquerade as current. The selected
source has finer peaks than the compact capture overview; waveform summaries use a
bounded cache, and close arrivals are visually grouped without changing their data.
The list virtualizes rows, and the suggestion selector limits visible menu items;
scroll/zoom or click the source lane to select other suggestions.

New-state metadata survives reopen. Editor zoom/page/snap choices are view controls,
not saved swell definitions. M3 still has no transport-synchronized swell playback.
Follow [FL Studio M3 validation](fl-studio-m3-validation.md) for host acceptance.
