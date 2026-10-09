<p align="right">
  <a href="README.zh_CN.md">简体中文</a> · <strong>English</strong>
</p>

# Assets

This directory stores reusable fonts, images, music, and sound effects, organized by asset type.

Keep each asset in the matching subdirectory and document its destination, naming, integration method, and source/license. Do not mix binary assets with Markdown documentation.

## Fonts

Store reusable font files and generated font sources in `fonts/`.

- Use descriptive names that include the family, weight, size, and format when relevant.
- Document the source, license, character range, conversion command, and expected destination.
- Check Flash and internal-RAM impact before adding a font; the ESP32-C3 has no PSRAM.
- Do not commit fonts whose license does not permit redistribution.

## Images

Store reusable source images and generated display assets in `images/`.

| File | Dimensions and format | Use and source |
| --- | --- | --- |
| [`images/home.jpg`](images/home.jpg) | 3840 × 2160, JPEG | Product hero image embedded in both upstream documentation overview files to foreground AI Passport and its open, maker-oriented identity. |
| [`images/readme-hardware-specs.png`](images/readme-hardware-specs.png) | 2172 × 724, PNG RGBA | Optional technical infographic retained as a reference asset; it is no longer used as the homepage hero. Generated for this repository with the built-in image generation tool on 2026-09-17; the six labels and values were checked against the documented hardware contract. |
| [`images/logo-wordmark.png`](images/logo-wordmark.png) | 1648 × 336, PNG RGBA | Transparent black wordmark extracted from the repository's original `images/logo.png`; embedded in both upstream documentation overview files for light backgrounds. |
| [`images/logo-wordmark-dark.png`](images/logo-wordmark-dark.png) | 1648 × 336, PNG RGBA | White version of the extracted wordmark, used by the upstream overview `<picture>` element when GitHub is in dark mode. |

- Use descriptive names and document dimensions, pixel format, conversion steps, and destination.
- Prefer formats suitable for the 240 × 320 RGB565 display and account for Flash and internal RAM.
- Preserve editable sources where licensing permits, and record the source and license.
- Never commit device QR secrets, credentials, or personal data in images.

## Music and sound effects

Store reusable music and sound-effect sources in `music/`.

- Document the source, license, sample rate, bit depth, channels, conversion command, and destination.
- Prefer 16 kHz, 16-bit mono PCM when it matches the current BSP audio path.
- Check Flash and internal-RAM cost before embedding audio; stream or chunk long recordings.
- Do not commit media without redistribution permission.

## Passport v0 materials

- `images/passport/tulip-seed-v1.png`, `tulip-sprout-v1.png` (1100×1430 RGBA), and `tulip-bud-v1.png` (1100×1429 RGBA): generated with built-in ImageGen on 2026-09-30 using `tulip-v2.png` as style reference. Prompt summaries and hashes are in `tulip-growth-sources.json`. The illustration permission policy below applies. Conversion adds three 55×65 RGB565A8 canvases with shared bottom anchors and visible heights 14/28/50 px, adding 32,175 bytes of const Flash data. Source alpha is preserved; mechanical conversion crops using alpha >= 8 to ignore nearly invisible edge noise.

- `images/passport/world-day-master-v2.png`, `mochun-idle-reference-v3.png`, `tulip-v2.png`, and `watering-can-v2.png` (PNG sources; character and objects have alpha): generated with ImageGen on 2026-09-29. The matsujun character uses a maintainer-supplied illustration as identity reference. These illustrations, their target PNGs, and image arrays are published as part of this project at the maintainer's request, but are excluded from the code's MIT license. No separate permission for reuse or redistribution is granted here; contact the maintainer for asset permissions. Historical `mochun` / `momo` filenames and image identifiers are retained for compatibility.
- `tools/prepare_passport_images.py` converts these to 528×320 RGB565 background, 150×200 RGB565A8 character, 55×65 RGB565A8 tulip and 60×45 RGB565A8 watering can const arrays in `images/passport/passport_images.c`, compiled by main CMake. Target PNGs document resizing. Image data is 478,920 bytes.
- `fonts/passport/NotoSansSC.ttf`: [Google Fonts Noto Sans SC](https://github.com/google/fonts/tree/main/ofl/notosanssc), SIL Open Font License 1.1, included as `OFL.txt`. The source TTF is not linked. `tools/prepare_passport_font.ps1` records exact symbols plus ASCII 0x20–0x7E, weight 500 / 18 px / 4 bpp / uncompressed conversion with lv_font_conv 1.5.3. `passport_font_18.c` is compiled and explicitly selected by the view. Actual descriptors are checked by `tests/passport_render/render.c` including a missing-glyph negative control.

### Cottage life assets (2026-10-09)

`images/passport/world-night-master-v1.png` and `matsujun-sleep-v1.png` were generated with built-in ImageGen using the existing day panorama and idle character as edit references. The existing illustration permission policy applies. The night prompt preserves the continuous garden/cabin composition and changes it to moonlit outdoors and warm lamplight indoors; the sleep prompt preserves identity/clothing and requests a seated, closed-eye pose on genuine transparency. `tools/prepare_passport_images.py` converts these into `passport_world_night` (528×320 RGB565) and `passport_momo_sleep` (150×200 RGB565A8, 155 px visible height, bottom anchor). Total const image payload is now 906,840 bytes. Original day/idle sources are retained. Target PNGs and generated arrays use the same display format as the existing art.
