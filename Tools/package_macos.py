#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build an unsigned macOS .pkg with user/system scope and AU/VST3/Standalone choices."""
import argparse
from datetime import datetime
import hashlib
from html import escape
import json
from pathlib import Path
import platform
import plistlib
import shutil
import subprocess
import tempfile
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parent.parent
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--build-dir', type=Path, default=ROOT / 'Working/build')
parser.add_argument('--output-dir', type=Path, default=ROOT / 'Working/Instalers/MacOS')
args = parser.parse_args()
if platform.system() != 'Darwin':
    parser.error('pkgbuild/productbuild require macOS; this does not create Windows or Linux installers.')
args.build_dir = args.build_dir.resolve()
args.output_dir = args.output_dir.resolve()
products = args.build_dir / 'BigAltedChico_artefacts/Release'
components = [
    ('au', 'Audio Units (AU)', 'AU/BigAltedChico.component', 'Library/Audio/Plug-Ins/Components', 'For Logic Pro, GarageBand and AU-compatible hosts.'),
    ('vst3', 'VST3', 'VST3/BigAltedChico.vst3', 'Library/Audio/Plug-Ins/VST3', 'For Ableton Live and other VST3-compatible hosts.'),
    ('standalone', 'Standalone', 'Standalone/BigAltedChico.app', 'Applications', 'Run without a DAW. Audio device settings are available inside the application.'),
]
versions = set()
for _, _, relative, _, _ in components:
    source = products / relative
    if not source.is_dir():
        parser.error(f'Missing bundle: {source}. Build all macOS targets first.')
    subprocess.run(['codesign', '--verify', '--deep', '--strict', str(source)], check=True)
    info = plistlib.loads((source / 'Contents/Info.plist').read_bytes())
    versions.add(info['CFBundleShortVersionString'])
    binary = source / 'Contents/MacOS/BigAltedChico'
    arch = subprocess.check_output(['lipo', '-archs', str(binary)], text=True).strip()
    if arch != 'arm64':
        parser.error('This first test installer is arm64-only; validate other architectures before packaging them.')
if len(versions) != 1:
    parser.error('All bundles must have the same version.')
version = versions.pop()
work = ROOT / 'Working/Packaging'
work.mkdir(parents=True, exist_ok=True)
stage = Path(tempfile.mkdtemp(prefix='macos-', dir=work))
resources = stage / 'Resources'
resources.mkdir()
for name in ['conclusion.html']:
    shutil.copy2(ROOT / 'Packaging/macOS' / name, resources / name)
# productbuild expects a regular file here. The native RTFD document and its
# embedded banner are inserted after productbuild, before final flattening.
(resources / 'welcome.rtf').write_text(r'{\rtf1\ansi BigAltedChico}')
license_text = (ROOT / 'LICENSE').read_text() + '\n\nJUCE AND THE COMBINED OPEN-SOURCE DISTRIBUTION\n\n' + (ROOT / 'Licenses/JUCE-LICENSE.md').read_text() + '\n\n' + (ROOT / 'Licenses/AGPL-3.0.txt').read_text()
(resources / 'license.html').write_text('<html><meta charset="utf-8"><body><pre style="white-space:pre-wrap;font:12px Helvetica">' + escape(license_text) + '</pre></body></html>')
scripts = stage / 'Scripts'
scripts.mkdir()
shutil.copy2(ROOT / 'Packaging/macOS/preinstall', scripts / 'preinstall')
packages = stage / 'Packages'
packages.mkdir()
ids = {}
for key, title, relative, destination, description in components:
    source = products / relative
    payload = stage / ('root-' + key)
    target = payload / destination / source.name
    target.parent.mkdir(parents=True)
    subprocess.run(['ditto', str(source), str(target)], check=True)
    config = stage / (key + '-bundles.plist')
    config.write_bytes(plistlib.dumps([{
        'RootRelativeBundlePath': str(target.relative_to(payload)),
        'BundleIsRelocatable': False,
        'BundleIsVersionChecked': True,
        'BundleHasStrictIdentifier': True,
        'BundleOverwriteAction': 'upgrade',
    }]))
    identifier = 'com.notficiallabs.bigaltechico.pkg.' + key
    ids[key] = identifier
    subprocess.run(['pkgbuild', '--root', str(payload), '--identifier', identifier,
                    '--version', version, '--install-location', '/', '--component-plist', str(config),
                    '--scripts', str(scripts), str(packages / (key + '.pkg'))], check=True)
# License files are installed separately from mutable user presets/skins/settings.
payload = stage / 'root-notices'
notices = payload / 'Library/Application Support/Notficial LABS/BigAltedChico/Licenses'
notices.mkdir(parents=True)
for source in [ROOT/'LICENSE', ROOT/'Licenses/AGPL-3.0.txt', ROOT/'Licenses/JUCE-LICENSE.md', ROOT/'Assets/Roboto-OFL.txt', ROOT/'ThirdParty/SignalsmithStretch-LICENSE.txt']:
    shutil.copy2(source, notices/source.name)
