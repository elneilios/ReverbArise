# Compact UI mockup review — issue #58

## Approved direction and border clearance

The owner approved Option A revision 4 and authorized implementation. Subsequent
clarification requires consistent padding/margins and a visible gap wherever
structural perpendicular divider lines approach one another. The native review
profile names `outerMargin=12`, `contentPadding=8`, `groupGap=12` and
`dividerClearance=8`; divider endpoints must stop at least eight logical pixels
before a perpendicular boundary. These semantic roles remain distinct even when
their current values coincide. Review actual native text, focus ink and border
clearances before adopting the profile as a production standard.

The owner has also approved the interim 900x600 native layout. That accepts the
current visual direction; it does not replace the final combined native review
after the other active UI changes land. Production rollout remains pending.

The revision-4 CSS now shortens the main vertical dividers at both ends. Earlier
sections below retain the dated design-only evidence; native implementation and
acceptance progress is recorded in the [implementation record](../../validation/2026-11-05-issue-58-compact-ui.md).

Created 5 November 2026 as the owner's requested **pre-Apply design checkpoint** for `compact-responsive-ui`. The historical sections below preserve that review and its browser-only scope. Apply and interim native review have since started; their results belong to the linked implementation record. Static mockup acceptance does not replace native, DPI or FL Studio acceptance.

Open [the interactive review gallery](index.html) in a browser at **100% browser zoom**. Option A now opens revision 4: responsive typography, optical logo alignment, full-width notices and explicit comfort clearances. Use **Typography** to compare revisions 2/3; choose a **Review size** or drag the plugin's bottom-right resize grip to inspect responsive type. The [original revision-1 gallery](index-v1.html), all previous PNGs, Option B and real plugin baselines remain available. Numeric values, sound controls, resets and playback/file actions are display specimens; they do not run DSP or issue commands.

## Current composition — revision 4

The owner's follow-up identified logo misalignment, a truncated message, still-large text and insufficient separation from dividers/margins. Revision 4 addresses those concerns together:

- **Optical logo alignment:** the source SVG's transparent canvas/clear space and the image fitting box displaced the visible wordmark. A mockup-only SVG projection uses the measured path bounds, aligning the visible left edge at x12 with the content. Original logo paths, colours, proportions and canonical SVG remain unchanged. The derived `assets/wordmark-optical.svg` changes only its viewport/intrinsic dimensions.
- **Complete normal notice:** the existing warning receives the full 876-pixel information row at 900 width. The mode description moves beside the logo, with a two-line allowance. The complete current warning is visible without ellipsis at the normal 12-pixel compact notice role; ordinary long-message wrapping has a bounded 11-pixel two-line fallback rather than indefinite shrinking. This mockup does not establish a policy for arbitrary unbounded diagnostic text.
- **Comfort clearance:** 12-pixel outer horizontal margins; at least 8 pixels between header and main workspace, inspector content and each column edge, tabs and content, Sound content and Reset, and Reset and the next divider. Entry text has 6-pixel horizontal insets. Audition is restored to 48 pixels so 32-pixel actions have 8 pixels above and below. Numeric and button target boxes remain usable specimens rather than shrinking with their text.
- **Responsive text:** role sizes interpolate continuously from compact to larger values using both available width and height. A wider but short editor cannot force oversized text into compact vertical space. Type changes are independent of optional user magnification; this is a layout proposal, not OS DPI scaling.

| Role | 900x600 | 1100x850 | 1400x1000 |
| --- | --- | --- | --- |
| Main headings | 16 px | 18 px | 21 px |
| Mode navigation | 14 px | 16 px | 18 px |
| Body / tabs / buttons | 12 px | 13 px | 14 px |
| Captions / numeric values | 11 px | 12 px | 13 px |
| Notice | 12 px | 13 px | 14 px |

These are mockup CSS sizes and require native readability/interaction review before adoption. Smaller type reclaims visual space within existing hit areas. The compact future Sound module is 259x236 in both contexts, with 8-pixel inspector side clearances; 36-pixel dials and 32-pixel ordinary actions are retained. Some existing 11-pixel small text is deliberately not reduced further.

