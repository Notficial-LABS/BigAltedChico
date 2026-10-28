# Local test installers

Generated installers live under **`Working/Instalers/`** (this spelling matches the local project convention):

- `MacOS/`: native `.pkg`, Apple Silicon, macOS 12 or newer.
- `Windows/`: NSIS `.exe`, x64 payloads, Windows 10/11.
- `Linux/`: reserved; no Linux installer has been built yet.

These are unsigned development previews. They are not notarized/public releases. Close the DAW and Standalone before installing. Reopen the newly generated installer after a rebuild; an already-open Installer window retains its old resources.

## macOS

```sh
python3 Tools/package_macos.py
```

Choose **all users** or **only me** in Destination Select, then AU, VST3 and/or Standalone in Installation Type.

| Component | All users (administrator authorization) | Only me |
| --- | --- | --- |
| Standalone | `/Applications/BigAltedChico.app` | `~/Applications/BigAltedChico.app` |
| AU | `/Library/Audio/Plug-Ins/Components/BigAltedChico.component` | `~/Library/Audio/Plug-Ins/Components/BigAltedChico.component` |
| VST3 | `/Library/Audio/Plug-Ins/VST3/BigAltedChico.vst3` | `~/Library/Audio/Plug-Ins/VST3/BigAltedChico.vst3` |

The all-users Standalone appears in Finder's usual Applications sidebar location. The earlier preview enabled only the home-directory domain; the current package enables both native Installer domains. Scope changes do not automatically remove a copy previously installed in the other scope. User presets/preferences remain in their existing user locations.

The welcome screen uses `Packaging/macOS/welcome.rtfd`, a native rich-text document containing the banner at **394 × 60.16 points**. This replaces HTML percentage sizing, which displayed an oversized/cropped image in Installer. Regenerate it with `swift Tools/render_macos_welcome.swift`; the tool checks the serialized attachment dimensions and writes a native text-layout preview to `Working/Previews/`.

The package uses `com.apple.rtfd` and includes the complete document bundle. Native AppKit layout and package resource integrity have been checked. Final light/dark-mode appearance in Installer still needs checking on the actual window.

## Windows

Build the actual Windows binaries first (see [BUILDING](BUILDING.md)), then:

```sh
python3 Tools/package_windows.py --build-dir Working/build-windows-msvc
```

NSIS (`makensis`) must be available. The script rejects missing or non-AMD64 PE payloads. The installer contains only VST3 and Standalone; **AU, VST2 and AAX are absent**.

It requests administrator permission for the standard all-users destinations:

- VST3: `C:\Program Files\Common Files\VST3\BigAltedChico.vst3`
- Standalone: `C:\Program Files\Notficial LABS\BigAltedChico\BigAltedChico.exe`
- Start Menu: `Notficial LABS > BigAltedChico` (when Standalone is selected).

Uninstall through Windows Settings or the installed `Uninstall.exe`. Uninstallation targets program files and bundled notices, not AppData, presets, skins or DAW projects. A later install replaces selected files at the same locations; deselecting a component does not uninstall a previously installed copy. Component-choice persistence is not implemented yet.

### PC validation checklist

1. Run the installer and verify the branding and component list.
2. Try Standalone only, then VST3 only; verify the respective files and shortcut.
3. Open Standalone, select an audio device, raise Mix from the Clean default and test audio.
4. Rescan VST3 in Ableton, open a new instance, test all three skins, LINK and tuning modes.
5. Check Quality/Live latency (expected 7200/1440 samples at 48 kHz), playback and saved-session reopening.
6. Reinstall over the preview, then uninstall with audio applications closed; verify user presets and skins remain.

Compilation and binary inspection on macOS do not replace these Windows runtime checks. Report the exact Windows/DAW version and any installer or scan error.

## Icons

Approved artwork remains in `Assets/Icons/BAC-mac.svg` and `BAC-windows.svg`. `Tools/render_app_icons.py` exports PNG/ICO/ICNS resources using Pillow and resvg-py; these generated icon assets are included in source control so ordinary builds do not require those rendering dependencies.

- Windows Setup and Uninstall embed the BAC icon through NSIS MUI_ICON/MUI_UNICON. The Standalone embeds it through `Packaging/Windows/StandaloneIcon.rc`; its Start Menu shortcut uses the executable icon. Windows Apps/Uninstall uses the custom uninstaller icon as DisplayIcon.
- The macOS app embeds AppIcon.icns through JUCE and references it from CFBundleIconFile. The icon is inside the signed app bundle.
- The local macOS `.pkg` also receives a custom Finder icon. Such file icons live in filesystem metadata, so the packager additionally writes a ZIP with resource-fork metadata preserved. Share the ZIP when the package's Finder icon must survive transfer; the app's embedded icon does not depend on that ZIP.

The Windows installer was reported working by the project owner on 2026-10-27. The subsequent icon update was checked by extracting the new installer and verifying the approved 256px image in Setup, Uninstall and Standalone. This does not imply new Windows audio/host tests. The macOS app icon/signature and a ZIP extraction preserving package icon metadata were checked locally.
