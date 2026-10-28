#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Package the editable Skin Kit; generated archives always stay outside source."""
import argparse
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile, ZipInfo

root = Path(__file__).resolve().parent.parent
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output', type=Path, default=root / 'Working/Generated/SkinTemplate.zip')
args = parser.parse_args()
args.output.parent.mkdir(parents=True, exist_ok=True)
with ZipFile(args.output, 'w', ZIP_DEFLATED) as archive:
    for path in sorted((root / 'SkinTemplate').iterdir()):
        if path.is_file() and not path.name.startswith('.'):
            entry = ZipInfo(path.name, date_time=(2026, 1, 1, 0, 0, 0))
            entry.compress_type = ZIP_DEFLATED
            entry.external_attr = 0o100644 << 16
            archive.writestr(entry, path.read_bytes())
print(f'Skin kit: {args.output}')
