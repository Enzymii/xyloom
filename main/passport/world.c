#include "world.h"
#include <string.h>

void passport_world_init(passport_world_t *w, uint32_t now) {
    memset(w, 0, sizeof(*w));
    w->scene = w->target = SCENE_HOUSE;
    w->momo = HOME_IDLE;
    w->battery_percent = -1;
    w->camera_x = HOUSE_X;
    w->last_activity = now;
}

int passport_camera_at(int from, int to, uint32_t elapsed) {
    if (elapsed >= TRANSITION_MS) return to;
    /* Fixed point smoothstep, bounded intermediates even on ESP32-C3. */
    int32_t t = (int32_t)(elapsed * 1024 / TRANSITION_MS);
    int32_t ease = (t * t * (3072 - 2 * t)) / (1024 * 1024);
    return from + (to - from) * ease / 1024;
}

void passport_world_handle(passport_world_t *w, passport_input_event_t event, uint32_t now) {
    if (event == INPUT_NONE) return;
    w->last_activity = now;
    if (w->sleeping) {
        w->sleeping = false;
        return;
    }
    if (event == INPUT_WAKE || w->transitioning) return;
    if ((event == LEFT_LONG && w->scene == SCENE_HOUSE) ||
        (event == RIGHT_LONG && w->scene == SCENE_GARDEN)) {
        w->target = w->scene == SCENE_HOUSE ? SCENE_GARDEN : SCENE_HOUSE;
        w->camera_from = w->camera_x;
        w->camera_to = w->target == SCENE_HOUSE ? HOUSE_X : 0;
        w->transition_started = now;
        w->transitioning = true;
        w->dialog = DIALOG_NONE;
        return;
    }
    if (w->dialog != DIALOG_NONE) {
        if (event == OK_SHORT) w->dialog = DIALOG_NONE;
        return;
    }
    if (event == LEFT_SHORT || event == RIGHT_SHORT) {
        w->focus[w->scene] ^= 1;
    } else if (event == OK_SHORT) {
        if (w->scene == SCENE_HOUSE) {
            w->dialog = w->focus[SCENE_HOUSE] ? DIALOG_STATUS :
                w->momo == OUT ? DIALOG_OUT : DIALOG_WELCOME;
        } else {
            w->dialog = w->focus[SCENE_GARDEN] ? DIALOG_CAN : DIALOG_TULIP;
        }
    }
}

void passport_world_tick(passport_world_t *w, uint32_t now, bool key_down) {
    if (key_down) w->last_activity = now;
    if (w->transitioning) {
        uint32_t elapsed = now - w->transition_started;
        w->camera_x = passport_camera_at(w->camera_from, w->camera_to, elapsed);
        if (elapsed >= TRANSITION_MS) {
            w->scene = w->target;
            w->transitioning = false;
            w->last_activity = now;
        }
    } else if (!w->sleeping && (uint32_t)(now - w->last_activity) >= STANDBY_MS) {
        w->sleeping = true;
        w->dialog = DIALOG_NONE;
    }
}
