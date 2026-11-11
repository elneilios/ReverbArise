# Issue #128 — native Loop Export preview

Branch: `codex/128-loop-wav-export`, based on integration revision `b34c64d`.
Checkpoint: layout and interaction review; owner approval pending. This is not a
completed Loop export implementation or host/audio acceptance result.

## Scope

The existing Export workspace has an optional Loop presentation composed from
shared ChoiceSelector, NumericInputControl, SliderControl, ActionButton,
FileDragTile and timeline controls. It shows Lead-in grid, Repeats, Wet level,
WAV start, Loop Bar 1, Repeat 2 and adjacent placement help. The preview follows
the [epic Export mockup](https://github.com/elneilios/ReverbArise/issues/116#issuecomment-6707861945)
and uses current production tokens. No new visual token or control-family
variant was introduced.

The native fixture supplies synthetic presentation data and does not publish
audio or files. File and audio actions are explicitly disabled. Production
editor entry and engine behavior have not been wired to this presentation.

## Verification

- Windows x64, MSVC 2026 Release, existing warm build tree.
- Initial wrapper attempts exposed a drawText overload ambiguity and two fixture
  compilation issues (peak-count narrowing and protected keyboard entry). These
  were corrected. The final focused `preverb_swell_ui_test` target build passed
  using the repository build lock after the wrapper released it.
- `preverb_swell_ui_test --test-loop-export`: passed. Covers notification-free
  refresh, repeat input rejection, wet editing, unavailable beat/meter choices,
  native keyboard focus/activation, stale-session input rejection and compact /
  expanded child containment. Registered as `preverb_loop_export_ui_test`.
- `scripts/check-ui-standards.ps1`: passed; existing debt inventory unchanged.
- Inspected native component screenshots at 900x600 and 1400x900, then opened
  and inspected the actual Windows review window through Computer Use.
- Logs: `out/issue-128/preview-build.log`, `preview-test.log` and `review-error.log`.
  Screenshots: `out/issue-128/preview/loop-export-900x600.png` and
  `loop-export-1400x900.png` (supporting regenerable output).

## Remaining gates

Explicit owner approval is pending. The preview window remains open for review.
Backend composition, delivery, metadata, guide maintenance, audio sample oracles,
full Release suite and host import/alignment checks remain unchecked in the
OpenSpec tasks. No system test VST3 was installed for this checkpoint, no PR was
opened and no physical DPI qualification is claimed; that matrix remains #130.

## Revised preview — shared Mix Character and Dry/Wet

The owner requested the existing Mix Character and Dry/Wet faders and explicitly
confirmed that the same pair must control Preview and the delivered WAV. The
OpenSpec proposal, Loop delta, design and tasks now record that decision; task
3.1 was reopened for this revision. The original wet-only preview is superseded.

The revised presentation reuses `ExportLevelsControl` directly and removes the
standalone Loop Wet slider. Grid and Repeats are above the waveform; the familiar
mix controls remain in the right column. Save and Return share one action row
in Loop presentation to preserve the full existing file tile at 900x600. Song
presentation keeps its existing layout. Wet Layer (0/100) is the fresh default;
dry-inclusive placement guidance warns against doubling the original dry.

Verification on the warm Windows MSVC Release tree:

- Focused native preview target build: passed under the repository build lock.
- `preverb_swell_ui_test --test-loop-export`: passed. Adds shared preset selection,
  Custom pair editing, continuous pair changes, stale-session/pass mix rejection,
  dry-inclusive guidance, dry-only origin, silent-pair presentation/recovery and
  full tile/layout containment. Refresh remains notification-free.
- `preverb_swell_ui_test --test-export-levels`: existing shared family regression
  checks passed.
- `scripts/check-ui-standards.ps1`: passed, unchanged debt inventory.
- Inspected Wet Layer and Custom mix screenshots at compact/expanded sizes and
  the actual revised native review window. No control-family visual exception or
  new design token was needed.

Logs are `out/issue-128/preview-v2-build.log`, `preview-v2-test.log` and
`export-levels-regression.log`. Supporting images are under
`out/issue-128/preview-v2/`, including `loop-export-900x600.png` and
`loop-export-mix-1400x900.png`.

The owner approved the revised native Export preview on 2026-11-10, including
the shared Mix Character and Dry/Wet controls. No visual-family exception was
requested. This remains a presentation fixture with audio/file actions disabled;
no sample-level DSP result, full
Release-suite result, host import result or production rollout is claimed.
