from pathlib import Path
from fontTools.ttLib import TTFont
from fontTools.varLib.instancer import instantiateVariableFont
from fontTools.pens.svgPathPen import SVGPathPen
from fontTools.pens.boundsPen import BoundsPen
from PIL import ImageFont, Image
from xml.sax.saxutils import escape
from concurrent.futures import ThreadPoolExecutor
import subprocess, json, shutil, zipfile, hashlib, math

WORK = Path(__file__).resolve().parent
OUT = WORK.parent / 'ReverbArise_Logo_Production_Pack'
OUT.mkdir(exist_ok=True)
for directory in ['svg', 'png/wordmark', 'png/tagline', 'png/mark', 'pdf', 'licenses', 'source']:
    (OUT / directory).mkdir(parents=True, exist_ok=True)

WHITE, AQUA, CHARCOAL = '#F5F7F9', '#00EFFF', '#0F1418'
FONT_SIZE = 100.0

def prepare_font(name, weight):
    f = instantiateVariableFont(TTFont(WORK / name), {'wght': weight, 'opsz': 32}, inplace=False)
    f.flavor = None
    p = WORK / f'{Path(name).stem}-{weight}-static.ttf'
    f.save(p)
    units = f['head'].unitsPerEm
    return f, ImageFont.truetype(str(p), units), units

bold, bold_layout, units = prepare_font('InterVariable.ttf', 700)
italic, italic_layout, italic_units = prepare_font('InterVariable-Italic.woff2', 400)

def text_paths(text, font, layout, em, size, x=0, baseline=0, fill=WHITE):
    glyphs, cmap = font.getGlyphSet(), font.getBestCmap()
    pieces = []
    scale = size / em
    for i, character in enumerate(text):
        if character == ' ':
            continue
        # Pillow's shaping supplies the actual Inter advances and pair kerning.
        offset = (layout.getlength(text[:i+1]) - layout.getlength(character)) * scale
        pen = SVGPathPen(glyphs)
        glyphs[cmap[ord(character)]].draw(pen)
        pieces.append(f'<path fill="{fill}" transform="translate({x+offset:.6f} {baseline:.6f}) scale({scale:.9f} {-scale:.9f})" d="{pen.getCommands()}"/>')
    return ''.join(pieces), layout.getlength(text) * scale

# A single continuous outlined ribbon, refined from the approved waveform-A.
# Open underside and no crossbar: the waveform replaces the A within Arise.
WAVE_D = ('M -8 0 C 5 0 12 -5 20 -17 L 49 -66 '
          'C 52 -71 54 -73 57 -73 C 60 -73 62 -71 65 -66 '
          'L 95 -17 C 103 -5 110 0 122 0 L 103 0 '
          'C 94 0 88 -4 82 -14 L 57 -55 L 32 -14 '
          'C 26 -4 18 0 7 0 Z')

def wordmark(first, second):
    first_paths, advance = text_paths('Reverb', bold, bold_layout, units, FONT_SIZE, fill=first)
    # Small intentional optical overlap joins the wave foot to the wordmark.
    wave_x = advance - 1.5
    wave = f'<path fill="{second}" transform="translate({wave_x:.6f} 0)" d="{WAVE_D}"/>'
    final_paths, rise_width = text_paths('rise', bold, bold_layout, units, FONT_SIZE, x=wave_x+116, fill=second)
    return first_paths + wave + final_paths, wave_x + 116 + rise_width

def svg_document(content, box, title, description):
    x,y,w,h = box
    return (f'<svg xmlns="http://www.w3.org/2000/svg" width="{w:.4f}" height="{h:.4f}" '
            f'viewBox="{x:.4f} {y:.4f} {w:.4f} {h:.4f}" role="img" aria-labelledby="title desc">'
            f'<title id="title">{escape(title)}</title><desc id="desc">{escape(description)}</desc>'
            f'{content}</svg>')

assets = []
def save_svg(name, content, box, family, sizes, description):
    p = OUT / 'svg' / f'{name}.svg'
    document = svg_document(content, box, name, description)
    if not p.exists() or p.read_text() != document:
        p.write_text(document, encoding='utf-8')
    assets.append({'name':name, 'path':p, 'family':family, 'sizes':sizes, 'box':box})

word_sizes = [256, 320, 512, 640, 1024, 2048, 4096]
tag_sizes = [512, 1024, 2048, 4096]
mark_sizes = [32, 64, 128, 256, 512, 1024]
variants = {
    'dark': (WHITE, AQUA),
    'light': (CHARCOAL, AQUA),
    'mono-white': (WHITE, WHITE),
    'mono-charcoal': (CHARCOAL, CHARCOAL),
}

for variant, colors in variants.items():
    content, width = wordmark(*colors)
    box = (-14, -91, width+28, 109)
    save_svg(f'reverbarise-wordmark-{variant}', content, box, 'wordmark', word_sizes,
             'Outlined Inter Bold wordmark with integrated waveform-A; transparent background; flat colours.')
    if variant in ['dark', 'light']:
        tagline = 'Reverse Reverb for Creative Swells'
        _, tagline_width = text_paths(tagline, italic, italic_layout, italic_units, 24, fill=colors[0])
        tagline_paths, _ = text_paths(tagline, italic, italic_layout, italic_units, 24,
                                     x=(width-tagline_width)/2, baseline=43, fill=colors[0])
        save_svg(f'reverbarise-tagline-{variant}', content+tagline_paths,
                 (-14,-91,width+28,151), 'tagline', tag_sizes,
                 'Outlined wordmark with the approved italic tagline. Transparent background; flat colours.')