| Updated specimen | PNG |
| --- | --- |
| Learn: Capture & Generate | [900x600](mockups/v4/a4-learn-capture-generate.png) |
| Learn: future Convolution Sound | [900x600](mockups/v4/a4-learn-sound-convolution-future.png) |
| Learn: future Algorithmic Sound | [900x600](mockups/v4/a4-learn-sound-algorithmic-future.png) |
| Perform: future Convolution Sound | [900x600](mockups/v4/a4-perform-sound-convolution-future.png) |
| Perform: future Algorithmic Sound | [900x600](mockups/v4/a4-perform-sound-algorithmic-future.png) |
| Export Levels | [900x600](mockups/v4/a4-export-levels.png) |
| Responsive transition | [1000x750](mockups/v4/a4-perform-responsive-1000x750.png) |
| Larger responsive text | [1100x850](mockups/v4/a4-perform-responsive-1100x850.png) |
| Largest reviewed composition | [1400x1000](mockups/v4/a4-perform-responsive-1400x1000.png) |

`render_option_a_v4.cjs` verifies ten compact states and four responsive sizes. The recorded checks include complete current notice text bounds, logo/content alignment, inspector containment, the named 8-pixel clearances, increasing computed role sizes, arbitrary 1050x800 resizing and size retention across a tab change. See [revision-4 browser checks](mockups/v4/render-checks-v4.json). All remain browser evidence, not a native host/DPI or production-standards approval. No plugin implementation, saved project or OpenSpec artifact was modified.

## Current typography comparison — revision 3

The owner found revision 2 squeezed and requested smaller type to give the composition more breathing room. Revision 3 keeps the same 900x600 composition, click targets, 36-pixel dials and 26-pixel entry boxes. Smaller type occupies less of each existing box; paired-control column gaps increase from 8 to 12 pixels. Numeric values remain 12 pixels, and the existing 11-pixel filter On text is not reduced further.

| Role | Revision 2 | Calmer revision 3 |
| --- | --- | --- |
| Main section headings | 21 px | 18 px |
| Mode navigation | 18 px | 16 px |
| Capture subsection headings | 18 px | 16 px |
| Body, helper, ordinary button and tab text | 14 px in most consumers | 13 px |
| Parameter captions | 13 px | 12 px |
| Numeric values | 12 px | 12 px, unchanged |
| Ruler, lane captions and position footer | 14 px | 12 px |

| Calmer composition | Exact-size PNG |
| --- | --- |
| Learn: Capture & Generate | [900x600](mockups/v3/a3-learn-capture-generate.png) |
| Learn: current Sound | [900x600](mockups/v3/a3-learn-sound-current.png) |
| Learn: future Convolution | [900x600](mockups/v3/a3-learn-sound-convolution-future.png) |
| Learn: future Algorithmic Reverb | [900x600](mockups/v3/a3-learn-sound-algorithmic-future.png) |
| Perform: Timing | [900x600](mockups/v3/a3-perform-timing.png) |
| Perform: Envelope | [900x600](mockups/v3/a3-perform-envelope.png) |
| Perform: current Sound | [900x600](mockups/v3/a3-perform-sound-current.png) |
| Perform: future Convolution | [900x600](mockups/v3/a3-perform-sound-convolution-future.png) |
| Perform: future Algorithmic Reverb | [900x600](mockups/v3/a3-perform-sound-algorithmic-future.png) |
| Export: independent levels | [900x600](mockups/v3/a3-export-levels.png) |
| Expanded Perform | [1100x850](mockups/v3/a3-perform-expanded-1100x850.png) |

The ten revised compact states pass the same browser bounds, clipping, asset, pinned-action and shared-module checks, with no page errors. See [revision-3 browser checks](mockups/v3/render-checks-v3.json). `option-a-v3.css` and `option-a-v3.js` supply the optional typography treatment; `render_option_a_v3.cjs` reproduces it. The revision-2 renderer explicitly requests its original typography so its retained comparison remains reproducible.

