#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "growth.h"

typedef struct { bool loaded, success; passport_growth_t record; } passport_storage_result_t;
/* Application-lifetime worker owns NVS. Main loop alone changes world state. */
bool passport_storage_start(void);
bool passport_storage_save(const passport_growth_t *record);
bool passport_storage_poll(passport_storage_result_t *result);
