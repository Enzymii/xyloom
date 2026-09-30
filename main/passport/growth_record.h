#pragma once
#include "growth.h"
#include <stddef.h>
enum { GROWTH_RECORD_SIZE = 24 };
void passport_growth_encode(uint8_t bytes[GROWTH_RECORD_SIZE], const passport_growth_t *state);
bool passport_growth_decode(const uint8_t *bytes, size_t size, passport_growth_t *state);
bool passport_growth_equal(const passport_growth_t *a, const passport_growth_t *b);
