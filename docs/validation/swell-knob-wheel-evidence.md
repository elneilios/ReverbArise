# Selected-swell wheel regression — issue #15

2026-11-02. Branch: `fix/15-swell-knob-interaction`, based on `origin/main`.

The shared `KnobValue` component explicitly disabled wheel input. Global
sensitivity sliders did not. Both use JUCE's RotaryHorizontalVerticalDrag style
and default drag travel; inspector controls use normalized 0–1 values before
conversion into their validated parameter domains. The global percentages use
0–100 with 0.1 steps. Inspector Shift-drag deliberately uses slower velocity
sensitivity (0.2); globals retain JUCE's default modifier/velocity settings.
No custom acceleration or panel-wide wheel override was added.

Enabling JUCE wheel handling restores the existing begin/delta/end edit path for
all inspector knobs. The regression sends native MouseEvent/MouseWheelDetails
input. It failed against the disabled-wheel component and passed with the fix.
The focused Release `preverb_swell_ui_test` passed (2.21 s; CTest 2.25 s), covering:

- Wheel direction and normalized travel against a global-style rotary slider,
  numeric updates during input, gesture completion, upper bounds and disabled input.
- Actual selected-swell Level model/field synchronization; neighboring swell
  shape and captured source revision remain unchanged.
- Inspector scrolling outside a knob, existing fine drag, exact typing, stale
  selection/deletion guards, canvas synchronization and persistence fixtures.

`git diff --check` passed. No DSP or persistence format changed. No full-suite
claim or FL Studio interaction-feel pass is made for this branch. Host wheel,
focus/scroll behavior and subjective comparison with global knobs remain for
owner validation after integration with PR #18; the issue is not yet closed.
