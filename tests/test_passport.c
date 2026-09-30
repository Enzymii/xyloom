#include "passport/input.h"
#include "passport/world.h"
#include "passport/watering_record.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>

static passport_input_event_t sample(passport_input_t *s, passport_key_t key, uint32_t t) {
    return passport_input_sample(s, key, t, false, false);
}
static void input_tests(void) {
    passport_input_t s;
    passport_input_init(&s);
    assert(sample(&s, KEY_LEFT, 0) == INPUT_NONE);
    assert(sample(&s, KEY_NONE, 10) == INPUT_NONE); /* bounce */
    assert(sample(&s, KEY_LEFT, 20) == INPUT_NONE);
    assert(sample(&s, KEY_LEFT, 50) == INPUT_NONE);
    assert(sample(&s, KEY_NONE, 120) == INPUT_NONE);
    assert(sample(&s, KEY_NONE, 150) == LEFT_SHORT);
    assert(sample(&s, KEY_NONE, 160) == INPUT_NONE);
    sample(&s, KEY_RIGHT, 200); sample(&s, KEY_RIGHT, 230);
    assert(sample(&s, KEY_RIGHT, 929) == INPUT_NONE);
    assert(sample(&s, KEY_RIGHT, 930) == RIGHT_LONG);
    assert(sample(&s, KEY_RIGHT, 2000) == INPUT_NONE);
    sample(&s, KEY_NONE, 2010);
    assert(sample(&s, KEY_NONE, 2040) == INPUT_NONE);
    sample(&s, KEY_OK, 2100); sample(&s, KEY_OK, 2130);
    assert(sample(&s, KEY_OK, 3000) == INPUT_NONE);
    sample(&s, KEY_NONE, 3010);
    assert(sample(&s, KEY_NONE, 3040) == INPUT_NONE); /* OK long undefined */

    passport_input_init(&s);
    sample(&s, KEY_LEFT, 0); sample(&s, KEY_LEFT, 30);
    assert(sample(&s, KEY_NONE, 729) == INPUT_NONE);
    assert(sample(&s, KEY_NONE, 759) == LEFT_SHORT); /* release debounce must not turn 699ms into hold */

    passport_input_init(&s);
    passport_input_sample(&s, KEY_OK, 0, true, false);
    assert(passport_input_sample(&s, KEY_OK, 30, true, false) == INPUT_WAKE);
    sample(&s, KEY_NONE, 100);
    assert(sample(&s, KEY_NONE, 130) == INPUT_NONE);
    sample(&s, KEY_OK, 200); sample(&s, KEY_OK, 230);
    sample(&s, KEY_NONE, 300);
    assert(sample(&s, KEY_NONE, 330) == OK_SHORT);

    passport_input_init(&s);
    passport_input_sample(&s, KEY_LEFT, 0, false, true);
    passport_input_sample(&s, KEY_LEFT, 30, false, true);
    assert(sample(&s, KEY_LEFT, 1000) == INPUT_NONE);
    sample(&s, KEY_NONE, 1010);
    assert(sample(&s, KEY_NONE, 1040) == INPUT_NONE);
    sample(&s, KEY_OK, 1100); sample(&s, KEY_OK, 1130);
    assert(sample(&s, KEY_INVALID, 1150) == INPUT_NONE);
    sample(&s, KEY_NONE, 1200);
    assert(sample(&s, KEY_NONE, 1230) == INPUT_NONE);

    passport_input_init(&s);
    sample(&s, KEY_LEFT, UINT32_MAX - 100);
    sample(&s, KEY_LEFT, UINT32_MAX - 70);
    assert(sample(&s, KEY_LEFT, 629) == LEFT_LONG); /* clock wrap */
}

static void restore(passport_world_t *w, bool ok, uint32_t total) {
    passport_growth_t record = {.total = total};
    passport_world_storage_loaded(w, ok, &record);
    w->day = 20261001;
}

