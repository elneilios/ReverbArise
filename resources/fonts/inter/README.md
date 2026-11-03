# Bundled Inter static faces

Unmodified static TTFs from the official Inter 4.1 release:
https://github.com/rsms/inter/releases/download/v4.1/Inter-4.1.zip

| File | Release path | Weight |
| --- | --- | --- |
| Inter-Regular.ttf | extras/ttf/Inter-Regular.ttf | 400 |
| Inter-SemiBold.ttf | extras/ttf/Inter-SemiBold.ttf | 600 |
| Inter-Bold.ttf | extras/ttf/Inter-Bold.ttf | 700 |
| Inter-Italic.ttf | extras/ttf/Inter-Italic.ttf | 400, italic |

`OFL.txt` is the release's `LICENSE.txt` (SIL Open Font License 1.1).
The VST3 build copies it to `Contents/Resources/Inter-OFL.txt`.
These files are embedded through JUCE BinaryData; no system installation is used.
The release ZIP SHA-256 is
`9883fdd4a49d4fb66bd8177ba6625ef9a64aa45899767dde3d36aa425756b11e`.

The comparison and owner decision are recorded in
[the typography evidence](../../../docs/validation/2026-11-03-static-inter-comparison.md).
