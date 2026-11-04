# Header mode SVG assets

Canonical original artwork for this change: [Learn](learn.svg),
[Perform](perform.svg), [Export](export.svg). These are final vector sources for
later integration, not traced raster placeholders. No third-party icon pack is used.
They are deliberately not embedded in the plugin build yet.

| Property | Exact asset value |
| --- | --- |
| Canvas | `viewBox="0 0 16 16"`, nominal width/height 16 |
| Stroke | 1.6 units; round caps and joins; scales with the viewBox |
| Fill | None, including the three Perform handles |
| Inactive source ink | `#F8FAFC`, matching the existing navigation foreground |
| Intended display | 16 logical px; 14 px is the compact QA size, not a new layout |
| Optical placement | Canvas centre `(8,8)` centred on the label line; no per-icon translation |
| Safety | Geometry and stroke remain inside the viewBox; transparent background |

Learn has five centred waveform bars. Perform has precisely three outlined
1.35-unit-radius handles and separate cubic segments meeting their outer rims;
the strokes do not cross or fill the handle centres. The long rise and short
release communicate a reverse swell, rather than a generic ADSR envelope.
Export combines a folded audio page, three short waveform bars and an outward
arrow. It is navigation artwork, not a promise of implemented WAV output.

Only SVG path/circle primitives, explicit solid stroke and standard root
attributes are used. No CSS, `currentColor`, external references, fonts, filters,
masks, embedded bitmaps, scripts, fixed non-scaling stroke or symbol instances
are required. This keeps the artwork straightforward to load through JUCE SVG
support. Native JUCE loading/recolouring remains a future validation gate.

Future integration should load these exact SVGs once into cached drawables and
replace the explicit source ink with the existing control's foreground colour:
inactive `#F8FAFC`, active `#0F1418` on `#00EFFF`. Reuse existing disabled alpha
and hover/focus policy. Keep parsed originals immutable and prepare per-ink cached
copies outside paint; do not cumulatively recolour one global shared drawable.
Do not create separate raster or manually redrawn path
copies, parse SVG in paint, or infer icon identity from button text.

The companion [preview](preview.html) is a static browser asset-review sheet.
It is not a JUCE implementation or an interactive-control approval fixture.
The [PNG](preview.png) records the inspected browser sheet and
[asset checks](asset-checks.json) record structure, stroke bounds and hashes.
See [design.md](../../design.md) for exact integration geometry and the staged
validation checklist.
