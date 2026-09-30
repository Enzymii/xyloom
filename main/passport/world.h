#pragma once
#include "input.h"

enum { WORLD_WIDTH = 528, VIEW_WIDTH = 240, VIEW_HEIGHT = 320,
       HOUSE_X = 288, TRANSITION_MS = 650, STANDBY_MS = 60000 };
typedef enum { SCENE_HOUSE, SCENE_GARDEN } passport_scene_t;
typedef enum { HOME_IDLE, HOME_SLEEP, OUT } passport_momo_t;
typedef enum { TULIP_SEED, TULIP_SPROUT, TULIP_BUD, TULIP_BLOOM } passport_tulip_t;
typedef enum { DIALOG_NONE, DIALOG_WELCOME, DIALOG_STATUS, DIALOG_OUT,
               DIALOG_TULIP, DIALOG_CAN } passport_dialog_t;
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
} passport_world_t;

void passport_world_init(passport_world_t *world, uint32_t now);
void passport_world_handle(passport_world_t *world, passport_input_event_t event, uint32_t now);
void passport_world_tick(passport_world_t *world, uint32_t now, bool key_down);
int passport_camera_at(int from, int to, uint32_t elapsed);
