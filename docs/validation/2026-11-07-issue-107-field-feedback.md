# Issue #107 field feedback validation

The owner approved the native field feedback preview on 7 November 2026, before
the Settings Swell rollout. The preview used the approved IR reference and a
source-range pair to review captions, multiline help and validation, correction,
and the readable unavailable state.

Settings Swell adopts that layout for the source-range pair. Return and the
existing button both submit through the same processor command. Settings
displays its own submission result, and selection or session replacement clears
the prior result. The production test exercises this through the real editor and
checks that rejection leaves the accepted source range intact.

Focused native Release checks passed on the Windows MSVC worktree based on
`c0b8870`:

- `preverb_swell_ui_test --test-source-range-feedback` verified the Settings
  binding, captions, multiline reflow and accessible descriptions at 900 × 600
  and 1100 × 850, each at 100% and 125% UI scale.
- `preverb_swell_ui_test --test-ir-editor` verified the existing IR field and
  folder alignment.
- `preverb_selector_popup_test --test-field-feedback` verified line bounds,
  long unbroken text and the independent red validation and aqua focus paint.

The warm Windows Release wrapper (`scripts/build.ps1 -Configuration Release -Test`)
passed all 98 CTest tests. UI standards, strict OpenSpec validation, user guide
currency and staged whitespace checks also passed.

The review and native tests cover Windows application presentation and user
scaling. FL Studio and physical Windows DPI variations were outside this check.
