# ReverbArise logo production pack

Production vector refinement of the approved ReverbArise wordmark reference. The typography uses outlined Inter Bold, with a custom outlined waveform-A refined from the supplied reference and integrated into “Arise”. This is a clean reconstruction of the approved direction, rather than a pixel trace of the generated reference image.

The standalone mark is the exact same waveform-A lifted from the wordmark. It does not introduce the conceptual square app icon, decorative waves, gradients or a new symbol.

## Choose an asset

| Variant | Use |
| --- | --- |
| `wordmark-dark` | Light “Reverb” and aqua “Arise”, for the plugin and dark backgrounds |
| `wordmark-light` | Charcoal “Reverb” and aqua “Arise”, for white/light documents and marketing |
| `wordmark-mono-white` | Entire wordmark in the approved light colour, for dark backgrounds |
| `wordmark-mono-charcoal` | Entire wordmark in charcoal, for light backgrounds or single-colour printing |
| `tagline-dark` / `tagline-light` | Wordmark with the approved “Reverse Reverb for Creative Swells” tagline |
| `mark-aqua` / `mark-white` / `mark-charcoal` | Waveform-A only, centred on a transparent square |

The light colour is the approved off-white `#F5F7F9`, rather than pure white. Aqua is `#00EFFF`; charcoal is `#0F1418`. Aqua can have low contrast against white at very small sizes; choose the monochrome charcoal wordmark when maximum small-size legibility is needed.

## Files and sizes

- **SVG:** nine scalable masters. Lettering and marks are actual paths; there are no live fonts, embedded bitmaps, filters, gradients or external dependencies. Transparent backgrounds.
- **PNG wordmarks:** widths of 256, 320, 512, 640, 1024, 2048 and 4096 pixels in four colour variants. Heights preserve the master proportions, including clear space.
- **PNG tagline lockups:** widths of 512, 1024, 2048 and 4096 pixels in two colour variants.
- **PNG marks:** 32, 64, 128, 256, 512 and 1024 pixels square in three colour variants.
- **PDF:** six vector wordmarks/lockups for placement in documents and print layouts. There is no painted background; some PDF viewers display the transparent page as white, which hides the off-white version until placed over a dark surface.
- **Preview:** `preview.png` / `preview.svg` show the four wordmark variants against appropriate example backgrounds. These preview backgrounds are not part of the logo assets.
- **Manifest:** `export-manifest.json` records dimensions, colour tokens and alpha validation for every PNG.
- **Source:** the reproducible vector/export build script and font source inputs, with the Inter font licence under `licenses/`.

## Plugin placement

Prefer the dark SVG for resolution-independent rendering where supported by the UI framework. Preserve the SVG viewBox and aspect ratio. Do not stretch the waveform-A separately from the wordmark.

For a 256 logical-pixel-wide PNG placement, use the 256 px export at 1× or the 512 px export at 2×. For a 320 logical-pixel placement, use 320 px or 640 px respectively. The supplied widths are canvas widths, including built-in clear space, not the visible letter bounds.

PNG files have genuine RGBA transparency, including the letter counters and space under the waveform. Do not remove the transparent border or add a black/white matte. Pick the intended colour variant for the surface rather than inverting the entire image.

Use the plain wordmark at small sizes; the tagline lockups are intended for larger presentation and marketing placements. Check final display/print size in context. SVG artwork is resolution-independent; PNGs are supplied at defined pixel dimensions and should not be enlarged beyond their intended use.

## Validation

All PNGs are exported from the same vector geometry. Automated checks verify RGBA output, transparent corners and margins, non-empty content, unclipped bounds and the approved flat colours in fully opaque interior pixels. Antialiased edges contain partial alpha, as expected. Every SVG is checked to contain path artwork rather than live text or raster images. The contact sheet is visually reviewed before delivery.

## Rebuilding

The source script requires Python with `fontTools`, `Pillow` and `brotli`, plus an `inkscape` executable. Run `python3 build_logos.py` from the supplied `source` folder with its two font inputs present. It writes a sibling `ReverbArise_Logo_Production_Pack` directory. Font inputs came from the Inter project: https://github.com/rsms/inter/tree/master/docs/font-files . Exported SVG/PNG/PDF assets do not require installing those fonts.

The discarded image-generation attempt is not a production master and is not included in this pack.