shutil.copy2(ROOT/'ThirdParty/signalsmith-linear/LICENSE.txt', notices/'SignalsmithLinear-LICENSE.txt')
ids['notices'] = 'com.notficiallabs.bigaltechico.pkg.notices'
subprocess.run(['pkgbuild', '--root', str(payload), '--identifier', ids['notices'], '--version', version, '--install-location', '/', str(packages/'notices.pkg')], check=True)
dist = ET.Element('installer-gui-script', {'minSpecVersion':'2'})
ET.SubElement(dist, 'title').text = 'BigAltedChico'
ET.SubElement(dist, 'options', {'customize':'always', 'require-scripts':'false', 'hostArchitectures':'arm64'})
ET.SubElement(dist, 'domains', {'enable_anywhere':'false', 'enable_currentUserHome':'true', 'enable_localSystem':'true'})
checks = ET.SubElement(dist, 'volume-check')
ET.SubElement(ET.SubElement(checks, 'allowed-os-versions'), 'os-version', {'min':'12.0'})
ET.SubElement(dist, 'welcome', {'file':'welcome.rtf', 'mime-type':'text/rtf'})
for tag, name in [('license','license.html'), ('conclusion','conclusion.html')]:
    ET.SubElement(dist, tag, {'file':name, 'mime-type':'text/html'})
outline = ET.SubElement(dist, 'choices-outline')
for key in [c[0] for c in components] + ['notices']:
    ET.SubElement(outline, 'line', {'choice':key})
for key, title, relative, destination, description in components:
    choice = ET.SubElement(dist, 'choice', {'id':key, 'title':title, 'description':description,
        'start_selected':'true', 'start_enabled':'true', 'start_visible':'true'})
    ET.SubElement(choice, 'pkg-ref', {'id':ids[key]})
choice = ET.SubElement(dist, 'choice', {'id':'notices','title':'License notices','visible':'false',
    'selected':'choices.au.selected || choices.vst3.selected || choices.standalone.selected'})
ET.SubElement(choice, 'pkg-ref', {'id':ids['notices']})
for key, identifier in ids.items():
    # Installer derives authorization from the chosen domain: the current
    # user for home installs, administrator authorization for system installs.
    node = ET.SubElement(dist, 'pkg-ref', {'id':identifier, 'version':version, 'onConclusion':'None'})
    node.text = key + '.pkg'
ET.indent(dist)
distribution = stage / 'Distribution.xml'
ET.ElementTree(dist).write(distribution, encoding='utf-8', xml_declaration=True)
args.output_dir.mkdir(parents=True, exist_ok=True)
output = args.output_dir / f'BigAltedChico-{version}-macOS-arm64-preview.pkg'
if output.exists():
    backup = ROOT/'Working/Backups'/('installer-'+datetime.now().strftime('%Y%m%d-%H%M%S'))
    backup.mkdir(parents=True)
    shutil.move(str(output), str(backup/output.name))
intermediate = stage / 'product.pkg'
subprocess.run(['productbuild', '--distribution', str(distribution), '--package-path', str(packages), '--resources', str(resources), str(intermediate)], check=True)
# RTFD stores the banner as an attachment with its physical size, avoiding
# Installer's HTML/CSS sizing problems and broken relative image URLs.
expanded = stage / 'expanded'
subprocess.run(['/usr/sbin/pkgutil', '--expand', str(intermediate), str(expanded)], check=True)
shutil.copytree(ROOT / 'Packaging/macOS/welcome.rtfd', expanded / 'Resources/welcome.rtfd')
(expanded / 'Resources/welcome.rtf').unlink()
final_distribution = ET.parse(expanded / 'Distribution')
welcome = final_distribution.getroot().find('welcome')
welcome.attrib.clear()
welcome.attrib.update({'file':'welcome.rtfd', 'uti':'com.apple.rtfd'})
final_distribution.write(expanded / 'Distribution', encoding='utf-8', xml_declaration=True)
subprocess.run(['/usr/sbin/pkgutil', '--flatten', str(expanded), str(output)], check=True)
subprocess.run(['swift', str(ROOT/'Tools/set_macos_file_icon.swift'),
                str(ROOT/'Assets/Icons/BAC-mac.icns'), str(output)], check=True)
# Finder custom icons are filesystem metadata. Preserve them when sharing the
# package by also providing a ZIP with AppleDouble/resource-fork information.
archive = output.with_suffix('.zip')
if archive.exists():
    backup = ROOT/'Working/Backups'/('macos-icon-archive-'+datetime.now().strftime('%Y%m%d-%H%M%S'))
    backup.mkdir(parents=True)
    shutil.move(str(archive), str(backup/archive.name))
subprocess.run(['ditto', '-c', '-k', '--sequesterRsrc', '--keepParent', str(output), str(archive)], check=True)
manifest = {'package':str(output), 'stage':str(stage), 'version':version, 'architecture':'arm64',
            'domains':['CurrentUserHomeDirectory', 'LocalSystem'], 'components':list(ids), 'developer_id_signed':False, 'notarized':False,
            'finder_icon':True, 'icon_preserving_archive':archive.name}
(output.with_suffix('.json')).write_text(json.dumps(manifest,indent=2)+'\n')
(args.output_dir/'SHA256SUMS.txt').write_text(''.join(
    hashlib.sha256(p.read_bytes()).hexdigest()+'  '+p.name+'\n' for p in [output, archive]))
print(f'Test installer: {output}\nStaging: {stage}')
