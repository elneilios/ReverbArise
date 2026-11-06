"""Crop captured display references; no source audio or native UI is modified."""
from pathlib import Path
from PIL import Image

root = Path(__file__).resolve().parent
assets = root / 'assets'
assets.mkdir(exist_ok=True)
regions = {
    'dry-lane': ('perform-clean', (214, 190, 762, 345)),
    'wet-lane': ('perform-clean', (214, 376, 762, 531)),
    'overview': ('perform-clean', (16, 736, 1072, 790)),
    'learn-waveform': ('learn', (348, 216, 1084, 584)),
    'export-waveform': ('export', (16, 221, 710, 726)),
}
for name, (mode, rect) in regions.items():
    Image.open(root / 'baseline' / f'{mode}-plugin.png').crop(rect).save(assets / f'{name}.png')
print('Prepared five cropped waveform references from real plugin captures.')
