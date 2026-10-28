# Make your first BigAltedChico skin

You need BigAltedChico and a plain-text editor. A vector editor is optional for custom artwork. No compiler is needed.

1. Copy `recolour-example.json` into a new folder and rename the copy `skin.json`.
2. Set your own unique `id`, for example `yourname-blue`, and a display `name`. Keep `schemaVersion` at `1` and `extends` at `salem` or `ondas`.
3. Change the two `#60C8F2` colours. Use six-digit hex colours: `#RRGGBB`.
4. Open BigAltedChico's gear, expand **Skin**, choose **Import skin...**, and select your `skin.json`.
5. To make more changes, choose **Open skins folder** and edit the files in the installed folder named after your skin ID. Keep that ID unchanged. Choose **Reload skins** to see the changes.

This first example contains no SVGs: it inherits Salem's layout and artwork. Once it works, use the full `skin.json` and its SVGs to design a complete interface.

For a full design, read [README.md](README.md), use [CONTROL_REFERENCE.md](CONTROL_REFERENCE.md), and open [control-map.svg](control-map.svg) and [modal-control-map.svg](modal-control-map.svg) in your vector editor as guides.

To share your work, copy your final installed `skin.json` and every SVG it references into one folder, add your credits/license, and ZIP it. Recipients extract the ZIP and import `skin.json`; the plugin does not import ZIP files directly.