static void world_tests(void) {
    passport_world_t w;
    passport_world_init(&w, 0);
    restore(&w, true, 0);
    assert(w.scene == SCENE_HOUSE && w.camera_x == HOUSE_X);
    passport_world_handle(&w, OK_SHORT, 10);
    assert(w.dialog == DIALOG_WELCOME);
    passport_world_handle(&w, LEFT_LONG, 20);
    assert(w.transitioning && w.dialog == DIALOG_NONE);
    passport_world_handle(&w, OK_SHORT, 30);
    assert(w.dialog == DIALOG_NONE);
    int previous = HOUSE_X;
    for (unsigned t = 0; t <= TRANSITION_MS; ++t) {
        int x = passport_camera_at(HOUSE_X, 0, t);
        assert(x >= 0 && x <= previous);
        previous = x;
    }
    assert(passport_camera_at(0, HOUSE_X, TRANSITION_MS) == HOUSE_X);
    passport_world_tick(&w, 20 + TRANSITION_MS, false);
    assert(w.scene == SCENE_GARDEN && w.camera_x == 0 && !w.transitioning);
    passport_world_handle(&w, LEFT_LONG, 700);
    assert(!w.transitioning);
    passport_world_handle(&w, RIGHT_SHORT, 710);
    assert(w.focus[SCENE_GARDEN] == 1 && w.focus[SCENE_HOUSE] == 0);
    passport_world_handle(&w, OK_SHORT, 720);
    assert(w.watering && w.record.total == 0);
    passport_world_tick(&w, 720 + WATERING_MS, false);
    passport_growth_t saved_count;
    assert(passport_world_take_water_save(&w, &saved_count) && saved_count.total == 1);
    passport_world_water_saved(&w, true, 720 + WATERING_MS);
    assert(w.dialog == DIALOG_WATER_DONE && w.record.total == 1);
    passport_world_tick(&w, 720 + WATERING_MS + STANDBY_MS, false);
    assert(w.sleeping && w.dialog == DIALOG_NONE);
    passport_world_handle(&w, INPUT_WAKE, 720 + WATERING_MS + STANDBY_MS + 1);
    assert(!w.sleeping && w.scene == SCENE_GARDEN && w.dialog == DIALOG_NONE);
    passport_world_handle(&w, RIGHT_LONG, 64000);
    passport_world_tick(&w, 65000, false);
    w.momo = OUT;
    passport_world_handle(&w, OK_SHORT, 65010);
    assert(w.dialog == DIALOG_OUT);
    passport_world_handle(&w, OK_SHORT, 65011);
    passport_world_handle(&w, RIGHT_SHORT, 65012);
    passport_world_handle(&w, OK_SHORT, 65013);
    assert(w.dialog == DIALOG_STATUS);
    passport_world_handle(&w, LEFT_SHORT, 65014);
    assert(w.dialog == DIALOG_STATUS && w.focus[SCENE_HOUSE] == 1);
    passport_world_handle(&w, RIGHT_SHORT, 65015);
    passport_world_handle(&w, OK_SHORT, 65015);
    assert(w.dialog == DIALOG_NONE && w.scene == SCENE_HOUSE);
    passport_world_handle(&w, OK_SHORT, 65016);
    passport_world_tick(&w, 65016 + STANDBY_MS, false);
    assert(w.sleeping && w.dialog == DIALOG_NONE);
    passport_world_init(&w, UINT32_MAX - 10);
    assert(w.battery_percent == -1);
    passport_world_tick(&w, 10, false);
    assert(!w.sleeping);
}

