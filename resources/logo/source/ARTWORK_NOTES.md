# Artwork provenance

The initial built-in image-generation request used the supplied `ReverbArise_Visual_Style_Reference.png` as a reference and asked for: “Reproduce only the approved horizontal ReverbArise wordmark: light Reverb, aqua waveform-shaped A integrated into Arise, aqua rise; match the modern Inter Bold appearance; flat #F5F7F9 and #00EFFF; transparent alpha background; one wordmark, no tagline, tile, captions, shadows or glow.”

That generated PNG was rejected because its edges and alpha were not clean enough for production. It is not embedded in or used as the pixel source of the delivered artwork.

The delivered production assets were reconstructed natively as SVG paths using Inter at weight 700 and optical size 32, with an explicitly drawn continuous waveform-A ribbon based on the approved reference. Taglines use outlined Inter Italic at weight 400 and optical size 32. Colour variants and all raster sizes are derived from these same vector masters using Inkscape. No logo letters are raster autotraces. There are no baked backgrounds or bitmap-only SVG wrappers.

The pack's manifest records checked raster dimensions and alpha. The waveform path and type placement are editable in the included build source or any standard SVG editor.
