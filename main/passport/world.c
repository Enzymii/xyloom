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

bool passport_world_busy(const passport_world_t *w) {
    return w->transitioning || w->watering || w->water_save_pending || w->water_save_inflight;
}

passport_water_pose_t passport_water_pose(uint32_t elapsed) {
    passport_water_pose_t pose = {140, 225, 0, false, 0};
    if (elapsed >= WATERING_MS) return pose;
    uint32_t travel;
    if (elapsed < WATER_TRAVEL_MS) travel = elapsed;
    else if (elapsed < WATERING_MS - WATER_TRAVEL_MS) {
        travel = WATER_TRAVEL_MS;
        pose.pouring = true;
        pose.pour_elapsed = elapsed - WATER_TRAVEL_MS;
    } else travel = WATERING_MS - elapsed;
    pose.x -= (int)(65 * travel / WATER_TRAVEL_MS);
    pose.y -= (int)(50 * travel / WATER_TRAVEL_MS);
    pose.rotation = (int)(250 * travel / WATER_TRAVEL_MS);
    return pose;
}

void passport_world_storage_loaded(passport_world_t *w, bool success, const passport_growth_t *record) {
    w->storage_state = success ? STORAGE_READY : STORAGE_ERROR;
    if (success) { w->record = *record; w->tulip = (passport_tulip_t)passport_growth_stage(record->growth); }
    if (w->dialog == DIALOG_STORAGE_LOADING) w->dialog = DIALOG_NONE;
}

bool passport_world_take_water_save(passport_world_t *w, passport_growth_t *record) {
    if (!w->water_save_pending || w->water_save_inflight) return false;
    *record = w->water_target;
    w->water_save_pending = false;
    w->water_save_inflight = true;
    return true;
}

void passport_world_water_saved(passport_world_t *w, bool success, uint32_t now) {
    if (!w->water_save_inflight) return;
    w->water_save_inflight = false;
    w->last_activity = now;
    if (success) {
        w->record = w->water_target;
        w->tulip = (passport_tulip_t)passport_growth_stage(w->record.growth);
        w->dialog = DIALOG_WATER_DONE;
    } else w->dialog = DIALOG_WATER_FAILED;
}

void passport_world_handle(passport_world_t *w, passport_input_event_t event, uint32_t now) {
    if (event == INPUT_NONE) return;
    w->last_activity = now;
    if (w->sleeping) {
        w->sleeping = false;
        return;
    }
    if (event == INPUT_WAKE || passport_world_busy(w)) return;
    if (w->dialog == DIALOG_WATER_FAILED && event == OK_SHORT) {
        w->dialog = DIALOG_NONE;
        w->water_save_pending = true;
        return;
    }
    if (w->dialog == DIALOG_FORGET_WIFI) {
        if (event == OK_SHORT) { w->forget_requested = true; w->dialog = DIALOG_STATUS; }
        else if (event == LEFT_SHORT || event == RIGHT_SHORT) w->dialog = DIALOG_STATUS;
        return;
    }
    if (w->dialog == DIALOG_STATUS) {
        if (event == LEFT_SHORT) { w->status_focus = (w->status_focus + 2) % 3; return; }
        if (event == RIGHT_SHORT) { w->status_focus = (w->status_focus + 1) % 3; return; }
        if (event == OK_SHORT && w->status_focus == 1) { w->setup_requested = true; return; }
        if (event == OK_SHORT && w->status_focus == 2) { w->dialog = DIALOG_FORGET_WIFI; return; }
    }
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
            w->status_focus = 0;
        } else {
            if (!w->focus[SCENE_GARDEN]) w->dialog = DIALOG_TULIP;
            else if (w->storage_state == STORAGE_LOADING) w->dialog = DIALOG_STORAGE_LOADING;
            else if (w->storage_state == STORAGE_ERROR) w->dialog = DIALOG_STORAGE_ERROR;
            else if (w->record.total == UINT32_MAX) w->dialog = DIALOG_WATER_LIMIT;
            else if (!passport_growth_drink(&w->record, w->day, &w->water_target)) w->dialog = DIALOG_CLOCK_WAIT;
            else {
                w->watering = true;
                w->watering_started = now;
                w->watering_elapsed = 0;
                w->dialog = DIALOG_NONE;
            }
        }
    }
}

void passport_world_tick(passport_world_t *w, uint32_t now, bool key_down) {
    if (key_down) w->last_activity = now;
    if (w->watering) {
        uint32_t elapsed = now - w->watering_started;
        w->watering_elapsed = elapsed < WATERING_MS ? elapsed : WATERING_MS;
        if (elapsed >= WATERING_MS) {
            w->watering = false;
            w->water_save_pending = true;
        }
        w->last_activity = now;
    } else if (w->water_save_pending || w->water_save_inflight) {
        w->last_activity = now;
    } else if (w->transitioning) {
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
