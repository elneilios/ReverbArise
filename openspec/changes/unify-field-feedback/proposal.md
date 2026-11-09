# Proposal

## Why

The IR selector already puts feedback next to the control, but the Settings source-range pair has side labels and no local explanation when its existing command rejects a range. This first C3 slice under [#107](https://github.com/elneilios/ReverbArise/issues/107) puts guidance and validation where the user can act on it and establishes how it wraps in a narrow field or group.

## What Changes

- Define a reusable field layout with a caption above each input and small, left-aligned help or validation directly below the input or its field group. The message wraps to as many lines as needed within that width at the approved type size; the layout makes room for it.
- Adopt the layout in **Settings → Swell → Source range (bars)**. Keep Bar start and End excl. as a pair with one message beneath the two inputs. Show existing command rejections there with red text and red borders on the implicated fields; keep aqua keyboard/accessibility focus separate. Keep explanations readable when an action is unavailable.
- Use the approved IR selector as the reference for feedback and as a regression check. Preserve its reviewed layout and import-button alignment. Review the C3 layout and interaction in a native preview before adopting it in Settings.
- Keep Return and Set source from bars as the pair's existing commit paths. Preserve parsing, eligibility, timing and accepted-state behavior. Other C3 surfaces, C4 editor mechanics, and the #108 and #8 work stay outside this slice.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `ui-design-system`: Define bounded, multiline feedback and above-field captions for the first shared C3 composition, including the source-range pair's local command feedback.

## Impact

The change affects the shared field presentation and the Settings Swell layout and binding. It may need a scoped result from the existing source-range command to display the reason it already supplies. It adds native review and focused coverage for layout, accessibility and the existing commit path. Workspace notices, transport, IR behavior, host parameters and saved sessions retain their existing contracts.
