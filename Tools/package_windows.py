#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Package actual Windows x64 VST3/Standalone binaries with NSIS (also runs on macOS)."""
import argparse
from datetime import datetime
import hashlib
import json
from pathlib import Path
import re
import shutil
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parent.parent
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--build-dir', type=Path, default=ROOT/'Working/build-windows-msvc')
parser.add_argument('--output-dir', type=Path, default=ROOT/'Working/Instalers/Windows')
parser.add_argument('--makensis', default='makensis')
args = parser.parse_args()
products = args.build_dir.resolve()/'BigAltedChico_artefacts/Release'
app = products/'Standalone/BigAltedChico.exe'
vst = products/'VST3/BigAltedChico.vst3'
dll = vst/'Contents/x86_64-win/BigAltedChico.vst3'

def pe_machine(path):
    data=path.read_bytes()
    if data[:2]!=b'MZ': raise ValueError(f'Not a Windows PE file: {path}')
    start=struct.unpack_from('<I',data,0x3c)[0]
    if data[start:start+4]!=b'PE\0\0': raise ValueError(f'Invalid PE signature: {path}')
    return struct.unpack_from('<H',data,start+4)[0]
for p in [app,dll]:
    if not p.is_file(): parser.error(f'Missing Windows binary: {p}')
    if pe_machine(p)!=0x8664: parser.error(f'Expected AMD64 executable: {p}')
version=re.search(r'project\(BigAltedChico VERSION ([\d.]+)',(ROOT/'CMakeLists.txt').read_text()).group(1)
work=ROOT/'Working/Packaging';work.mkdir(parents=True,exist_ok=True)
stage=Path(tempfile.mkdtemp(prefix='windows-',dir=work))
shutil.copy2(app,stage/app.name)
shutil.copytree(vst,stage/vst.name)
licenses=stage/'Licenses';licenses.mkdir()
for source,name in [(ROOT/'LICENSE','MIT.txt'),(ROOT/'Licenses/AGPL-3.0.txt','AGPL-3.0.txt'),(ROOT/'Licenses/JUCE-LICENSE.md','JUCE-LICENSE.md'),(ROOT/'Assets/Roboto-OFL.txt','Roboto-OFL.txt'),(ROOT/'ThirdParty/SignalsmithStretch-LICENSE.txt','SignalsmithStretch-LICENSE.txt'),(ROOT/'ThirdParty/signalsmith-linear/LICENSE.txt','SignalsmithLinear-LICENSE.txt')]:
    shutil.copy2(source,licenses/name)
combined='BigAltedChico '+version+' - Notficial LABS\n\nOriginal code: MIT. The combined open-source application includes JUCE under AGPLv3.\n\n'
combined+='\n\n'.join(p.read_text() for p in sorted(licenses.iterdir()))
(licenses/'Combined-License.txt').write_text(combined,encoding='utf-8-sig')
(stage/'UninstallLicenses.nsh').write_text('\n'.join('Delete "$INSTDIR\\Licenses\\'+p.name+'"' for p in sorted(licenses.iterdir()))+'\n')
args.output_dir=args.output_dir.resolve();args.output_dir.mkdir(parents=True,exist_ok=True)
output=args.output_dir/f'BigAltedChico-{version}-Windows-x64-preview-Setup.exe'
if output.exists():
    backup=ROOT/'Working/Backups'/('windows-installer-'+datetime.now().strftime('%Y%m%d-%H%M%S'));backup.mkdir(parents=True)
    shutil.move(str(output),str(backup/output.name))
subprocess.run([args.makensis,'-V3',f'-DPAYLOAD={stage}',f'-DOUTPUT={output}',f'-DVERSION={version}',f'-DRESOURCES={ROOT/"Assets/Installer"}',str(ROOT/'Packaging/Windows/BigAltedChico.nsi')],check=True)
manifest={'package':output.name,'version':version,'payload_architecture':'AMD64','components':['VST3','Standalone'],'sha256':hashlib.sha256(output.read_bytes()).hexdigest(),'signed':False,'windows_runtime_tested':False,'files':{str(p.relative_to(products)):hashlib.sha256(p.read_bytes()).hexdigest() for p in [app,dll]}}
output.with_suffix('.json').write_text(json.dumps(manifest,indent=2)+'\n')
(args.output_dir/'SHA256SUMS.txt').write_text(manifest['sha256']+'  '+output.name+'\n')
print(f'Windows test installer: {output}')
