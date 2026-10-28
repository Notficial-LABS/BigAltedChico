#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Archive committed project files and the exact JUCE sources used for a binary release."""
import argparse
from datetime import datetime
import gzip
import hashlib
import io
import json
from pathlib import Path
import re
import shutil
import subprocess
import tarfile

ROOT = Path(__file__).resolve().parent.parent
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--juce-dir', required=True, type=Path)
parser.add_argument('--output-dir', type=Path, default=ROOT/'Working/Instalers/Source')
args = parser.parse_args()
juce = args.juce_dir.resolve()
if not (juce/'CMakeLists.txt').is_file(): parser.error('JUCE source directory is required.')
version = re.search(r'project\(BigAltedChico VERSION ([\d.]+)', (ROOT/'CMakeLists.txt').read_text()).group(1)
juce_version = re.search(r'project\(JUCE VERSION ([\d.]+)', (juce/'CMakeLists.txt').read_text()).group(1)
if juce_version != '9.0.4': parser.error('These preview build instructions require the JUCE 9.0.4 baseline.')
commit = subprocess.check_output(['git','rev-parse','HEAD'], cwd=ROOT, text=True).strip()
if subprocess.check_output(['git','status','--porcelain','--untracked-files=normal'], cwd=ROOT, text=True).strip():
    parser.error('Commit all publishable project changes before creating corresponding source.')
epoch = int(subprocess.check_output(['git','show','-s','--format=%ct','HEAD'], cwd=ROOT, text=True))
project = [Path(p) for p in subprocess.check_output(['git','ls-files','-z'], cwd=ROOT).decode().split('\0') if p]
if any(p.parts[0]=='Working' for p in project): parser.error('Working must never be tracked.')
# Exclude bundled executables and local instruction files, not build sources.
excluded = {'.git', '.DS_Store', 'AGENTS.md', 'CLAUDE.md'}
dependency = sorted(p for p in juce.rglob('*') if p.is_file() and not any(part in excluded or part.endswith('.app') for part in p.relative_to(juce).parts))
for p in [*(ROOT/p for p in project), *dependency]:
    if p.is_symlink(): parser.error(f'Unexpected symlink: {p.name}')
checksums = ''.join(hashlib.sha256(p.read_bytes()).hexdigest()+'  '+p.relative_to(juce).as_posix()+'\n' for p in dependency)
provenance = {'project_version':version,'project_commit':commit,'juce_version':juce_version,
              'juce_source_files':len(dependency),'juce_inventory_sha256':hashlib.sha256(checksums.encode()).hexdigest(),
              'juce_modifications':'Exact local source files used for the builds; inventory included.',
              'excluded_dependency_items':['bundled .app executables','.git','.DS_Store','AGENTS.md','CLAUDE.md']}
guide = '''# Corresponding source — BigAltedChico '''+version+'''

This archive contains the committed project and the JUCE source files used for
its macOS arm64 and Windows x64 preview builds. See SOURCE_INFO.json for the
project commit and JUCE_SOURCE_FILES.sha256 for the dependency inventory.
Signalsmith headers, artwork, font and applicable license notices are included.

From this extracted directory, with CMake, Python 3 and a C++17 toolchain:

    cmake -S . -B Working/build -DJUCE_DIR=Dependencies/JUCE -DCMAKE_BUILD_TYPE=Release
    cmake --build Working/build --config Release --parallel 3

The macOS preview used Apple Clang 21, deployment target 12.0 and JUCE 9.0.4.
The Windows x64 preview used LLVM/Clang-CL + LLD 23.1.4, Microsoft's CRT/SDK
prepared with xwin 0.10.0, and a static C++ runtime. The cross-build toolchain
file and packaging commands are documented in docs/BUILDING.md.

For a native Windows Visual Studio build, follow docs/BUILDING.md and point
JUCE_DIR at Dependencies/JUCE. Native Windows builds are a different toolchain
from the supplied cross-compiled preview. Windows SDK/CRT and general-purpose
compiler binaries are obtained separately, not included here.

On macOS add -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0 for the advertised minimum.
Installer scripts are in Tools/package_macos.py and Tools/package_windows.py.
Read LICENSE, Licenses/ and dependency notices for their respective terms.
Local file paths, credentials, personal audio and compiled applications are
not included in this source archive.
'''
args.output_dir.mkdir(parents=True,exist_ok=True)
output = args.output_dir/f'BigAltedChico-{version}-corresponding-source.tar.gz'
if output.exists():
    backup = ROOT/'Working/Backups'/('source-archive-'+datetime.now().strftime('%Y%m%d-%H%M%S'))
    backup.mkdir(parents=True);shutil.move(str(output),str(backup/output.name))
prefix = f'BigAltedChico-{version}-source/'
def add_bytes(archive,name,data):
    info=tarfile.TarInfo(prefix+name);info.size=len(data);info.mode=0o644;info.mtime=epoch
    archive.addfile(info,io.BytesIO(data))
with output.open('wb') as raw, gzip.GzipFile(filename='',fileobj=raw,mode='wb',mtime=epoch) as zipped, tarfile.open(fileobj=zipped,mode='w') as archive:
    for path in project:
        data = subprocess.check_output(['git','show',f'{commit}:{path.as_posix()}'], cwd=ROOT)
        info = archive.gettarinfo(str(ROOT/path), arcname=prefix+path.as_posix())
        info.uid=info.gid=0;info.uname=info.gname='';info.mtime=epoch;info.size=len(data)
        archive.addfile(info, io.BytesIO(data))
    for source in dependency:
        info = archive.gettarinfo(str(source), arcname=prefix+'Dependencies/JUCE/'+source.relative_to(juce).as_posix())
        info.uid=info.gid=0;info.uname=info.gname='';info.mtime=epoch
        with source.open('rb') as stream: archive.addfile(info,stream)
    add_bytes(archive,'SOURCE_INFO.json',(json.dumps(provenance,indent=2)+'\n').encode())
    add_bytes(archive,'JUCE_SOURCE_FILES.sha256',checksums.encode())
    add_bytes(archive,'BUILD_FROM_SOURCE.md',guide.encode())
print(f'Corresponding source: {output}\nProject commit: {commit}\nJUCE source files: {len(dependency)}')
