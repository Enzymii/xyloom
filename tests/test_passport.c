#include "passport/input.h"
#include "passport/world.h"
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

static void world_tests(void) {
    passport_world_t w;
    passport_world_init(&w, 0);
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
    assert(w.dialog == DIALOG_CAN);
    passport_world_tick(&w, 720 + STANDBY_MS, false);
    assert(w.sleeping && w.dialog == DIALOG_NONE);
    passport_world_handle(&w, INPUT_WAKE, 720 + STANDBY_MS + 1);
    assert(!w.sleeping && w.scene == SCENE_GARDEN && w.dialog == DIALOG_NONE);
    passport_world_handle(&w, RIGHT_LONG, 62000);
    passport_world_tick(&w, 63000, false);
    w.momo = OUT;
    passport_world_handle(&w, OK_SHORT, 63010);
    assert(w.dialog == DIALOG_OUT);
    passport_world_handle(&w, OK_SHORT, 63011);
    passport_world_handle(&w, RIGHT_SHORT, 63012);
    passport_world_handle(&w, OK_SHORT, 63013);
    assert(w.dialog == DIALOG_STATUS);
    passport_world_handle(&w, LEFT_SHORT, 63014);
    assert(w.dialog == DIALOG_STATUS && w.focus[SCENE_HOUSE] == 1);
    passport_world_handle(&w, OK_SHORT, 63015);
    assert(w.dialog == DIALOG_NONE && w.scene == SCENE_HOUSE);
    passport_world_handle(&w, OK_SHORT, 63016);
    passport_world_tick(&w, 63016 + STANDBY_MS, false);
    assert(w.sleeping && w.dialog == DIALOG_NONE);
    passport_world_init(&w, UINT32_MAX - 10);
    assert(w.battery_percent == -1);
    passport_world_tick(&w, 10, false);
    assert(!w.sleeping);
}

int main(void) {
    input_tests(); world_tests();
    puts("Passport input/world tests: PASS");
    return 0;
}