for variant, color in [('aqua', AQUA), ('white', WHITE), ('charcoal', CHARCOAL)]:
    content = f'<path fill="{color}" d="{WAVE_D}"/>'
    save_svg(f'reverbarise-mark-{variant}', content, (-23,-116.5,160,160), 'mark', mark_sizes,
             'Waveform-A taken directly from the wordmark, centred on a transparent square. No tile or new symbol.')

def export_png(job):
    asset, size = job
    name, family = asset['name'], asset['family']
    target = OUT / 'png' / family / f'{name}-{size}px.png'
    if target.exists() and target.stat().st_mtime >= asset['path'].stat().st_mtime:
        try:
            with Image.open(target) as existing:
                existing.load()
                if existing.width == size and existing.mode == 'RGBA':
                    return target
        except OSError:
            pass
    subprocess.run(['inkscape', str(asset['path']), '--export-type=png',
                    f'--export-filename={target}', f'--export-width={size}',
                    '--export-background-opacity=0', '--export-area-page',
                    '--export-png-color-mode=RGBA_8'],
                   check=True, capture_output=True)
    return target

jobs = [(a, size) for a in assets for size in a['sizes']]
with ThreadPoolExecutor(max_workers=4) as pool:
    pngs = list(pool.map(export_png, jobs))

for asset in assets:
    if asset['family'] == 'mark':
        continue
    subprocess.run(['inkscape', str(asset['path']), '--export-type=pdf',
                    f'--export-filename={OUT / "pdf" / (asset["name"]+".pdf")}',
                    '--export-area-page'], check=True, capture_output=True)

checks = []
for p in pngs:
    with Image.open(p) as im:
        assert im.mode == 'RGBA', (p, im.mode)
        alpha = im.getchannel('A')
        assert alpha.getextrema() == (0,255), p
        box = alpha.getbbox()
        assert box and box[0]>0 and box[1]>0 and box[2]<im.width and box[3]<im.height, (p,box)
        assert all(alpha.getpixel(pos)==0 for pos in [(0,0),(im.width-1,0),(0,im.height-1),(im.width-1,im.height-1)]), p
        pixels = im.get_flattened_data() if hasattr(im, 'get_flattened_data') else im.getdata()
        full = [rgb[:3] for rgb in pixels if rgb[3]==255]
        allowed = {(245,247,249),(0,239,255),(15,20,24)}
        assert set(full).issubset(allowed), (p,set(full)-allowed)
        checks.append({'file':str(p.relative_to(OUT)), 'width':im.width,'height':im.height,
                       'mode':im.mode,'alpha_min':0,'alpha_max':255,'content_bounds':box})

for a in assets:
    s = a['path'].read_text()
    assert '<text' not in s and '<image' not in s and '<filter' not in s and '<linearGradient' not in s
    assert s.count('<svg')==1 and s.count('</svg>')==1

shutil.copyfile(WORK / 'OFL.txt', OUT / 'licenses' / 'Inter-OFL.txt')
shutil.copyfile(Path(__file__), OUT / 'source' / 'build_logos.py')
shutil.copyfile(WORK / 'InterVariable.ttf', OUT / 'source' / 'InterVariable.ttf')
shutil.copyfile(WORK / 'InterVariable-Italic.woff2', OUT / 'source' / 'InterVariable-Italic.woff2')
(OUT / 'export-manifest.json').write_text(json.dumps({'version':'1.0','font':'Inter, weight 700, optical size 32; tagline Inter Italic 400',
    'colors':{'aqua':AQUA,'white':WHITE,'charcoal':CHARCOAL},'exports':checks},indent=2))

# A vector contact sheet for visual QA. Individual delivery assets remain transparent.
rows = []
for idx, (variant, colors) in enumerate(variants.items()):
    content, width = wordmark(*colors)
    bg = CHARCOAL if variant in ['dark','mono-white'] else '#FFFFFF'
    fg = WHITE if bg==CHARCOAL else CHARCOAL
    y = 105+idx*195
    rows.append(f'<rect x="32" y="{y}" width="1136" height="176" rx="8" fill="{bg}"/>')
    rows.append(f'<text x="58" y="{y+30}" fill="{fg}" font-family="sans-serif" font-size="15">{escape(variant.upper())}</text>')
    rows.append(f'<g transform="translate(70 {y+153}) scale(1.5)">{content}</g>')
preview = ('<svg xmlns="http://www.w3.org/2000/svg" width="1200" height="920" viewBox="0 0 1200 920">'
           '<rect width="1200" height="920" fill="#E9EDF1"/>'
           '<text x="32" y="48" font-family="sans-serif" font-weight="700" font-size="28" fill="#0F1418">ReverbArise — production wordmarks</text>'
           '<text x="32" y="78" font-family="sans-serif" font-size="16" fill="#4A5058">Outlined vectors · Transparent exports · Shared geometry at every size</text>'
           + ''.join(rows) + '</svg>')
(OUT/'preview.svg').write_text(preview)
subprocess.run(['inkscape',str(OUT/'preview.svg'),'--export-type=png',f'--export-filename={OUT/"preview.png"}'],check=True,capture_output=True)

print(json.dumps({'directory':str(OUT),'svg_count':len(assets),'png_count':len(pngs),'pdf_count':len(list((OUT/'pdf').glob('*.pdf'))),
                  'preview':str(OUT/'preview.png'),'verified_pngs':len(checks)},indent=2))
