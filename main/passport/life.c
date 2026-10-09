#include "life.h"
#include <string.h>

bool passport_life_time_valid(uint32_t stamp) {
    return stamp >= 1577836800u && stamp < 4102444800u;
}
unsigned passport_life_walk_start(uint32_t local_day) {
    /* One reproducible half-hour walk, starting between 10:00 and 16:59.
     * Reboot never generates another plan for the same remembered day. */
    uint32_t hash = local_day * 2654435761u;
    return 600 + (hash >> 16) % 420;
}
passport_life_state_t passport_life_at(uint32_t stamp) {
    if (!passport_life_time_valid(stamp)) return (passport_life_state_t){0};
    uint64_t local = (uint64_t)stamp + 8 * 3600;
    unsigned minute = (local % 86400) / 60;
    unsigned walk = passport_life_walk_start((uint32_t)(local / 86400));
    return (passport_life_state_t){
        .night = minute < 420 || minute >= 1140,
        .asleep = minute < 420 || minute >= 1320,
        .away = minute >= walk && minute < walk + 30,
    };
}
static uint32_t get(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}
static void put(uint8_t *p, uint32_t n) {
    for (unsigned i = 0; i < 4; ++i) p[i] = (uint8_t)(n >> (8 * i));
}
static uint32_t checksum(const uint8_t *p) {
    uint32_t n = 2166136261u;
    for (unsigned i = 0; i < 12; ++i) n = (n ^ p[i]) * 16777619u;
    return n;
}
void passport_life_encode(uint8_t bytes[LIFE_RECORD_SIZE], uint32_t stamp) {
    memset(bytes, 0, LIFE_RECORD_SIZE);
    memcpy(bytes, "XYLC", 4); bytes[4] = 1;
    put(bytes + 8, stamp); put(bytes + 12, checksum(bytes));
}
bool passport_life_decode(const uint8_t *bytes, size_t size, uint32_t *stamp) {
    if (size != LIFE_RECORD_SIZE || memcmp(bytes, "XYLC", 4) ||
        bytes[4] != 1 || bytes[5] || bytes[6] || bytes[7] ||
        get(bytes + 12) != checksum(bytes) || !passport_life_time_valid(get(bytes + 8))) return false;
    *stamp = get(bytes + 8); return true;
}
