#pragma once
#include "world.h"
/* Call only while holding the BSP LVGL lock. The view lives for app lifetime. */
void passport_view_create(void);
void passport_view_render(const passport_world_t *world);