static void watering_tests(void) {
    passport_world_t w;
    passport_world_init(&w, 0);
    w.scene = w.target = SCENE_GARDEN; w.camera_x = 0;
    passport_world_handle(&w, OK_SHORT, 1);
    assert(w.dialog == DIALOG_TULIP && !w.watering);
    passport_world_handle(&w, OK_SHORT, 2);
    passport_world_handle(&w, RIGHT_SHORT, 3);
    passport_world_handle(&w, OK_SHORT, 4);
    assert(w.dialog == DIALOG_STORAGE_LOADING && !w.watering);
    restore(&w, false, 0);
    passport_world_handle(&w, OK_SHORT, 5);
    assert(w.dialog == DIALOG_STORAGE_ERROR && !w.watering);
    passport_world_handle(&w, OK_SHORT, 6);
    restore(&w, true, 7);
    w.momo = OUT;
    uint32_t started = UINT32_MAX - 500;
    passport_world_handle(&w, OK_SHORT, started);
    assert(w.watering && w.record.total == 7 && passport_world_busy(&w));
    passport_world_handle(&w, OK_SHORT, started + 1);
    passport_world_handle(&w, RIGHT_LONG, started + 2);
    passport_world_handle(&w, LEFT_SHORT, started + 3);
    assert(w.watering && !w.transitioning && w.focus[SCENE_GARDEN] == 1);
    passport_world_tick(&w, started + WATERING_MS - 1, false);
    assert(w.watering && w.record.total == 7);
    passport_world_tick(&w, started + WATERING_MS, false);
    assert(!w.watering && w.water_save_pending && w.record.total == 7);
    passport_growth_t count;
    assert(passport_world_take_water_save(&w, &count) && count.total == 8);
    assert(!passport_world_take_water_save(&w, &count));
    passport_world_tick(&w, started + WATERING_MS + STANDBY_MS, false);
    assert(!w.sleeping && w.record.total == 7);
    passport_world_water_saved(&w, false, 65000);
    assert(w.dialog == DIALOG_WATER_FAILED && w.record.total == 7);
    passport_world_handle(&w, OK_SHORT, 65001);
    assert(passport_world_take_water_save(&w, &count) && count.total == 8);
    passport_world_water_saved(&w, true, 65002);
    assert(w.record.total == 8 && w.dialog == DIALOG_WATER_DONE);
    passport_world_water_saved(&w, true, 65003);
    assert(w.record.total == 8); /* Duplicate result must not credit twice. */
    passport_world_handle(&w, OK_SHORT, 65004);
    assert(w.dialog == DIALOG_NONE && !w.watering);
    passport_world_handle(&w, OK_SHORT, 65005);
    assert(w.watering); /* Only a fresh press starts another watering. */

    passport_world_init(&w, 0);
    restore(&w, true, UINT32_MAX);
    w.scene = SCENE_GARDEN; w.focus[SCENE_GARDEN] = 1;
    passport_world_handle(&w, OK_SHORT, 1);
    assert(w.dialog == DIALOG_WATER_LIMIT && !w.watering);

    passport_water_pose_t p = passport_water_pose(0);
    assert(p.x == 140 && p.y == 225 && !p.pouring && p.rotation == 0);
    p = passport_water_pose(WATER_TRAVEL_MS);
    assert(p.x == 75 && p.y == 175 && p.pouring && p.rotation == 250);
    p = passport_water_pose(WATERING_MS - WATER_TRAVEL_MS);
    assert(p.x == 75 && p.y == 175 && !p.pouring);
    p = passport_water_pose(WATERING_MS);
    assert(p.x == 140 && p.y == 225 && !p.pouring && p.rotation == 0);
    for (uint32_t t = 0; t <= WATERING_MS; ++t) {
        p = passport_water_pose(t);
        assert(p.x >= 75 && p.x <= 140 && p.y >= 175 && p.y <= 225);
    }
    passport_world_init(&w, 0); restore(&w, true, 0);
    w.scene = SCENE_GARDEN; w.focus[SCENE_GARDEN] = 1; w.day = 0;
    passport_world_handle(&w, OK_SHORT, 1);
    assert(w.dialog == DIALOG_CLOCK_WAIT && !w.watering && w.record.total == 0);
    passport_world_handle(&w, OK_SHORT, 2); w.day = 20261001;
    passport_world_handle(&w, OK_SHORT, 3); w.day = 20261002;
    passport_world_tick(&w, 3 + WATERING_MS, false);
    assert(passport_world_take_water_save(&w, &count) && count.day == 20261001);
    passport_world_water_saved(&w, false, 2000);
    passport_world_handle(&w, OK_SHORT, 2001);
    assert(passport_world_take_water_save(&w, &count) && count.day == 20261001 && count.growth == 1);
    passport_world_water_saved(&w, true, 2002);
    assert(w.record.total == 1 && w.record.day == 20261001);
    passport_world_handle(&w, OK_SHORT, 2003);
    passport_world_handle(&w, OK_SHORT, 2004);
    assert(w.water_target.day == 20261002 && w.water_target.today == 1 && w.water_target.growth == 2);

    passport_world_init(&w, 0); w.dialog = DIALOG_STATUS;
    passport_world_handle(&w, RIGHT_SHORT, 1); passport_world_handle(&w, OK_SHORT, 2);
    assert(w.setup_requested && w.dialog == DIALOG_STATUS);
    passport_world_handle(&w, RIGHT_SHORT, 3); passport_world_handle(&w, OK_SHORT, 4);
    assert(w.dialog == DIALOG_FORGET_WIFI && !w.forget_requested);
    passport_world_handle(&w, LEFT_SHORT, 5);
    assert(w.dialog == DIALOG_STATUS && !w.forget_requested);
    passport_world_handle(&w, OK_SHORT, 6); passport_world_handle(&w, OK_SHORT, 7);
    assert(w.forget_requested && w.dialog == DIALOG_STATUS);
}

static void record_tests(void) {
    uint8_t record[WATERING_RECORD_SIZE];
    const uint32_t values[] = {0, 1, 255, 0x12345678, UINT32_MAX};
    for (unsigned i = 0; i < sizeof(values) / sizeof(values[0]); ++i) {
        uint32_t decoded = 99;
        passport_watering_encode(record, values[i]);
        assert(memcmp(record, "XYWC\1\0\0\0", 8) == 0);
        assert(passport_watering_decode(record, sizeof(record), &decoded) && decoded == values[i]);
    }
    uint32_t unchanged = 42;
    assert(!passport_watering_decode(record, sizeof(record) - 1, &unchanged) && unchanged == 42);
    record[0] = 'Z';
    assert(!passport_watering_decode(record, sizeof(record), &unchanged));
    passport_watering_encode(record, 10); record[4] = 2;
    assert(!passport_watering_decode(record, sizeof(record), &unchanged));
    record[4] = 1; record[7] = 1;
    assert(!passport_watering_decode(record, sizeof(record), &unchanged));
}

int main(void) {
    input_tests(); world_tests(); watering_tests(); record_tests();
    puts("Passport input/world tests: PASS");
    return 0;
}
