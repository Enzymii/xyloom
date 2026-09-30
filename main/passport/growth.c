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
        (s->day ? passport_day_valid(s->day) : (s->today == 0 && s->daily == 0));
}

bool passport_growth_drink(const passport_growth_t *s, uint32_t day, passport_growth_t *next) {
    if (!passport_growth_valid(s) || !passport_day_valid(day) || day < s->day ||
        s->total == UINT32_MAX) return false;
    passport_growth_t value = *s;
    if (day != s->day) { value.day = day; value.today = 0; value.daily = 0; }
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
