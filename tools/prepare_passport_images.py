#!/usr/bin/env python3
"""Deterministic target-size conversion of approved art to LVGL 9 constants."""
from pathlib import Path
from PIL import Image, ImageOps

ROOT = Path(__file__).resolve().parents[1]
ASSETS = ROOT / "assets/images/passport"


def descriptor(name, source, size, alpha, stage_height=None):
    image = Image.open(ASSETS / source).convert("RGBA" if alpha else "RGB")
    if alpha:
        # Keep new sprite proportions and anchor its actual feet/base to bottom.
        bounds = image.getchannel("A").point(lambda a: 255 if a >= 8 else 0).getbbox() if stage_height else image.getchannel("A").getbbox()
        if bounds is None:
            raise ValueError(f"Empty sprite: {source}")
        # Earlier growth stages share the bloom's canvas and ground anchor.
        # Their visible height is calibrated; never enlarge a seed to flower size.
        fit = (size[0], stage_height) if stage_height else size
        sprite = ImageOps.contain(image.crop(bounds), fit, Image.Resampling.LANCZOS)
        image = Image.new("RGBA", size, (0, 0, 0, 0))
        image.paste(sprite, ((size[0] - sprite.width) // 2, size[1] - sprite.height))
    else:
        image = image.resize(size, Image.Resampling.LANCZOS)
    image.save(ASSETS / f"{name}-target.png")
    rgb = bytearray()
    for r, g, b in image.convert("RGB").getdata():
        pixel = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
        rgb.extend(pixel.to_bytes(2, "little"))
    # LVGL RGB565A8 stores the RGB565 plane then the 8-bit alpha plane.
    if alpha:
        rgb.extend(image.getchannel("A").tobytes())
    rows = [", ".join(f"0x{b:02x}" for b in rgb[i:i+24]) for i in range(0, len(rgb), 24)]
    body = ",\n    ".join(rows)
    color = "RGB565A8" if alpha else "RGB565"
    return f'''_Alignas(4) static const uint8_t {name}_data[] = {{
    {body}
}};
const lv_image_dsc_t {name} = {{
    .header = {{.magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_{color},
               .w = {size[0]}, .h = {size[1]}, .stride = {size[0]*2}}},
    .data_size = sizeof({name}_data), .data = {name}_data,
}};
'''


if __name__ == "__main__":
    content = '#include "lvgl.h"\n'
    content += descriptor("passport_world_day", "world-day-master-v2.png", (528, 320), False)
    content += descriptor("passport_momo_idle", "mochun-idle-reference-v3.png", (150, 200), True)
    content += descriptor("passport_tulip", "tulip-v2.png", (55, 65), True)
    content += descriptor("passport_tulip_seed", "tulip-seed-v1.png", (55, 65), True, 14)
    content += descriptor("passport_tulip_sprout", "tulip-sprout-v1.png", (55, 65), True, 28)
    content += descriptor("passport_tulip_bud", "tulip-bud-v1.png", (55, 65), True, 50)
    content += descriptor("passport_can", "watering-can-v2.png", (60, 45), True)
    (ASSETS / "passport_images.c").write_text(content, encoding="utf-8")
    print("Image payload: 478920 bytes in const Flash data, no full framebuffer")
