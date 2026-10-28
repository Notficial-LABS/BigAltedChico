#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Export approved SVG artwork to JUCE/NSIS/macOS icon resources.
Requires Pillow and resvg-py. Run only when the SVG artwork changes.
"""
import io
from pathlib import Path
import resvg_py
from PIL import Image
ROOT = Path(__file__).resolve().parent.parent
icons = ROOT/'Assets/Icons'
for platform in ['mac', 'windows']:
    png = resvg_py.svg_to_bytes(svg_path=str(icons/f'BAC-{platform}.svg'), width=1024, height=1024)
    (icons/f'BAC-{platform}.png').write_bytes(png)
    image = Image.open(io.BytesIO(png)).convert('RGBA')
    if platform == 'windows':
        image.save(icons/'BAC-windows.ico', sizes=[(n,n) for n in (16,24,32,48,64,128,256)])
    else:
        image.save(icons/'BAC-mac.icns')
print('Updated PNG, ICO and ICNS resources from the approved SVG originals.')
