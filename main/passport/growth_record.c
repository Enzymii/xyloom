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
    memcpy(b, "XYWC", 4); b[4] = 2; b[5] = s->growth; b[6] = s->daily;
    put(b + 8, s->total); put(b + 12, s->today); put(b + 16, s->day);
}
bool passport_growth_decode(const uint8_t *b, size_t size, passport_growth_t *s) {
    passport_growth_t value = {0};
    if (size == WATERING_RECORD_SIZE) {
        if (!passport_watering_decode(b, size, &value.total)) return false;
        /* Legacy records have no date evidence: preserve totals, start a new plant. */
    } else {
        if (size != GROWTH_RECORD_SIZE || memcmp(b, "XYWC", 4) || b[4] != 2 ||
            b[7] || b[20] || b[21] || b[22] || b[23]) return false;
        value = (passport_growth_t){get(b + 8), get(b + 12), get(b + 16), b[5], b[6]};
        if (!passport_growth_valid(&value)) return false;
    }
    *s = value;
    return true;
}
bool passport_growth_equal(const passport_growth_t *a, const passport_growth_t *b) {
    return a->total == b->total && a->today == b->today && a->day == b->day &&
        a->growth == b->growth && a->daily == b->daily;
}