This is a mockup typography proposal. Native readability, actual JUCE font metrics, focus/hit behavior and the required standards/OpenSpec reconciliation still need explicit review before production adoption. Captured waveform pixels inside the mockups do not change their embedded text size when browser text roles change.

## Current Option A — revision 2

Updated at the owner's request on 5 November 2026, before Apply. Learn now has **Capture & Generate / Sound** tabs, with Generate, readiness and manual-placement help pinned below both tabs. Default IR lives in Sound. The future-state view uses the same 267x236 Sound module in Learn defaults and Perform selected-swell editing; changing the specimen Character selector discloses its relevant engine controls.

| Revised composition | Exact-size PNG |
| --- | --- |
| Learn: Capture & Generate | [900x600](mockups/v2/a2-learn-capture-generate.png) |
| Learn: current Sound / Default IR only | [900x600](mockups/v2/a2-learn-sound-current.png) |
| Learn: future Convolution Sound | [900x600](mockups/v2/a2-learn-sound-convolution-future.png) |
| Learn: future Algorithmic Reverb Sound | [900x600](mockups/v2/a2-learn-sound-algorithmic-future.png) |
| Perform: Timing | [900x600](mockups/v2/a2-perform-timing.png) |
| Perform: Envelope | [900x600](mockups/v2/a2-perform-envelope.png) |
| Perform: current Sound / IR and Level | [900x600](mockups/v2/a2-perform-sound-current.png) |
| Perform: future Convolution Sound | [900x600](mockups/v2/a2-perform-sound-convolution-future.png) |
| Perform: future Algorithmic Reverb Sound | [900x600](mockups/v2/a2-perform-sound-algorithmic-future.png) |
| Export: Mix Character and independent Dry/Wet | [900x600](mockups/v2/a2-export-levels.png) |
| Perform: larger responsive composition | [1100x850](mockups/v2/a2-perform-expanded-1100x850.png) |

**Future-state scope:** Character selects IR / Convolution or an illustrative Algorithmic Reverb. Convolution shows its IR/import controls; Algorithmic shows three capacity slots labelled Decay, Size and Damp. Both states include Tone, Low Cut, High Cut, Stretch and Level. Filter On controls explicitly reserve bypass access. No unsupported engine controls appear in the current-controls view.

The illustrative algorithm parameters and values are not an approved parameter set or sonic defaults for #30. Tone's numeric domain remains undecided; its neutral `0` is a specimen, not a dB claim. Stretch shows neutral 1x without deciding its pitch, duration or arrival policy (#85/#86). Cutoff values are sample display values, not new defaults. A Learn-default Level control is included as a capacity reservation; #80 still owns only its delivered shared IR foundation, not this hypothetical extension. This artifact authorizes no production placeholders or DSP implementation.

Export follows the owner's [supplied reference](reference/export-levels-owner-reference.png): **Export Levels**, **Mix Character: Wet Layer**, independent **Dry 0.0% / Wet 100.0%**, percentage legend, drag target, Save and Return. Issue #51 owns the actual additive mixer/headroom behavior. Export Mix Character is a gain-pair preset; it is distinct from the Learn/Perform processing Character.

### Revised compact budget

| Region / role | Revision 2 measurement | Change from revision 1 |
| --- | --- | --- |
| Shared header | y12, h64 | 82 -> 64; mode names remain 18 px, main headings 21 px |
| Main Perform columns | y78, h380 | 40 more vertical pixels; same 160 / 416 / 276 widths |
| Inspector content | 238 high | 198 -> 238 |
| Future Sound module | 267x236 | Character 28 + engine area 46 + three 54-pixel treatment rows |
| Audition | y458, h44 | 48 -> 44 |
| Overview | y502, h58 | 70 -> 58 |
| Host position / help | y560, h20 / y580, h20 | Unchanged; normal 14 px help |
| Dials | 36x36 | 42 -> 36 |
| Field labels / numeric values | 13 / 12 px | 14 / 13 -> 13 / 12 |
| Ordinary actions / exact-entry height | 32 / 26 | 34 / 28 -> 32 / 26 |

