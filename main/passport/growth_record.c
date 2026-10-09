#include "growth_record.h"
#include "watering_record.h"
#include <string.h>

static void put(uint8_t *b, uint32_t v) {
    for (unsigned i = 0; i < 4; ++i) b[i] = (uint8_t)(v >> (8 * i));
}
static uint32_t get(const uint8_t *b) {
    uint32_t v = 0;
    for (unsigned i = 0; i < 4; ++i) v |= (uint32_t)b[i] << (8 * i);
    return v;
}
void passport_growth_encode(uint8_t b[GROWTH_RECORD_SIZE], const passport_growth_t *s) {
    memset(b, 0, GROWTH_RECORD_SIZE);
    memcpy(b, "XYWC", 4); b[4] = 3; b[5] = s->growth; b[6] = s->daily; b[7] = s->clock_mode;
    put(b + 8, s->total); put(b + 12, s->today); put(b + 16, s->day);
    put(b + 20, (uint32_t)s->runtime_seconds); put(b + 24, (uint32_t)(s->runtime_seconds >> 32));
    put(b + 28, s->phase_seconds);
}
bool passport_growth_decode(const uint8_t *b, size_t size, passport_growth_t *s) {
    passport_growth_t value = {0};
    if (size == WATERING_RECORD_SIZE) {
        if (!passport_watering_decode(b, size, &value.total)) return false;
        /* Legacy records have no date evidence: preserve totals, start a new plant. */
    } else {
        if (size < 24 || memcmp(b, "XYWC", 4)) return false;
        bool legacy = size == 24 && b[4] == 2;
        if (legacy) {
            if (b[7] || b[20] || b[21] || b[22] || b[23]) return false;
        } else {
            if (size != GROWTH_RECORD_SIZE || b[4] != 3) return false;
            for (unsigned i = 32; i < GROWTH_RECORD_SIZE; ++i) if (b[i]) return false;
        }
        value.total = get(b + 8); value.today = get(b + 12); value.day = get(b + 16);
        value.growth = b[5]; value.daily = b[6];
        value.clock_mode = legacy ? (value.day ? CLOCK_CALENDAR : CLOCK_LOCAL) : b[7];
        if (legacy && !value.day && (value.today || value.daily)) return false;
        if (!legacy) {
            value.runtime_seconds = get(b + 20) | ((uint64_t)get(b + 24) << 32);
            value.phase_seconds = get(b + 28);
        }
        if (!passport_growth_valid(&value)) return false;
    }
    *s = value;
    return true;
}
bool passport_growth_equal(const passport_growth_t *a, const passport_growth_t *b) {
    return a->total == b->total && a->today == b->today && a->day == b->day &&
        a->growth == b->growth && a->daily == b->daily &&
        a->runtime_seconds == b->runtime_seconds && a->phase_seconds == b->phase_seconds &&
        a->clock_mode == b->clock_mode;
}
