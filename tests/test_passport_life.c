#include "passport/life.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
/* 2026-10-01 00:00 UTC+8. */
static const uint32_t midnight = 1790784000u;
int main(void) {
    passport_life_state_t s = passport_life_at(0);
    assert(!s.night && !s.asleep && !s.away);
    s = passport_life_at(midnight + 7 * 3600 - 1); assert(s.night && s.asleep && !s.away);
    s = passport_life_at(midnight + 7 * 3600); assert(!s.night && !s.asleep && !s.away);
    s = passport_life_at(midnight + 19 * 3600 - 1); assert(!s.night && !s.asleep);
    s = passport_life_at(midnight + 19 * 3600); assert(s.night && !s.asleep);
    s = passport_life_at(midnight + 22 * 3600); assert(s.night && s.asleep && !s.away);
    uint32_t day = (midnight + 8 * 3600) / 86400;
    unsigned start = passport_life_walk_start(day);
    assert(start >= 600 && start < 1020);
    assert(!passport_life_at(midnight + start * 60 - 1).away);
    assert(passport_life_at(midnight + start * 60).away);
    assert(passport_life_at(midnight + (start + 30) * 60 - 1).away);
    assert(!passport_life_at(midnight + (start + 30) * 60).away);
    uint8_t bytes[LIFE_RECORD_SIZE]; uint32_t restored = 0;
    uint32_t departure = midnight + start * 60;
    passport_life_encode(bytes, departure);
    assert(passport_life_decode(bytes, sizeof(bytes), &restored) && restored == departure);
    assert(passport_life_at(restored).away); /* Reboot keeps the day's plan. */
    assert(!passport_life_at(restored + 1800).away); /* Offline powered time returns home. */
    assert(passport_life_at(restored - 1).away == false); /* Clock correction recomputes safely. */
    for (unsigned i = 0; i < sizeof(bytes); ++i) {
        uint8_t broken[LIFE_RECORD_SIZE]; memcpy(broken, bytes, sizeof(bytes)); broken[i] ^= 1;
        assert(!passport_life_decode(broken, sizeof(broken), &restored));
    }
    assert(!passport_life_decode(bytes, sizeof(bytes)-1, &restored));
    passport_life_encode(bytes, 0); assert(!passport_life_decode(bytes, sizeof(bytes), &restored));
    assert(!passport_life_time_valid(UINT32_MAX));
    puts("Cottage day/night, sleep, walk plan and remembered time: PASS");
    return 0;
}
