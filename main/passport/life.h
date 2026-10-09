#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

enum { LIFE_RECORD_SIZE = 16 };
typedef struct { bool night, asleep, away; } passport_life_state_t;
/* UTC seconds, independent of Wi-Fi and the watering calendar. Zero is unknown. */
bool passport_life_time_valid(uint32_t stamp);
passport_life_state_t passport_life_at(uint32_t stamp);
unsigned passport_life_walk_start(uint32_t local_day);
void passport_life_encode(uint8_t bytes[LIFE_RECORD_SIZE], uint32_t stamp);
bool passport_life_decode(const uint8_t *bytes, size_t size, uint32_t *stamp);
