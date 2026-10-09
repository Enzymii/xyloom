#include "growth.h"

static unsigned month_days(unsigned year, unsigned month) {
    static const unsigned days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month < 1 || month > 12) return 0;
    return days[month - 1] + (month == 2 && year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}

bool passport_day_valid(uint32_t day) {
    unsigned year = day / 10000, month = day / 100 % 100, date = day % 100;
    return year >= 2020 && year <= 2099 && date > 0 && date <= month_days(year, month);
}

uint32_t passport_day_next(uint32_t day) {
    if (!passport_day_valid(day) || day == 20991231) return 0;
    unsigned year = day / 10000, month = day / 100 % 100, date = day % 100;
    if (++date > month_days(year, month)) {
        date = 1;
        if (++month > 12) { month = 1; ++year; }
    }
    return year * 10000 + month * 100 + date;
}

bool passport_growth_valid(const passport_growth_t *s) {
    return s->growth <= GROWTH_BLOOM && s->growth <= s->total && s->daily <= GROWTH_DAILY_MAX &&
        s->today <= s->total && s->daily <= s->today && s->daily <= s->growth &&
        (!s->day || passport_day_valid(s->day)) && s->clock_mode <= CLOCK_AWAITING &&
        (s->clock_mode == CLOCK_LOCAL ? s->phase_seconds < CLOCK_PERIOD_SECONDS :
         s->clock_mode == CLOCK_AWAITING ?
         (passport_day_valid(s->day) && s->phase_seconds < CLOCK_PERIOD_SECONDS) :
         (passport_day_valid(s->day) && s->phase_seconds == 0));
}

bool passport_growth_clock(const passport_growth_t *s, uint32_t day,
                           uint64_t elapsed, passport_growth_t *next) {
    if (!passport_growth_valid(s) || elapsed > UINT64_MAX - s->runtime_seconds) return false;
    passport_growth_t value = *s;
    value.runtime_seconds += elapsed;
    bool trusted = passport_day_valid(day) && day >= s->day;
    if (s->clock_mode == CLOCK_LOCAL || !trusted) {
        uint64_t cycles = elapsed / CLOCK_PERIOD_SECONDS;
        uint32_t phase = (uint32_t)(elapsed % CLOCK_PERIOD_SECONDS) + s->phase_seconds;
        bool refreshed = cycles || phase >= CLOCK_PERIOD_SECONDS;
        if (refreshed) { value.today = 0; value.daily = 0; }
        value.phase_seconds = phase % CLOCK_PERIOD_SECONDS;
        /* Waiting for this boot's SNTP does not undate saved calendar cups.
         * An offline refresh or drink does create an uncertain local bucket. */
        value.clock_mode = !day && s->clock_mode != CLOCK_LOCAL && !refreshed ?
            CLOCK_AWAITING : CLOCK_LOCAL;
        if (trusted) {
            /* Bind this uncertain bucket without granting a second allowance or
             * presenting its cups as having a known calendar date. */
            value.day = day; value.phase_seconds = 0; value.clock_mode = CLOCK_BRIDGED;
        }
    } else if (day != s->day) {
        value.day = day; value.today = 0; value.daily = 0;
        value.phase_seconds = 0; value.clock_mode = CLOCK_CALENDAR;
    } else if (s->clock_mode == CLOCK_AWAITING) {
        value.phase_seconds = 0; value.clock_mode = CLOCK_BRIDGED;
    }
    *next = value;
    return true;
}

bool passport_growth_drink(const passport_growth_t *s, uint32_t day, passport_growth_t *next) {
    passport_growth_t value;
    if (s->total == UINT32_MAX || !passport_growth_clock(s, day, 0, &value)) return false;
    if (value.clock_mode == CLOCK_AWAITING) value.clock_mode = CLOCK_LOCAL;
    ++value.total; ++value.today;
    if (value.daily < GROWTH_DAILY_MAX) {
        ++value.daily;
        if (value.growth < GROWTH_BLOOM) ++value.growth;
    }
    *next = value;
    return true;
}

unsigned passport_growth_stage(uint8_t growth) {
    return growth >= 56 ? 3 : growth >= 20 ? 2 : growth >= 4 ? 1 : 0;
}

unsigned passport_growth_stage_percent(uint8_t growth) {
    if (growth >= GROWTH_BLOOM) return 100;
    unsigned start = growth >= 20 ? 20 : growth >= 4 ? 4 : 0;
    unsigned end = growth >= 20 ? GROWTH_BLOOM : growth >= 4 ? 20 : 4;
    return (growth - start) * 100 / (end - start);
}
