# Control and artwork reference

Main-interface logical canvas: **1200 × 390**. Rectangles are `[x, y, width, height]`, measured from its top-left. These IDs are case-sensitive. Omitted controls inherit the selected base.

| ID | Function | Ondas default | Salem default |
| --- | --- | --- | --- |
| `previous` | Previous sound preset | `[44, 6, 34, 34]` | `[34, 12, 32, 32]` |
| `next` | Next sound preset | `[83, 6, 34, 34]` | `[76, 12, 32, 32]` |
| `preset` | Current preset and preset menu | `[126, 6, 340, 34]` | `[122, 12, 350, 32]` |
| `save` | Save sound preset | `[474, 6, 34, 34]` | `[484, 12, 36, 32]` |
| `reset` | Reset sound to the initial preset | `[514, 6, 34, 34]` | `[530, 12, 36, 32]` |
| `processing` | LIVE processing toggle | `[932, 6, 88, 34]` | `[878, 12, 100, 32]` |
| `settings` | Open settings modal | `[1029, 6, 34, 34]` | `[992, 12, 40, 32]` |
| `bypass` | Latency-aligned bypass toggle | `[1070, 6, 88, 34]` | `[1045, 12, 120, 32]` |
| `pitch` | Pitch knob | `[168, 111, 152, 152]` | `[110, 119, 156, 156]` |
| `formant` | Formant knob | `[522, 111, 152, 152]` | `[484, 119, 156, 156]` |
| `drive` | Drive knob | `[845, 135, 112, 112]` | `[811, 142, 112, 112]` |
| `mix` | Dry/wet Mix knob | `[1025, 135, 112, 112]` | `[1009, 142, 112, 112]` |
| `pitchValue` | Editable Pitch readout | `[200, 273, 88, 28]` | `[139, 278, 98, 23]` |
| `formantValue` | Editable Formant readout | `[554, 273, 88, 28]` | `[513, 278, 98, 23]` |
| `link` | Bidirectional Pitch/Formant link toggle | `[395, 144, 58, 58]` | `[354, 170, 56, 42]` |
| `transpose` | Transpose tuning-mode button | `[464, 314, 112, 28]` | `[430, 326, 125, 34]` |
| `quantize` | Quantize tuning-mode button | `[584, 314, 112, 28]` | `[565, 326, 125, 34]` |
| `robot` | Robot tuning-mode button | `[704, 314, 112, 28]` | `[700, 326, 125, 34]` |
| `pluginLogo` | Main-interface BigAltedChico artwork slot | `[40, 342, 245, 32]` | `[43, 330, 235, 28]` |
| `brandLogo` | Main-interface Notficial LABS artwork slot | `[1050, 340, 115, 34]` | `[1041, 327, 114, 32]` |
| `skinName` | Skin-name caption | `[550, 365, 100, 16]` | `[550, 373, 100, 12]` |
| `modeLabel` | MODE caption | `[385, 314, 70, 28]` | `[344, 329, 62, 28]` |

The minimum accepted rectangle is 16 × 12; use larger interactive hit areas. Allow extra space for the labels/painting around knobs and LINK. Overlap is not checked automatically.

Artwork property names `pluginLogo` and `brandLogo` refer to filenames at the top level of the manifest and to rectangles inside `controls`: these have different purposes. For example:

```json
"pluginLogo": "plugin-logo.svg",
"controls": { "pluginLogo": [40, 342, 245, 32] }
```

The filename replaces the image; the rectangle moves/resizes its main-interface slot. Use this fragment inside a complete manifest. Main `controls.pluginLogo` does not move the modal header.

## Settings reference

Settings layout is fixed. The rectangles below are for planning background artwork, not keys accepted in the manifest's `controls` object. All positions use the full 1200 × 390 canvas origin. The modal background itself has a local 660 × 340 canvas.

| Region | Rectangle |
| --- | --- |
| `panel` | `[270, 25, 660, 340]` |
| `pluginLogo` | `[400, 33, 400, 30]` |
| `companyLink` | `[480, 81, 240, 20]` |
| `size75` | `[300, 143, 90, 31]` |
| `size100` | `[402, 143, 90, 31]` |
| `size125` | `[504, 143, 90, 31]` |
| `skinChoice` | `[350, 182, 244, 26]` |
| `developerLink` | `[300, 251, 305, 20]` |
| `licenseNotices` | `[655, 280, 240, 22]` |
| `closeSettings` | `[893, 31, 28, 28]` |
| `licenseText` | `[300, 143, 600, 146]` |
| `backToSettings` | `[300, 298, 70, 24]` |
| `watermark` | `[480, 115, 240, 233]` |

`licenseText` and `backToSettings` belong to the alternative license-reader view. Other listed buttons/links are on the normal settings view. They are not all visible simultaneously.

The large watermark is drawn at 10% opacity, the repeating logo pattern at 2%, with 10 logical units of logo clearance. Keep logo paths opaque in the input SVG.

See [README.md](README.md) for authoring, importing and sharing.
