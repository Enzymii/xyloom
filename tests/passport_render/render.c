#include "lvgl.h"
#include "passport/view.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

LV_FONT_DECLARE(passport_font_18);
static uint16_t s_frame[240 * 320];
static uint16_t s_partial[240 * 20];
static void flush(lv_display_t *display, const lv_area_t *area, uint8_t *data) {
    uint16_t *pixels = (uint16_t *)data;
    for (int y = area->y1; y <= area->y2; ++y)
        for (int x = area->x1; x <= area->x2; ++x)
            s_frame[y * 240 + x] = *pixels++;
    lv_display_flush_ready(display);
}
static void save(lv_display_t *display, const char *path, passport_world_t *world) {
    passport_view_render(world);
    lv_refr_now(display);
    FILE *file = fopen(path, "wb");
    assert(file);
    fprintf(file, "P6\n240 320\n255\n");
    for (unsigned i = 0; i < 240 * 320; ++i) {
        uint16_t p = s_frame[i];
        uint8_t rgb[] = {(uint8_t)(((p >> 11) & 31) * 255 / 31),
            (uint8_t)(((p >> 5) & 63) * 255 / 63), (uint8_t)((p & 31) * 255 / 31)};
        fwrite(rgb, 1, 3, file);
    }
    fclose(file);
}
int main(void) {
    lv_init();
    /* Full UI text inventory, including punctuation, checked against the
     * actual generated glyph descriptors; a negative control must fail. */
    const uint32_t cps[] = {0x6cab,0x7eaf,0x4e3b,0x4eba,0x56de,0x6765,0x5566,
        0xff5e,0x623f,0x95f4,0x90c1,0x91d1,0x9999,0x6c34,0x58f6,0x5c4b,
        0x91cc,0x9759,0x6084,0x7684,0x3002,0x51fa,0x95e8,0x4e86,0x6a59,
        0x8272,0x559d,0x5c06,0x5728,0x540e,0x7eed,0x9636,0x6bb5,0x5f00,0x653e,0x4e0e,0x5206,0x56de,0x5728,0x5907,0x5bb6,0x5c0f,0x6001,0x60a0,0x6309,0x65f6,0x72b6,0x7535,0x7740,0x7761,0x884c,0x8bbe,0x8fd0,0x8fd4,0x91cf,0x95f2};
    for (unsigned i=0; i<sizeof(cps)/sizeof(cps[0]); ++i) {
        lv_font_glyph_dsc_t glyph = {0};
        if (!lv_font_get_glyph_dsc(&passport_font_18, &glyph, cps[i], 0) || glyph.is_placeholder) {
            fprintf(stderr, "Missing glyph U+%04X\n", (unsigned)cps[i]);
            return 1;
        }
    }
    lv_font_glyph_dsc_t missing = {0};
    bool found = lv_font_get_glyph_dsc(&passport_font_18, &missing, 0x9f98, 0);
    assert(!found || missing.is_placeholder);
    lv_display_t *display = lv_display_create(240, 320);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(display, s_partial, NULL, sizeof(s_partial), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, flush);
    passport_view_create();
    passport_world_t world;
    passport_world_init(&world, 0);
    save(display, "house.ppm", &world);
    passport_world_handle(&world, OK_SHORT, 0);
    save(display, "dialog.ppm", &world);
    passport_world_handle(&world, LEFT_LONG, 1);
    passport_world_tick(&world, 326, false);
    save(display, "door.ppm", &world);
    passport_world_tick(&world, 651, false);
    save(display, "garden.ppm", &world);
    passport_world_handle(&world, RIGHT_SHORT, 652);
    save(display, "garden-can.ppm", &world);
    passport_world_handle(&world, RIGHT_LONG, 653);
    passport_world_tick(&world, 1303, false);
    passport_world_handle(&world, RIGHT_SHORT, 1304);
    save(display, "house-room.ppm", &world);
    world.battery_percent = 83;
    world.uptime_minutes = 125;
    passport_world_handle(&world, OK_SHORT, 1305);
    save(display, "status.ppm", &world);
    world.battery_percent = -1;
    world.momo = OUT;
    save(display, "status-unavailable.ppm", &world);
    world.momo = HOME_SLEEP;
    save(display, "status-sleep.ppm", &world);
    puts("LVGL render and font coverage: PASS");
    return 0;
}
