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


/* Releasing during an animation must not swallow the first fresh gesture
 * after it ends, even if there is no idle sample between the two. */
static void transition_release_tests(void) {
    passport_input_t s;
    passport_input_init(&s);
    passport_input_sample(&s, KEY_OK, 0, false, true);
    passport_input_sample(&s, KEY_OK, 30, false, true);
    passport_input_sample(&s, KEY_NONE, 100, false, true);
    passport_input_sample(&s, KEY_NONE, 130, false, true);
    assert(passport_input_sample(&s, KEY_LEFT, 200, false, false) == INPUT_NONE);
    assert(passport_input_sample(&s, KEY_LEFT, 230, false, false) == INPUT_NONE);
    assert(passport_input_sample(&s, KEY_LEFT, 930, false, false) == LEFT_LONG);
    assert(passport_input_sample(&s, KEY_NONE, 940, false, false) == INPUT_NONE);
    assert(passport_input_sample(&s, KEY_NONE, 970, false, false) == INPUT_NONE);
    const passport_key_t keys[] = {KEY_LEFT, KEY_RIGHT, KEY_OK};
    const passport_input_event_t taps[] = {LEFT_SHORT, RIGHT_SHORT, OK_SHORT};
    for (unsigned i = 0; i < 3; ++i) {
        passport_input_init(&s);
        passport_input_sample(&s, KEY_NONE, 0, false, true);
        passport_input_sample(&s, KEY_NONE, 100, false, true);
        passport_input_sample(&s, keys[i], 200, false, false);
        passport_input_sample(&s, keys[i], 230, false, false);
        passport_input_sample(&s, KEY_NONE, 300, false, false);
        assert(passport_input_sample(&s, KEY_NONE, 330, false, false) == taps[i]);
    }
    /* A release shorter than debounce does not re-arm an interrupted hold. */
    passport_input_init(&s);
    passport_input_sample(&s, KEY_RIGHT, 0, false, true);
    passport_input_sample(&s, KEY_RIGHT, 30, false, true);
    passport_input_sample(&s, KEY_NONE, 100, false, true);
    assert(passport_input_sample(&s, KEY_RIGHT, 120, false, false) == INPUT_NONE);
    assert(passport_input_sample(&s, KEY_RIGHT, 1000, false, false) == INPUT_NONE);
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
    assert(w.watering && w.record.total == 0 && w.water_target.clock_mode == CLOCK_LOCAL);
    w.day = 20261002; /* Sync during a transaction must not change its frozen bucket. */
    passport_world_tick(&w, 1 + WATERING_MS, false);
    assert(passport_world_take_water_save(&w, &count) && count.day == 0);
    passport_world_water_saved(&w, false, 2000);
    passport_world_handle(&w, OK_SHORT, 2001);
    assert(passport_world_take_water_save(&w, &count) && count.day == 0 && count.growth == 1);
    passport_world_water_saved(&w, true, 2002);
    assert(w.record.total == 1 && w.record.day == 0);
    passport_world_handle(&w, OK_SHORT, 2003);
    passport_world_handle(&w, OK_SHORT, 2004);
    assert(w.water_target.day == 20261002 && w.water_target.today == 2 && w.water_target.growth == 2 && w.water_target.clock_mode == CLOCK_BRIDGED);

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


typedef struct {
    passport_input_t input;
    passport_world_t world;
    uint32_t now;
} journey_t;

/* Use the application's 10 ms sampling order rather than injecting semantic
 * button events; persistence acknowledgements stay explicit below. */
static void journey_frames(journey_t *j, passport_key_t key, unsigned duration) {
    for (unsigned elapsed = 0; elapsed < duration; elapsed += 10) {
        j->now += 10;
        passport_world_clock_tick(&j->world, j->now);
        if (j->world.save_is_clock && j->world.water_save_pending) {
            passport_growth_t clock;
            assert(passport_world_take_water_save(&j->world, &clock));
            passport_world_water_saved(&j->world, true, j->now);
        }
        passport_input_event_t event = passport_input_sample(&j->input, key, j->now,
            j->world.sleeping, passport_world_busy(&j->world));
        passport_world_handle(&j->world, event, j->now);
        passport_world_tick(&j->world, j->now, key != KEY_NONE && key != KEY_INVALID);
    }
}
static void journey_tap(journey_t *j, passport_key_t key) {
    journey_frames(j, key, 100);
    journey_frames(j, KEY_NONE, 50);
}
static void journey_tests(void) {
    journey_t j = {0};
    passport_input_init(&j.input);
    passport_world_init(&j.world, 0);
    restore(&j.world, true, 0);
    journey_tap(&j, KEY_OK);
    assert(j.world.dialog == DIALOG_WELCOME);
    journey_tap(&j, KEY_OK);
    assert(j.world.dialog == DIALOG_NONE);
    journey_frames(&j, KEY_LEFT, 800);
    assert(j.world.transitioning && j.world.camera_x < HOUSE_X && j.world.camera_x > 0);
    journey_frames(&j, KEY_NONE, 700);
    assert(j.world.scene == SCENE_GARDEN && j.world.camera_x == 0);
    journey_tap(&j, KEY_RIGHT);
    assert(j.world.focus[SCENE_GARDEN] == 1);
    j.world.day = 0;
    journey_tap(&j, KEY_OK);
    assert(j.world.watering && j.world.record.total == 0);
    journey_frames(&j, KEY_RIGHT, 1900); /* Held during watering: no replay. */
    passport_growth_t record;
    assert(passport_world_take_water_save(&j.world, &record) && record.total == 1);
    passport_world_water_saved(&j.world, false, j.now);
    assert(j.world.dialog == DIALOG_WATER_FAILED && j.world.record.total == 0);
    journey_frames(&j, KEY_NONE, 50);
    journey_tap(&j, KEY_OK);
    assert(passport_world_take_water_save(&j.world, &record) && record.total == 1);
    passport_world_water_saved(&j.world, true, j.now);
    assert(j.world.dialog == DIALOG_WATER_DONE && j.world.record.total == 1);
    journey_tap(&j, KEY_OK);
    journey_frames(&j, KEY_NONE, 100);
    assert(!j.world.transitioning && j.world.scene == SCENE_GARDEN);
    journey_frames(&j, KEY_RIGHT, 800);
    journey_frames(&j, KEY_NONE, 700);
    assert(j.world.scene == SCENE_HOUSE && j.world.camera_x == HOUSE_X);
    assert(j.world.focus[SCENE_GARDEN] == 1 && j.world.focus[SCENE_HOUSE] == 0);
    journey_frames(&j, KEY_NONE, STANDBY_MS);
    assert(j.world.sleeping);
    journey_frames(&j, KEY_LEFT, 1600); /* Wake hold must not also leave home. */
    journey_frames(&j, KEY_NONE, 50);
    assert(!j.world.sleeping && !j.world.transitioning && j.world.scene == SCENE_HOUSE);
    journey_tap(&j, KEY_OK);
    assert(j.world.dialog == DIALOG_WELCOME && j.world.record.total == 1);
    journey_tap(&j, KEY_OK);
    journey_frames(&j, KEY_LEFT, 800);
    journey_frames(&j, KEY_NONE, 700);
    journey_frames(&j, KEY_NONE, STANDBY_MS);
    assert(j.world.sleeping && j.world.scene == SCENE_GARDEN);
    journey_tap(&j, KEY_OK); /* Wake OK must not water the selected can. */
    assert(!j.world.sleeping && !j.world.watering && j.world.dialog == DIALOG_NONE);
    assert(j.world.record.total == 1);
    journey_tap(&j, KEY_OK);
    assert(j.world.watering && j.world.water_target.total == 2);
}

static void restart_date_tests(void) {
    passport_world_t w;
    passport_world_init(&w, 0);
    passport_growth_t record = {.total = 15, .today = 3, .day = 20261008,
        .growth = 12, .daily = 3, .clock_mode = CLOCK_CALENDAR};
    passport_world_storage_loaded(&w, true, &record);
    passport_world_clock_tick(&w, 1000);
    assert(passport_world_take_water_save(&w, &record) && w.save_is_clock);
    assert(record.clock_mode == CLOCK_AWAITING && record.daily == 3);
    passport_world_water_saved(&w, true, 1000);
    w.day = 20261009;
    passport_world_clock_tick(&w, 1010);
    assert(passport_world_take_water_save(&w, &record) && record.daily == 0);
    assert(record.total == 15 && record.growth == 12 && w.record.daily == 3);
    passport_world_water_saved(&w, false, 1011);
    assert(w.record.daily == 3 && w.dialog == DIALOG_WATER_FAILED);
    passport_world_handle(&w, OK_SHORT, 1012);
    assert(passport_world_take_water_save(&w, &record) && record.daily == 0);
    passport_world_water_saved(&w, true, 1013);
    assert(!w.record.daily && !w.record.today && w.dialog == DIALOG_NONE);
    assert(w.record.growth == 12 && w.record.total == 15);
}

static void offline_clock_tests(void) {
    passport_world_t w;
    passport_world_init(&w, 0);
    passport_growth_t record = {.total = 4, .today = 4, .daily = 4, .growth = 4,
        .phase_seconds = 86340, .runtime_seconds = 86340};
    passport_world_storage_loaded(&w, true, &record);
    passport_world_clock_tick(&w, 59999);
    assert(!w.water_save_pending && w.record.daily == 4);
    passport_world_clock_tick(&w, 60000);
    assert(w.water_save_pending && w.save_is_clock);
    assert(passport_world_take_water_save(&w, &record));
    assert(record.daily == 0 && record.today == 0 && record.growth == 4);
    passport_world_clock_tick(&w, 61000);
    passport_world_water_saved(&w, false, 61000);
    passport_world_t asleep = w;
    passport_world_tick(&asleep, 61000 + STANDBY_MS, false);
    assert(asleep.sleeping && asleep.dialog == DIALOG_WATER_FAILED);
    passport_world_handle(&asleep, INPUT_WAKE, 61001 + STANDBY_MS);
    assert(!asleep.sleeping && asleep.dialog == DIALOG_WATER_FAILED);
    passport_world_handle(&asleep, OK_SHORT, 61002 + STANDBY_MS);
    assert(passport_world_take_water_save(&asleep, &record) && record.runtime_seconds == 86400);
    passport_world_handle(&w, LEFT_LONG, 61001);
    assert(w.dialog == DIALOG_WATER_FAILED && !w.transitioning);
    w.day = 20261008;
    passport_world_clock_tick(&w, 62000);
    passport_world_handle(&w, OK_SHORT, 62001);
    assert(passport_world_take_water_save(&w, &record) && !record.day && record.runtime_seconds == 86400);
    passport_world_water_saved(&w, true, 62002);
    assert(w.clock_pending_ms == 2000 && w.record.daily == 0 && w.dialog == DIALOG_NONE);
    passport_world_clock_tick(&w, 62300); /* Pending time remains after the frozen acknowledgement. */
    assert(passport_world_take_water_save(&w, &record));
    assert(record.runtime_seconds == 86402 && record.clock_mode == CLOCK_BRIDGED);
    passport_world_water_saved(&w, true, 62300);
    assert(w.clock_pending_ms == 300 && w.last_activity == 62001);
    passport_world_tick(&w, 122001, false);
    assert(w.sleeping); /* Background clock saves cannot keep the backlight awake. */

    record.clock_mode = CLOCK_LOCAL;
    passport_world_init(&w, UINT32_MAX - 499);
    passport_world_storage_loaded(&w, true, &record);
    passport_world_clock_tick(&w, 500);
    assert(w.clock_pending_ms == 1000); /* Monotonic tick wrap, not wall-clock subtraction. */
    w.day = 0; w.scene = SCENE_GARDEN; w.focus[SCENE_GARDEN] = 1;
    passport_world_handle(&w, OK_SHORT, 501);
    assert(w.watering && w.water_target.total == 5 && w.save_clock_ms == 1000);
    passport_world_clock_tick(&w, 2301);
    passport_world_tick(&w, 2301, false);
    assert(passport_world_take_water_save(&w, &record) && record.runtime_seconds == 86403);
    passport_world_water_saved(&w, true, 2302);
    assert(w.clock_pending_ms == 1801 && w.record.growth == 5 && w.record.daily == 1);
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
    input_tests(); transition_release_tests(); world_tests(); watering_tests(); journey_tests(); restart_date_tests(); offline_clock_tests(); record_tests();
    passport_world_t w; passport_world_init(&w, 0);
    w.momo = HOME_SLEEP; passport_world_handle(&w, OK_SHORT, 1);
    assert(w.dialog == DIALOG_SLEEP); passport_world_handle(&w, OK_SHORT, 2);
    assert(w.dialog == DIALOG_NONE && w.momo == HOME_SLEEP);
    passport_growth_t record = {0}; passport_world_storage_loaded(&w, true, &record);
    w.momo = OUT; passport_world_handle(&w, OK_SHORT, 3); assert(w.dialog == DIALOG_OUT);
    passport_world_handle(&w, LEFT_LONG, 4); passport_world_tick(&w, 654, false);
    passport_world_handle(&w, RIGHT_SHORT, 655); passport_world_handle(&w, OK_SHORT, 656);
    assert(w.watering && w.momo == OUT); passport_world_tick(&w, 2456, false);
    assert(passport_world_take_water_save(&w, &record) && record.total == 1);
    passport_world_water_saved(&w, true, 2457);
    assert(w.record.growth == 1 && w.record.total == 1 && w.momo == OUT);
    puts("Passport input/world tests: PASS");
    return 0;
}
