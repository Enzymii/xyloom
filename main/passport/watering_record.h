#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

enum { WATERING_RECORD_SIZE = 12 };
void passport_watering_encode(uint8_t record[WATERING_RECORD_SIZE], uint32_t count);
bool passport_watering_decode(const uint8_t *record, size_t size, uint32_t *count);
