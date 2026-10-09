#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "growth.h"

typedef struct {
    bool loaded, success, life_saved, life_ready;
    uint32_t life_stamp;
    passport_growth_t record;
} passport_storage_result_t;
/* Application-lifetime worker owns NVS. Main loop alone changes world state. */
bool passport_storage_start(void);
bool passport_storage_save(const passport_growth_t *record, uint32_t calendar_day, bool drink);
bool passport_storage_save_life(uint32_t stamp);
bool passport_storage_poll(passport_storage_result_t *result);
