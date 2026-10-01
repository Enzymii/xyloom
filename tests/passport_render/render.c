#include "lvgl.h"
#include "passport/view.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

LV_FONT_DECLARE(passport_font_18);
static unsigned s_flushes;
static uint16_t s_frame[240 * 320];
static uint16_t s_partial[240 * 20];
static void flush(lv_display_t *display, const lv_area_t *area, uint8_t *data) {
    ++s_flushes;
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
    const uint32_t cps[] = {0xb7, 0x3002, 0x4e0d, 0x4e2d, 0x4e3b, 0x4e86, 0x4eba, 0x4eca, 0x4fdd, 0x50a8, 0x5148, 0x51fa, 0x5206, 0x53d6, 0x53ef, 0x53f3, 0x540e, 0x542f, 0x5566, 0x5668, 0x56de, 0x5728, 0x58f6, 0x5907, 0x5931, 0x5b50, 0x5b58, 0x5b8c, 0x5bb6, 0x5bc6, 0x5c0f, 0x5c4b, 0x5de6, 0x5df2, 0x5e7c, 0x5f00, 0x5f55, 0x5f85, 0x6001, 0x60a0, 0x6210, 0x624b, 0x6253, 0x62e9, 0x6309, 0x63a5, 0x65e5, 0x65f6, 0x671f, 0x672a, 0x673a, 0x6765, 0x676f, 0x67e5, 0x6821, 0x68c0, 0x6a59, 0x6b63, 0x6c34, 0x6cab, 0x6d47, 0x6d4f, 0x6d88, 0x6e05, 0x6ee1, 0x70b9, 0x70ed, 0x72b6, 0x7528, 0x7535, 0x7740, 0x7761, 0x7801, 0x786e, 0x79cd, 0x7b49, 0x7d2f, 0x7eaf, 0x7f51, 0x7f6e, 0x8054, 0x8272, 0x82b1, 0x82d7, 0x82de, 0x884c, 0x89c8, 0x8ba1, 0x8ba4, 0x8bb0, 0x8bbe, 0x8bd5, 0x8bf7, 0x8bfb, 0x8d25, 0x8fd0, 0x8fd4, 0x8fde, 0x9009, 0x90c1, 0x914d, 0x91cd, 0x91cf, 0x91d1, 0x957f, 0x95e8, 0x95f2, 0x9664, 0x9999, 0xff1f, 0xff5e};
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
    passport_growth_t initial = {0};
    passport_world_storage_loaded(&world, true, &initial);
    world.day = 20261001;
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
    passport_world_handle(&world, OK_SHORT, 653);
    passport_world_tick(&world, 878, false);
    save(display, "watering-travel.ppm", &world);
    passport_world_tick(&world, 1403, false);
    save(display, "watering-pour.ppm", &world);
    passport_world_tick(&world, 2228, false);
    save(display, "watering-return.ppm", &world);
    passport_world_tick(&world, 2453, false);
    passport_growth_t count;
    assert(passport_world_take_water_save(&world, &count) && count.total == 1);
    save(display, "watering-saving.ppm", &world);
    passport_world_water_saved(&world, false, 2454);
    save(display, "watering-failed.ppm", &world);
    passport_world_handle(&world, OK_SHORT, 2455);
    assert(passport_world_take_water_save(&world, &count) && count.total == 1);
    passport_world_water_saved(&world, true, 2456);
    save(display, "watering-done.ppm", &world);
    passport_world_handle(&world, OK_SHORT, 2457);
    passport_world_handle(&world, LEFT_SHORT, 2458);
    passport_world_handle(&world, OK_SHORT, 2459);
    save(display, "watering-count.ppm", &world);
    world.record.total = UINT32_MAX;
    save(display, "watering-count-max.ppm", &world);
    world.storage_state = STORAGE_ERROR;
    save(display, "watering-unavailable.ppm", &world);
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
    world.storage_state = STORAGE_READY;
    world.record.total = 56; world.record.today = 4; world.record.daily = 4; world.record.day = world.day;
    world.scene = SCENE_GARDEN; world.camera_x = 0; world.dialog = DIALOG_NONE;
    const char *paths[] = {"growth-seed.ppm", "growth-sprout.ppm", "growth-bud.ppm", "growth-bloom.ppm"};
    const unsigned progress[] = {0, 4, 20, 56};
    for (unsigned i = 0; i < 4; ++i) { world.record.growth = progress[i]; save(display, paths[i], &world); }

    world.dialog = DIALOG_TULIP;
    const unsigned midway[] = {2, 12, 38};
    const char *percent_paths[] = {"percent-seed.ppm", "percent-sprout.ppm", "percent-bud.ppm"};
    for (unsigned i = 0; i < 3; ++i) {
        world.record.growth = midway[i];
        save(display, percent_paths[i], &world);
    }
    world.record.growth = 56;
    save(display, "growth-inspect.ppm", &world);
    world.record.total = world.record.today = UINT32_MAX;
    save(display, "growth-cups-max.ppm", &world);
    world.scene = SCENE_HOUSE; world.camera_x = HOUSE_X; world.dialog = DIALOG_STATUS;
    world.wifi_connected = true; save(display, "wifi-connected.ppm", &world);
    world.network_state = 3; snprintf(world.setup_password, sizeof(world.setup_password), "ABCD1234");
    save(display, "wifi-setup.ppm", &world);
    world.dialog = DIALOG_FORGET_WIFI; save(display, "wifi-forget.ppm", &world);
    world.scene = SCENE_GARDEN; world.camera_x = 0; world.day = 0; world.wifi_connected = false;
    world.dialog = DIALOG_CLOCK_WAIT; save(display, "clock-wait.ppm", &world);
    world.day = 20261001; world.record.day = world.day; world.record.total = 56;
    world.record.today = world.record.daily = 3; world.record.growth = 20;
    world.dialog = DIALOG_NONE; save(display, "daily-three.ppm", &world);
    /* Stationary screen must stop flushing, even as input timestamps change. */
    unsigned flushed = s_flushes;
    for (unsigned i = 0; i < 300; ++i) {
        world.last_activity += 20;
        world.uptime_minutes++;
        passport_view_render(&world);
        lv_tick_inc(20); lv_timer_handler();
    }
    assert(s_flushes == flushed);
    world.record.today++;
    passport_view_render(&world); lv_refr_now(display);
    assert(s_flushes > flushed);
    flushed = s_flushes;
    world.dialog = DIALOG_STATUS; world.scene = SCENE_HOUSE;
    passport_view_render(&world); lv_refr_now(display);
    assert(s_flushes > flushed);
    flushed = s_flushes;
    for (unsigned i = 0; i < 100; ++i) {
        passport_view_render(&world); lv_tick_inc(20); lv_timer_handler();
    }
    assert(s_flushes == flushed);
    world.uptime_minutes++;
    passport_view_render(&world); lv_refr_now(display);
    assert(s_flushes > flushed);
    puts("LVGL render, idle refresh and font coverage: PASS");
    return 0;
}