These are scoped **mockup alternatives requiring explicit native look/feel and standards review**, not already approved production tokens. The existing OpenSpec plan still describes invariant roles; adopting these smaller roles requires deliberately reconciling that plan and the production standards before rollout. Headings, mode navigation and normal help are preserved; the compact change targets field/entry rows and space allocation.

Ten revised compact PNGs passed exact canvas, region bounds, parameter-caption/entry bounds, loaded-asset and pinned-action overlap checks. The module geometry matches in both contexts; engine disclosure and tab navigation checks pass with no browser script errors. See [revision-2 browser checks](mockups/v2/render-checks-v2.json). Waveform pixels remain captured display references rather than proof of native waveform/handle rendering or hit targets. No native build or DSP/audio test is implied.

`option-a-v2.css` and `option-a-v2.js` override Option A only. `render_option_a_v2.cjs` reproduces the new renders. `render_mockups.cjs` now explicitly uses `index-v1.html` to reproduce the retained original renders. No real plugin, project file or OpenSpec planning artifact was changed during this revision.

## Real plugin baselines

FL Studio was initially closed. The existing recent `PreverbTesting.flp` project was opened, then its existing Piano ReverbArise instance. Transport remained stopped; no Reset, Generate, capture, playback or file-delivery action was invoked. Mode changes were used to obtain the screenshots; Export prepared its preview through its ordinary entry behavior. The instance was returned to Perform with the inspector at the top. No project save was requested or performed.

| Mode / state | Plugin-content crop | FL Studio application capture |
| --- | --- | --- |
| Learn | [1100x850](baseline/learn-plugin.png) | [1920x1032](baseline/learn-desktop.png) |
| Perform, initial inspector | [1100x850](baseline/perform-plugin.png) | [1920x1032](baseline/perform-desktop.png) |
| Export, ready | [1100x850](baseline/export-plugin.png) | [1920x1032](baseline/export-desktop.png) |
| Perform, Reset revealed by scrolling | [1100x850](baseline/perform-reset-revealed-plugin.png) | [1920x1032](baseline/perform-reset-revealed-desktop.png) |

The display was reported by the owner as 1920x1080. Computer Use captured the maximized FL Studio application/work-area window at **1920x1032**, excluding the remaining screen area. The plugin-content rectangle is `(770,122)-(1870,972)` in those captures: exactly 1100x850 pixels. The saved crops only remove the surrounding host; the baseline content is unaltered. `perform-clean` is a further capture after restoring the original inspector scroll position, used for waveform references without an overview mouse pointer.

Installed plugin build/version and Windows DPI were not independently retrieved. These screenshots establish the UI actually running on the owner's machine; they do not certify binary parity with the planning checkout `84894ee` or a Windows DPI setting.

## Retained Option A — revision 1

| Mode / section | Exact-size PNG |
| --- | --- |
| Learn, all existing settings visible | [900x600](mockups/a-learn.png) |
| Perform, Timing | [900x600](mockups/a-perform-timing.png) |
| Perform, Envelope | [900x600](mockups/a-perform-envelope.png) |
| Perform, Sound | [900x600](mockups/a-perform-sound.png) |
| Export | [900x600](mockups/a-export.png) |
| Perform, responsive expansion | [1100x850](mockups/a-perform-expanded-1100x850.png) |

**Recommended starting point.** The three persistent inspector labels expose the complete current control set: Timing has Target, Move Wet and both Source bounds; Envelope has Pre-roll, End Offset, Fade In/Out and Curve In/Out; Sound has IR/import/status and Level. Every current parameter in the selected section fits without inspector scrolling. Enabled/Solo/Freeze and both existing Reset / Reset All actions remain visible across sections.

Learn fits its present capture, detection, generation, Default IR and sensitivity controls without an extra tab. Its redundant “Used by Generate” hint moves to the normal-sized bottom help row. Export bounds the drag target so its current mix, playback, navigation and file actions fit.

At larger sizes the same control sizes are retained, the timeline grows, more swell rows become visible and navigation groups share one row. No whole-editor raster shrinking is used.

## Option B — scrolling inspector with pinned actions

