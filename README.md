<h1 align="center">
  <img src="Assets/Brand/BigAltedChico-Wordmark.svg" alt="BigAltedChico" width="700">
</h1>

**Juguete**

![BigAltedChico — Juguete skin](docs/images/juguete.png)

**Ondas**

![BigAltedChico — Ondas skin](docs/images/ondas.png)

**Salem**

![BigAltedChico — Salem skin](docs/images/salem.png)

An open-source monophonic voice transformer by **Notficial LABS**, developed by **[Leonardo Marcelo](https://github.com/byLeonardoMarcelo)**. Written in C++17 with JUCE and Signalsmith Stretch.

Pitch and formant shifting, linked controls, chromatic quantization, robot tuning, drive, and three selectable skins: **Juguete**, **Ondas**, and **Salem**.

**[Download releases](https://github.com/Notficial-LABS/BigAltedChico/releases)** · [Installation guide](docs/INSTALLERS.md) · [Create a skin](SkinTemplate/QUICKSTART.md) · [Contribute](CONTRIBUTING.md) · [Report a bug](https://github.com/Notficial-LABS/BigAltedChico/issues)

## Status and supported platforms

Current preview: **0.1** (bundle metadata `0.1.0`). The maintainer has tried both macOS and Windows installers. macOS audio/UI and VST3 test-host checks have passed locally; broader Windows audio/host and hardware compatibility testing is ongoing. The first public preview is available from [Releases](https://github.com/Notficial-LABS/BigAltedChico/releases).

| Platform | Architecture | Formats | Availability |
| --- | --- | --- | --- |
| macOS 12 or newer | Apple Silicon (arm64) | AU, VST3, Standalone | v0.1.0 preview |
| Windows 10/11 | x64 | VST3, Standalone | v0.1.0 preview |
| Linux | Not yet validated | VST3, Standalone planned | No installer available |

Intel Mac builds are not currently supplied. VST2 and AAX are excluded. AU is only available on macOS.

## Download and install

You do not need Git, JUCE or a compiler to use a prebuilt installer.

1. Open **[GitHub Releases](https://github.com/Notficial-LABS/BigAltedChico/releases)** and choose a release compatible with your OS and architecture. Preview builds may be marked **Pre-release**.
2. Expand its **Assets** list and download the installer for your platform:

   | Platform | Asset to choose |
   | --- | --- |
   | macOS Apple Silicon | `BigAltedChico-<version>-macOS-arm64-*.zip` containing the `.pkg`, or the `.pkg` if supplied directly |
   | Windows x64 | `BigAltedChico-<version>-Windows-x64-*-Setup.exe` |

3. Close your DAW and BigAltedChico, then run the installer and select the supported formats you want.
4. Reopen your DAW and rescan plugins, or launch the Standalone. New instances start with Clean and Mix at minimum: raise Mix to hear the effect.

The GitHub-generated **Source code (zip)** / **Source code (tar.gz)** downloads and **Code → Download ZIP** contain source files. For a ready-to-install build, choose the platform-specific asset listed above. If a release has no installer assets, that release does not yet offer a prebuilt download for your platform.

### macOS

Extract the downloaded ZIP and open its `.pkg`. In **Destination Select**, choose:

- **All users:** Standalone in `/Applications/BigAltedChico.app`, visible in Finder's usual Applications sidebar; plugins in `/Library/Audio/Plug-Ins/Components` (AU) and `/Library/Audio/Plug-Ins/VST3`.
- **Only me:** Standalone in `~/Applications/BigAltedChico.app`; plugins in the corresponding `~/Library/Audio/Plug-Ins/` folders.

The all-users option requires administrator authorization. Changing scope does not remove an earlier copy from the other scope. The ZIP preserves the installer's custom Finder icon.

### Windows

Run `Setup.exe` and select **VST3** and/or **Standalone**. Installation uses the all-users locations and requests administrator permission:

- Standalone: `C:\Program Files\Notficial LABS\BigAltedChico\BigAltedChico.exe`, with a Start Menu shortcut.
- VST3: `C:\Program Files\Common Files\VST3\BigAltedChico.vst3`.

Uninstall through Windows Settings or the installed `Uninstall.exe`. User presets and custom skins are kept separately from program files.

The current previews are unsigned; macOS is not notarized. See each release's notes for its tested platforms and limitations. Updates are installed by downloading and running a newer installer; there is no automatic updater. See [installation details](docs/INSTALLERS.md).

Installers are attached to versioned releases. The repository contains source, editable assets, tools and documentation. For maintainers, see [release preparation](docs/RELEASES.md).

## Audio and controls

- **Pitch / Formant:** −12 to +12 semitones; double-click resets to 0.
- **LINK:** moving either knob moves both.
- **Transpose / Quantize / Robot:** free pitch shift, nearest chromatic note, or a fixed target note.
- **Drive / Mix:** saturation and dry/wet blend.
- **LIVE on:** 30 ms processing latency. **LIVE off:** Quality, 150 ms.
- **Settings:** UI size 75% / 100% / 125%, skin selection, custom skin import, credits and license notices.

New instances start with **00 Clean Start**: Pitch/Formant 0, Drive/Mix minimum. Raise Mix to hear the effect. The Standalone resets sound to Clean on launch while retaining the selected skin and audio device settings. DAW projects restore their saved parameters. Switching skins preserves sound.

The measured latency matches the reported delay in offline and VST3-host impulse tests. It excludes audio-interface and host buffering; **neither mode has zero latency**. LIVE trades spectral detail and low-note accuracy for speed. See [measurements and limitations](docs/LATENCY.md). Direct Ableton UI confirmation was obtained for Quality; direct LIVE confirmation remains pending.

## Build from source

Requirements: Git, CMake 3.24+, a C++17 compiler, Python 3, and JUCE (tested with **9.0.4**). Signalsmith and the necessary artwork/font files are included; JUCE is obtained separately.

Clone the official repository to explore or build the source:

```sh
git clone https://github.com/Notficial-LABS/BigAltedChico.git
cd BigAltedChico
```

Install the dependencies for your OS as described in [BUILDING](docs/BUILDING.md), then configure and build. Replace `/path/to/JUCE` with your JUCE source directory:

```sh
cmake -S . -B Working/build -DJUCE_DIR=/path/to/JUCE -DCMAKE_BUILD_TYPE=Release
cmake --build Working/build --config Release --parallel
```

Products are generated under `Working/build/BigAltedChico_artefacts/Release/`. The Skin Kit ZIP is generated from `SkinTemplate/` during CMake configuration; no downloaded binary asset is needed.

See [platform build instructions](docs/BUILDING.md) for macOS, Windows, Linux and tests. On macOS, `./build.sh build` builds without installing; `./build.sh install` explicitly installs the VST3. `./build.sh preview` opens the development Standalone, and `python3 watch_preview.py` watches for source changes.

## Repository layout

| Path | Contents |
| --- | --- |
| `Source/` | Audio processor, DSP, editor, skins, settings and Standalone application |
| `ThirdParty/` | Vendored Signalsmith headers, versions and original MIT notices |
| `Assets/` | Runtime font/artwork, reusable brand assets, approved icon and banner designs |
| `SkinTemplate/` | Editable JSON/SVG skin kit, including the Salem starting point |
| `Tools/` | Audio/UI checks, VST3 host checks, render analysis, skin and macOS packaging |
| `Packaging/` | Native installer resources and scripts |
| `docs/` | Build/release guides, usage, curated screenshots and measurement tables |
| `Licenses/` | JUCE license overview and AGPL text for the combined open-source distribution |
| `Working/` | **Ignored:** builds, tests, drafts, backups and installers in `Instalers/MacOS`, `Instalers/Windows`, `Instalers/Linux` |

`Working/` is local-only and recreated as needed. Do not force-add its contents to Git. A clean checkout does not need anything from that directory to build.

## Contributing and custom skins

Create your own skins with JSON and SVG—no C++ or plugin rebuild required. Start with the [Skin Quickstart](SkinTemplate/QUICKSTART.md), then use the [complete creator guide](SkinTemplate/README.md) and [control/branding reference](SkinTemplate/CONTROL_REFERENCE.md). The kit includes working examples and editable maps for the main UI and settings modal. See [skin usage](docs/SKINS.md) for selecting and installing packs.

Code, skins, documentation, translations, test results and bug reports are welcome.

To propose a change:

1. Open the [project repository](https://github.com/Notficial-LABS/BigAltedChico) and click **Fork** to create your own copy.
2. Clone your fork, add the original repository as `upstream`, and create a branch. Replace `YOUR-USERNAME` with your GitHub username:

   ```sh
   git clone https://github.com/YOUR-USERNAME/BigAltedChico.git
   cd BigAltedChico
   git remote add upstream https://github.com/Notficial-LABS/BigAltedChico.git
   git switch -c feat/my-change
   ```

3. Make your change and check it on the platforms you can test. Keep local builds, renders and installers in ignored `Working/`.
4. Commit and push your branch to your fork, then open a **Pull request** targeting `Notficial-LABS/BigAltedChico` → `main`. Explain the change and what you tested.

See [CONTRIBUTING.md](CONTRIBUTING.md) for commit/push examples, updating your fork and the validation expected for each type of change. Documentation and skin contributions do not require compiling C++.

For a bug, open an [Issue](https://github.com/Notficial-LABS/BigAltedChico/issues) with your OS/architecture, DAW/version or Standalone, sample rate, buffer size, steps to reproduce, expected result and actual result.

## Licensing

**Original BigAltedChico code, documentation and project artwork are MIT-licensed**: see [LICENSE](LICENSE). You may modify, redistribute, sell, or include that code in other projects, subject to retaining the MIT notice. Third-party files retain their own licenses.

**MIT does not replace JUCE's license.** JUCE offers AGPLv3 and commercial licensing. Our planned open-source binary releases use JUCE under AGPLv3, so the combined application must be distributed under the applicable AGPL terms with corresponding source. A closed-source distributor needs a suitable JUCE commercial license; the MIT license on our own code does not grant one. See [licensing details](docs/LICENSING.md) and [JUCE's license](Licenses/JUCE-LICENSE.md).

Signalsmith Stretch/Linear are MIT; Roboto is SIL OFL 1.1. Notices are retained in the source tree and the plugin's offline license reader.
