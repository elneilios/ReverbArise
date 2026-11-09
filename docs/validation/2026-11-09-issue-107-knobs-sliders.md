# Issue #107 P5 knob and slider validation

The owner approved the standalone native P5 preview for Level, Move Wet,
Event sensitivity, horizontal zoom, Height and the unavailable state. The
owner then approved the 0 dB Level detent and asked for a little more
attraction. The implementation widens the previewed band from ±0.2 to
±0.3 dB. The tick marks 0 dB at 12 o'clock. The band applies during drag.

The production baseline is merged P4 at `733f3cb`. The production painter
uses the approved quiet hover and held surface, aqua value indicator and
off-white marker, with keyboard/accessibility brackets outside the body.
The shared track keeps its own focus and wheel area separate from the P4
value field. D6 hidden controls remain compatibility seams.

The warm Windows MSVC Release build and focused native Level, Perform knob,
editor and responsive-scale checks passed. The Level test covers a 0.25 dB
drag candidate from either side, passing through and reversing direction,
nearby exact entry and accessibility, Ctrl+wheel and an ordinary click.
The real editor exercises Learn and Export levels, Settings value boxes and
Height. Software captures are retained under `out/ui-review/p5-integrated`
for comparison with the owner-approved fixture. The compact Sound and Timing
captures retain access to both Reset actions and show the shared zoom track.

The native review covers the Windows desktop fixture and software UI scaling.
Physical DPI and FL Studio host integration were outside this validation.

The focused checks on the final production build also passed for a typed
value so close to home that the dial position was already rounded to neutral,
real zoom track steps of 0.05 and 0.005, and the Reset menu. An earlier suite
exposed a pointer dismissal that briefly acquired keyboard brackets during
native focus restoration. The final check verified that pointer and keyboard
Reset each return focus with their own decoration and preserve captured
action behavior.

The owner tried the integrated Level and Reset review manually and confirmed
the interaction. During that review the owner requested arc-shaped mouse
feedback on the dials. The revision places quiet hover and stronger held ink
under the existing dial sweep, leaving the centre and bottom gap clear.
The Windows Release build and `--test-dial-pointer-paint` passed. The check
compares the arc band, the clear centre and gap, the disabled state and the
independent keyboard brackets. The owner approved the arc feedback.

The next review uses the shared 74-unit width for all eleven Perform dial
fields. Feedback aligns under the field and wraps within the value column.
Windows Release build `20261109-231012-5c7cb82f` passed.
`--test-knob-value-width` passed for standard, wide and space-constrained
columns in both dial layouts. `--test-dial-pointer-paint`, the UI standards
guard, guide currency and strict OpenSpec validation passed. The Focused Level
review preserved plain exact-entry fields per the owner. The owner then
requested removal of Target's permanent format hint. Its existing footer help
and accessible description carry the format, and validation still appears
beneath the field. Windows Release build `20261109-231435-41793e54` and the
offscreen `--test-responsive` check passed. That check exercised the footer
from the Target dial and field and verified that inline feedback appears for
invalid input and clears afterward. The owner approved the updated native
Timing review on 9 November 2026. Responsive layout and software scaling
checks passed on the approved revision. Layout coverage starts at 900 × 600
and extends through 1600 × 1000. The software scale check verifies 100% and
125% rendering and input mapping.

The final Windows Release wrapper run, `20261109-231836-b7f4c2bd`, built
successfully and passed 96 of 98 CTest entries on the first run. Reset and
readability timed out before exercising their assertions because Windows did
not give the fixtures foreground focus. Both passed when run separately on
the same binaries with their native windows active. The Reset retry verified
keyboard brackets on return, their absence after a pointer selection, and
captured Reset behavior. The readability retry covered all eleven fixed-width
fields and exact entry. UI standards, guide currency, strict OpenSpec and
whitespace checks passed. Physical DPI and FL Studio coverage remain open.