| Mode / state | Exact-size PNG |
| --- | --- |
| Learn, Generate section | [900x600](mockups/b-learn.png) |
| Learn, Capture section | [900x600](mockups/b-learn-capture.png) |
| Perform, top of inspector | [900x600](mockups/b-perform.png) |
| Perform, lower inspector | [900x600](mockups/b-perform-scrolled.png) |
| Export | [900x600](mockups/b-export.png) |

The inspector keeps a continuous control stack with visible overflow and labelled jump links. Reset / Reset All are outside the scrolling content. This keeps the existing stacked grouping closer to its current form but needs more scrolling and provides less central width than A. Learn's Capture / Generate tabs demonstrate a second way to accommodate future density; its Generate action stays visible. Export uses a shorter drag tile.

The specimen scroll cue is static; a production implementation would update overflow/position cues to match the real scroll state. Both options require native review of the final overflow/section family and complete keyboard access.

## Space budget and fidelity

| Compact region | Option A | Option B |
| --- | --- | --- |
| Canvas | 900x600 | 900x600 |
| Shared header | x12, y12, w876, h82 | Same |
| Main columns | y102, h340 | Same |
| List / centre / inspector | 160 / 416 / 276 | 144 / 388 / 320 |
| Column gaps | 12 | 12 |
| Audition | y442, h48 | Same |
| Overview | y490, h70 | Same |
| Host position / bottom help | y560, h20 / y580, h20 | Same |
| Inspector selected content | 198 high | 178 scroll viewport plus 20-pixel cue |
| Inspector Reset footer | y408, h34 | Same |

The current wordmark SVG, mode icon SVGs, embedded Inter font files, semantic palette, flat rules, control concepts and captured waveform appearance are reused. Nominal browser text is 21-pixel headings, 18-pixel mode navigation, 14-pixel labels/help and 13-pixel numeric entries. Dials remain 42x42; ordinary actions remain 34 pixels high and numeric entries 28 pixels high. This preserves readable control dimensions while reducing layout area by approximately **42%** compared with 1100x850.

Proposed exceptions requiring owner/native review: captions above dial/value pairs, persistent inspector section navigation or jump/overflow composition, revised region metrics and tagline omission at compact widths. These values are design hypotheses, not new approved production tokens.

The waveform regions are cropped display references from the real captures, resized inside the mockup. Embedded waveform handles/rulers in those regions are illustrative; their resized pixels do **not** prove native hit areas, physical sharpness, audio correctness or pixel-perfect JUCE typography. Browser Inter metrics are not identical to JUCE font-height metrics. Native implementation must redraw waveforms/handles at the actual resolution and preserve their hit geometry.

The 900x600 PNGs have exactly that intrinsic pixel size. Chat previews or image viewers may shrink them; view at 100% or use the gallery at 100% browser zoom for the intended size. Browser CSS pixels can still be enlarged by Windows DPI. [1920x1080 review-page render](mockups/review-at-1920x1080.png) shows the gallery's footprint, not an FL Studio host capture of an implemented compact plugin.

## Verification and reproduction

- Eight principal compact renders passed exact 900x600 canvas, region-containment and loaded-image checks. A's parameter captions/entries passed section-bounds and caption-overflow checks.
- Section checks enumerate all 4 Timing, 6 Envelope and 1 Level controls; IR/import/status are additionally visible in Sound. Reset remains visible in every section and after B scrolls.
- Additional PNGs show B's lower scroll state, B's Capture section and A's 1100x850 expansion. Visual inspection covered all principal modes and A's inspector states.
- [Machine-readable browser checks](mockups/render-checks.json) are browser-layout evidence only. No native plugin code, source/audio/session state, OpenSpec planning artifact or production standard was changed. No native build, MSVC/CTest or host compact-layout/DPI validation was run.

`prepare_assets.py` uses the bundled Python/Pillow runtime to crop waveform references. `render_mockups.cjs` uses the configured bundled Node/Playwright runtime and installed Chrome to render local HTML at device scale 1. The scripts retain the paths used on this PC and require no downloaded browser or network service.

The original A/B decision is complete: Option A revision 4 is the approved
direction. The next delivery gate is integration after upstream UI changes,
followed by a fresh combined native 900x600 preview before finalisation.
