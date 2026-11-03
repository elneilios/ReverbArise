# Static Inter and Segoe UI comparison — PR #24

Date: 2026-11-03. Review baseline: `2a99386`. The experiment below left the then
approved Segoe UI implementation and VST3 unchanged. PR #24 is not merged.

**Subsequent owner decision:** the owner approved static Inter as more readable
and instructed adoption, explicitly accepting the Zoom-button wrapping until the
Perform redesign. The comparison below preserves the evidence available before
that decision. Production adoption and new build results are recorded at the end
of [the issue #16 evidence](shared-mode-header-evidence.md).

## Finding and recommendation

Static Inter is a viable alternative, with clearer and more substantial small
text in these Windows captures. The earlier mixed Inter/Segoe implementation was
not a valid test of an all-Inter UI. My recommendation is to use explicit bundled
static Inter faces as the candidate for portable typography, subject to owner
approval of the comparison and a focused narrow-button text-fitting check.

It is not a pixel-identical replacement: at unchanged logical heights, Inter has
a larger apparent text footprint. In particular, the existing Zoom buttons wrap
their labels onto two lines. Keep the approved Segoe implementation until that
fit difference is resolved and the candidate is accepted. No layout, spacing,
font-size, DSP, identity or saved-state changes are proposed by this report.

## Why the earlier result was inconclusive

Inspection of `WorkspaceTheme.h` at `2891e5d` confirms that the original loader
used the embedded variable face only for plain text. Both `font()` and
`getTypefaceForFont()` explicitly routed bold/italic requests to Segoe UI. It
selected no variable axes and supplied no embedded italic face.

The current bundled variable TTF has `wght` 100–900, default 400, and `opsz`
14–32, default 14. It has no italic axis. Pinned JUCE 9.0.4 includes
`FontOptions::withVariableSettings()` and
`Typeface::cloneWithVariableSettings()`. The prior result therefore establishes
an integration limitation, not that Inter cannot render suitable weights.
This review compares the requested static faces; it does not claim a rendered
variable-axis implementation has also been validated.

References: [official Inter release](https://github.com/rsms/inter/releases/tag/v4.1),
[JUCE FontOptions API](https://docs.juce.com/master/classjuce_1_1FontOptions.html).
API availability was also checked against the actual pinned local JUCE source.

## Controlled experiment

An isolated detached checkout at
`out/typography-comparison/worktree/` contains review-only font resources and
font-selection/test hooks. No primary-checkout runtime file changed. Static
faces came from the official [Inter 4.1 archive](https://github.com/rsms/inter/releases/download/v4.1/Inter-4.1.zip),
with its SIL OFL license retained. No fonts were installed into Windows.

| Role | Static Inter file | Verified weight/style |
| --- | --- | --- |
| Body and numeric/control text | `Inter-Regular.ttf` | 400 / Regular |
| Mode navigation | `Inter-SemiBold.ttf` | 600 / SemiBold |
| Headings | `Inter-Bold.ttf` | 700 / Bold |
| Tagline and description | `Inter-Italic.ttf` | 400 / actual Italic face |

The experiment embeds the four files through JUCE BinaryData, loads each once
with `Typeface::createSystemTypefaceFor(bytes, size)`, and creates fonts using
`FontOptions(face).withHeight(existingHeight)`. It selects the exact face before
rendering, rather than asking family-name lookup to infer weight/style.
Inter fallback is disabled in the comparison to expose unintended substitutions.
Custom Graphics painting uses that same resolver as component fonts, including
navigation, waveform/list text and notices. Segoe uses the unchanged approved
resolver and existing system Regular/Semibold/Bold/Italic faces.

Logical sizes remain 21 for headings/tagline, 18 for navigation, 16 for the mode
description, and the existing 14/13/12/11 body/control/metadata roles. The minimum
editor is **1100 × 850**; all control/region positions, colors, weights by role,
spacing, SVG branding and waveform data are unchanged between candidates.

Five scenes per family were captured at 100%, 125%, 150% and 200%: Perform,
Learn with a notice, Export with a notice, Settings, and a weight/style specimen.
All 40 PNG dimensions were checked. These are JUCE software-rendered scale
captures, not screenshots of physical monitors at four Windows DPI settings.
No Direct2D-vs-software equivalence, new FL Studio acceptance or macOS result is
claimed. Browser fit-to-panel previews must not be mistaken for actual pixels.

## Rendering and fit results

| Check | Result |
| --- | --- |
| Exact face/style selection | All four Inter and four Segoe styles resolved as requested; Inter static faces report zero variable axes |
| Component-font audit | 234 text-bearing component records, 117 per family; no family substitutions in sampled Labels, TextEditors, ComboBoxes and TextButtons |
| Custom Graphics paths | Same explicit resolver for waveform/list/notices; Semibold navigation independently verified |
| Glyph coverage | Printable ASCII covered by each tested face; broad localization/symbol fallback was not tested |
| Layout regions | Same header, role, selector, notice, list, waveform, inspector and Overview bounds for both families |
| Header fit | Both fit the minimum-size header; Inter Learn description measures 287.72 logical pixels in its 330-pixel box, Segoe 236.48 |
| Narrow controls | Inter Zoom +/− labels are wider and use JUCE's inherited two-line fitting; Segoe retains its one-line appearance |
| Settings | Both render real control captions, fields, dropdowns and explanatory text; Inter has stronger apparent body-text presence |

The sample `Swell Bar 12.3 | 0.0 dB | 48,000 Hz` illustrates the metric difference:

| Face at the existing height | Segoe width | Inter width | Difference |
| --- | ---: | ---: | ---: |
| Regular, 14 | 154.85 | 189.42 | +22.3% |
| Semibold, 18 | 204.89 | 247.87 | +21.0% |
| Bold, 21 | 251.98 | 291.64 | +15.7% |
| Italic, 14 | 154.50 | 189.89 | +22.9% |

At 100%, Inter's small labels and values are more prominent; Segoe is more
compact. At 125% and 150%, both remain readable in the inspected captures, with
the same Inter width/wrapping difference. At 200%, true Regular/Semibold/Bold/
Italic distinctions are clear in both. No sampled missing glyphs, new container
overlaps or mode-description clipping were observed. The Zoom wrapping is a real
fit difference, not an antialiasing failure. The inherited inspector scroll cutoff
is present in both and was not redesigned.

### Matched minimum-size screenshots

| Scale | Static Inter | Approved Segoe UI | Specimens |
| --- | --- | --- | --- |
| 100% | [Inter](images/typography-comparison/inter-perform-100.png) | [Segoe](images/typography-comparison/segoe-perform-100.png) | [Inter](images/typography-comparison/inter-specimen-100.png) / [Segoe](images/typography-comparison/segoe-specimen-100.png) |
| 125% | [Inter](images/typography-comparison/inter-perform-125.png) | [Segoe](images/typography-comparison/segoe-perform-125.png) | [Inter](images/typography-comparison/inter-specimen-125.png) / [Segoe](images/typography-comparison/segoe-specimen-125.png) |
| 150% | [Inter](images/typography-comparison/inter-perform-150.png) | [Segoe](images/typography-comparison/segoe-perform-150.png) | [Inter](images/typography-comparison/inter-specimen-150.png) / [Segoe](images/typography-comparison/segoe-specimen-150.png) |
| 200% | [Inter](images/typography-comparison/inter-perform-200.png) | [Segoe](images/typography-comparison/segoe-perform-200.png) | [Inter](images/typography-comparison/inter-specimen-200.png) / [Segoe](images/typography-comparison/segoe-specimen-200.png) |

Other 100% scenes: Learn [Inter](images/typography-comparison/inter-learn-100.png) /
[Segoe](images/typography-comparison/segoe-learn-100.png); Export
[Inter](images/typography-comparison/inter-export-100.png) /
[Segoe](images/typography-comparison/segoe-export-100.png); Settings
[Inter](images/typography-comparison/inter-settings-100.png) /
[Segoe](images/typography-comparison/segoe-settings-100.png).

The local viewer `out/typography-comparison/index.html` includes every scene at
all four scales, family toggles, actual-pixel/fit modes and original-image links.

## Portability and adoption boundary

Bundled static Inter avoids relying on a particular installed system family and
provides the same licensed face data for a future macOS build. Rasterization can
still differ across platforms; it must be checked when macOS is supported.
Segoe is dependable for the currently tested Windows installation, but a future
macOS implementation would need a licensed available face or a different policy.
This comparison does not establish that Segoe can be redistributed on macOS.

Four static files total **1,669,200 bytes** of uncompressed font data. They would
need deliberate packaging and OFL retention if adopted. If replacing the existing
879,708-byte variable file, the raw font-data increase would be 789,492 bytes;
this is not a measured final binary-size increase.

For adoption, retain the shared deterministic resolver and existing geometry;
do not introduce font-specific padding throughout the UI. Review a narrow,
consistent button-text fitting policy to preserve the approved one-line Zoom
appearance. That change and the fuller Inter footprint need explicit visual
approval before replacing Segoe. The subsequent owner approval explicitly
accepts the Zoom wrapping until the Perform redesign; see the decision above.

## Verification and retained experiment

- Isolated wrapper build: `scripts/build.ps1 -Configuration Release`, exit 0;
  configure/build passed. Build-only invocation did not run CTest.
- Review executable: `preverb_swell_ui_test.exe --compare-fonts`, exit 0, including
  face/style/ASCII checks, the component audit, region invariance and 40 renders.
  This is focused comparison evidence, not a new full-suite or host acceptance.
- Latest isolated build logs:
  `out/typography-comparison/worktree/out/build-logs/Release/20261103-005825-a31ae441/`.
- Local reproducibility material: `prepare_review.py`, `review-only-source.patch`,
  `font-provenance.json`, `font-metrics.json`, `component-font-audit.json` and
  `comparison-run.log` under `out/typography-comparison/`.
- Official release archive SHA-256:
  `9883fdd4a49d4fb66bd8177ba6625ef9a64aa45899767dde3d36aa425756b11e`.
- Primary-checkout runtime source, approved typography standards and plugin
  resources are unchanged. Approved Release binary SHA-256 remains
  `6fc0cfa0054173de1d085139dd3ab37bfc138502c1777095546bb88d477de889`.

These verification results describe the isolated comparison before adoption.
The subsequent approved production change is documented in the issue #16
evidence linked above. No merge was performed.
