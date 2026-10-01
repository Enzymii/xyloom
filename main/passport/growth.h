#pragma once
#include <stdbool.h>
#include <stdint.h>

enum { GROWTH_DAILY_MAX = 4, GROWTH_BLOOM = 56 };
typedef struct {
    uint32_t total, today, day;
    uint8_t growth, daily;
} passport_growth_t;

/* A calendar day is YYYYMMDD, never elapsed time since boot. Zero is unknown. */
bool passport_day_valid(uint32_t day);
uint32_t passport_day_next(uint32_t day);
bool passport_growth_valid(const passport_growth_t *state);
bool passport_growth_drink(const passport_growth_t *state, uint32_t day,
                           passport_growth_t *next);
unsigned passport_growth_stage(uint8_t growth);

/* Whole percent within the current stage; a new stage starts at zero. */
unsigned passport_growth_stage_percent(uint8_t growth);
