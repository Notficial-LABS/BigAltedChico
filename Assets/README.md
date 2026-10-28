# Assets

Original project artwork is covered by the root MIT license. Third-party fonts retain their own license. SVG source assets are editable, self-contained vectors.

- `NotficialLabs-Logo.svg`: isolated white gear, transparent background and centre, for the modal watermark/pattern.
- `NotficialLabs-Wordmark.svg`: white horizontal brand wordmark with its original frame.
- `NotficialLabs-Trademark.svg`: alternative stacked brand mark.
- `NotficialLabs.png`: transparent white raster brand mark used by the original interface.
- `Skins/Ondas/OndasBackground.svg`, `Skins/Salem/SalemBackground.svg`: decorative vector backgrounds; functional controls are drawn separately by JUCE.
- `Brand/BigAltedChico-Wordmark.svg`: product logo used as the README heading, with clearance for the Big descender.
- `Brand/BlackWaves.svg`: reusable full-bleed vector texture without logo or corner mask.
- `Brand/BlackWaves-Banner.svg`: wide adaptation of the same texture.
- `Icons/BAC-mac.svg`, `Icons/BAC-windows.svg`: approved standalone icon artwork, half gear + tightly spaced BAC and the full wordmark below. PNG/ICO/ICNS exports are integrated into the Standalone apps and installer icons. Regenerate with `Tools/render_app_icons.py`.
- `Installer/Banner.svg`: approved black-wave banner with both wordmarks. Native macOS and Windows installers use exported branding resources from this folder.
- `ThirdPartyNotices.txt`: embedded project/dependency notices shown by the offline license reader.

The brand marks were provided by Notficial LABS and vectorised from the owner's original PNGs; raster cleanup was used for the legacy image. Source images, design iterations and validation drafts remain local under ignored `Working/`.

`Roboto.ttf` is the variable Roboto font from https://github.com/google/fonts/blob/main/ofl/roboto/Roboto%5Bwdth,wght%5D.ttf (retrieved 2026-10-25), under SIL OFL 1.1. The full notice is in `Roboto-OFL.txt`. No system font installation is required by the plugin.

The Skin Kit ZIP is generated from `SkinTemplate/` during CMake configuration. It is no longer tracked under Assets; the source template is the editable asset.
