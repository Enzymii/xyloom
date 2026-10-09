#pragma once
#include "input.h"
#include "growth.h"

enum { WORLD_WIDTH = 528, VIEW_WIDTH = 240, VIEW_HEIGHT = 320,
       HOUSE_X = 288, TRANSITION_MS = 650, STANDBY_MS = 60000,
       WATERING_MS = 1800, WATER_TRAVEL_MS = 450 };
typedef enum { SCENE_HOUSE, SCENE_GARDEN } passport_scene_t;
typedef enum { HOME_IDLE, HOME_SLEEP, OUT } passport_momo_t;
typedef enum { TULIP_SEED, TULIP_SPROUT, TULIP_BUD, TULIP_BLOOM } passport_tulip_t;
typedef enum { DIALOG_NONE, DIALOG_WELCOME, DIALOG_STATUS, DIALOG_OUT,
               DIALOG_TULIP, DIALOG_CAN, DIALOG_WATER_DONE, DIALOG_WATER_FAILED,
               DIALOG_STORAGE_LOADING, DIALOG_STORAGE_ERROR, DIALOG_WATER_LIMIT,
               DIALOG_CLOCK_WAIT, DIALOG_FORGET_WIFI, DIALOG_SLEEP } passport_dialog_t;
typedef enum { STORAGE_LOADING, STORAGE_READY, STORAGE_ERROR } passport_storage_state_t;
typedef struct { int x, y, rotation; bool pouring; uint32_t pour_elapsed; } passport_water_pose_t;
typedef struct {
    passport_scene_t scene, target;
    passport_momo_t momo;
    passport_tulip_t tulip;
    uint8_t focus[2];
    bool night, sleeping, transitioning;
    uint32_t transition_started, last_activity;
    int battery_percent; /* -1 when unavailable */
    uint32_t uptime_minutes;
    int camera_x, camera_from, camera_to;
    passport_dialog_t dialog;
    passport_storage_state_t storage_state;
    passport_growth_t record, water_target;
    uint32_t day, watering_started, watering_elapsed;
    unsigned status_focus, network_state;
    bool wifi_connected, setup_requested, forget_requested;
    char setup_password[9];
    bool watering, water_save_pending, water_save_inflight, save_is_clock;
    uint32_t clock_tick_at, save_calendar_day;
    uint64_t clock_pending_ms, save_clock_ms;
} passport_world_t;

void passport_world_init(passport_world_t *world, uint32_t now);
void passport_world_handle(passport_world_t *world, passport_input_event_t event, uint32_t now);
void passport_world_clock_tick(passport_world_t *world, uint32_t now);
void passport_world_tick(passport_world_t *world, uint32_t now, bool key_down);
int passport_camera_at(int from, int to, uint32_t elapsed);
bool passport_world_busy(const passport_world_t *world);
void passport_world_storage_loaded(passport_world_t *world, bool success, const passport_growth_t *record);
bool passport_world_take_water_save(passport_world_t *world, passport_growth_t *record);
void passport_world_water_saved(passport_world_t *world, bool success, uint32_t now);
passport_water_pose_t passport_water_pose(uint32_t elapsed);
