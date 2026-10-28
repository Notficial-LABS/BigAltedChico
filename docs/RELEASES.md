# Release plan

The public repository contains source and editable resources. Versioned installers belong in **GitHub Releases**, not Git commits. All local staging/output belongs under ignored `Working/`. Installers are grouped in `Working/Instalers/MacOS`, `Working/Instalers/Windows` and `Working/Instalers/Linux`.

## Formats

| OS | Ship |
| --- | --- |
| macOS | AU, VST3, Standalone |
| Windows | VST3, Standalone |
| Linux | VST3, Standalone |

Do not include VST2 or AAX, even as disabled installer choices. AU must be absent from Windows/Linux UI and payloads. CMake enforces the build-side platform selection.

## Current status

- macOS arm64: local development builds and audio/UI/host tests exist.
- macOS `.pkg`: local arm64 preview implemented in `Tools/package_macos.py`; selectable AU/VST3/Standalone, current-user or all-users installation, bundled license notices. Package payloads and native Installer choice evaluation checked. The project owner reported testing both macOS and Windows installers; comprehensive upgrade/uninstall and compatibility validation remains pending.
- Windows installer: NSIS preview implemented, using x64 VST3/Standalone binaries cross-compiled with LLVM/Clang-CL and the Microsoft CRT/SDK. PE architecture, exports and system-only imports inspected. The project owner reported that the Windows installer works. Detailed audio/host, upgrade and uninstall checks remain pending.
- Linux: graphical installer approach discussed; implementation and platform testing pending.
- No automatic-update system exists. The previews are unsigned; macOS is not notarized. The first public distribution is labelled a pre-release.

Approved icon source SVGs are in `Assets/Icons/`; branded banner artwork is in `Assets/Installer/`; reusable backgrounds are in `Assets/Brand/`. Working mockups remain outside Git in `Working/Design/`. Preview art is not an executable installer.

## Preparing a release

1. Choose a new version in `project(BigAltedChico VERSION ...)`; update the UI version as needed. Keep plugin IDs, parameter IDs, bundle identifiers and compatible state formats stable.
2. Build and validate on each advertised OS/architecture. Do not label macOS binaries as Windows/Linux products. Run audio tests, latency/host checks, state/preset round-trips, UI and skin tests, and clean-install/upgrade/uninstall checks.
3. Build each platform's installer once its implementation exists. Include only the selected, supported components. Package the appropriate notices, dependency license texts and corresponding source for the AGPL combined distribution.
4. For public macOS releases, prepare appropriate signing/notarization. Do not present the local ad-hoc signature as a notarized release. Keep signing keys/passwords out of Git; provide them through the release environment. Prepare Windows signing separately if used.
5. Stage binaries, installers and complete matching source under `Working/dist/<version>/`. Preserve the exact JUCE version/source and the build/packaging scripts used. The automatic GitHub source ZIP alone does not contain external JUCE; ensure recipients have access to the complete corresponding source as required by the chosen distribution license.
6. Compute SHA-256 checksums, create a version tag and attach artifacts to that GitHub Release after validation. Document OS, architecture, minimum system version, known limitations and source availability. No publication or remote repository creation is performed by this guide.

Suggested names once products are actually ready:

- `BigAltedChico-<version>-macOS-<arch>.pkg`
- `BigAltedChico-<version>-Windows-x64-Setup.exe`
- `BigAltedChico-<version>-Linux-x86_64-Installer.run` (installer technology and extension remain to be confirmed)
- `BigAltedChico-<version>-corresponding-source.tar.gz`
- `SHA256SUMS.txt`

## Upgrade behavior

The macOS preview uses stable package IDs and versioned bundle replacement at fixed paths within the selected installation domain. It refuses replacement when it detects an active BigAltedChico process or loaded plugin. It does not migrate copies between installation scopes or remember previously selected components. Detection across restricted processes, clean install and upgrade behavior require manual testing. Future installers should remember component choices and replace matching program/plugin files. Preserve user presets, custom skins, preferences and DAW projects. Changing the selected component set must not silently delete user data. Retain plugin identity and test saved-session compatibility between releases.

Publishing a GitHub Release alone will not update users' computers. Initially users will download and run the new installer. An online updater is a separate feature and is not implemented.

First-release notes prepared in [releases/v0.1.0.md](releases/v0.1.0.md). Publish as a pre-release while compatibility testing is ongoing.

## Complete source archive

After committing the source for a release, run:

```sh
python3 Tools/package_source.py --juce-dir /path/to/the/exact/JUCE-used-for-the-build
```

Output is written to `Working/Instalers/Source/`. The archive contains tracked project files, the local JUCE sources (excluding bundled executable apps and local instruction files), rebuild instructions, a commit identifier and a SHA-256 inventory of dependency sources. It refuses a dirty project tree or a tracked Working directory. Use the same JUCE baseline as the binaries and validate the extracted source before publishing it.
