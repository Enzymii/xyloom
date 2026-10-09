#include "passport/network_access.h"
#include "passport/growth.h"
#include "passport/growth_record.h"
#include "passport/watering_record.h"
#include "passport/network_form.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void offline_tests(void) {
    passport_growth_t s = {0}, next, reboot;
    uint8_t bytes[GROWTH_RECORD_SIZE];
    for (unsigned i = 0; i < 7; ++i) {
        assert(passport_growth_drink(&s, 0, &s));
        assert(s.today == i + 1 && s.daily == (i < 4 ? i + 1 : 4));
        assert(s.growth == s.daily && !s.day && s.clock_mode == CLOCK_LOCAL);
    }
    assert(passport_growth_clock(&s, 0, 86399, &s));
    passport_growth_encode(bytes, &s);
    assert(passport_growth_decode(bytes, sizeof(bytes), &reboot));
    assert(reboot.phase_seconds == 86399 && reboot.daily == 4 && reboot.total == 7);
    assert(passport_growth_drink(&reboot, 0, &next) && next.growth == 4 && next.today == 8);
    assert(passport_growth_clock(&next, 0, 1, &s));
    assert(!s.today && !s.daily && s.growth == 4 && s.runtime_seconds == 86400);
    assert(passport_growth_drink(&s, 0, &s) && s.growth == 5 && s.total == 9);
    assert(passport_growth_clock(&s, 0, 86400 * 3 + 17, &s));
    assert(!s.daily && !s.today && s.phase_seconds == 17 && s.growth == 5);
    for (unsigned i = 0; i < 5; ++i) assert(passport_growth_drink(&s, 0, &s));
    assert(s.daily == 4 && s.growth == 9); /* Missed cycles do not bank allowances. */
    assert(passport_growth_clock(&s, 20261008, 0, &s));
    assert(s.clock_mode == CLOCK_BRIDGED && s.daily == 4 && s.today == 5);
    assert(passport_growth_drink(&s, 20261008, &s) && s.growth == 9 && s.today == 6);
    assert(passport_growth_clock(&s, 20261009, 0, &s));
    assert(s.clock_mode == CLOCK_CALENDAR && !s.daily && !s.today);
    assert(passport_growth_drink(&s, 20261009, &s) && s.growth == 10);
    assert(passport_growth_clock(&s, 20261008, 60, &s));
    assert(s.clock_mode == CLOCK_LOCAL && s.day == 20261009 && s.phase_seconds == 60);
    assert(passport_growth_drink(&s, 20261008, &s) && s.daily == 2);
    assert(passport_growth_clock(&s, 20261009, 1, &s));
    assert(s.clock_mode == CLOCK_BRIDGED && s.daily == 2 && s.growth == 11);
    assert(passport_growth_clock(&s, 0, 10, &s));
    assert(s.clock_mode == CLOCK_LOCAL && s.daily == 2 && s.phase_seconds == 10);
    assert(passport_growth_clock(&s, 20261012, 0, &s));
    assert(s.daily == 2 && s.clock_mode == CLOCK_BRIDGED); /* Cold boot cannot date the offline cups. */
    s.runtime_seconds = UINT64_MAX;
    assert(!passport_growth_clock(&s, 0, 1, &next));
    s = (passport_growth_t){.phase_seconds = 86400};
    assert(!passport_growth_valid(&s));
    s = (passport_growth_t){.total = 4, .today = 4, .day = 20261008, .growth = 4,
        .daily = 4, .clock_mode = CLOCK_CALENDAR};
    passport_growth_encode(bytes, &s);
    bytes[4] = 2; bytes[7] = 0; /* The prior 24-byte version retains its plant/quota. */
    assert(passport_growth_decode(bytes, 24, &reboot));
    assert(reboot.growth == 4 && reboot.daily == 4 && reboot.clock_mode == CLOCK_CALENDAR);
    assert(passport_growth_drink(&reboot, 0, &next) && next.growth == 4 && next.today == 5);
    for (size_t i = 0; i < 12; ++i) assert(!passport_growth_decode(bytes, i, &reboot));
    bytes[4] = 4; assert(!passport_growth_decode(bytes, sizeof(bytes), &reboot));
}

int main(void) {
    offline_tests();
    const uint8_t ap4[] = {192,168,4,1};
    const uint8_t ap6[] = {0,0,0,0,0,0,0,0,0,0,255,255,192,168,4,1};
    const uint8_t lan4[] = {192,168,1,1};
    uint8_t bad6[16]; memcpy(bad6, ap6, sizeof(bad6)); bad6[15] = 2;
    assert(passport_setup_address(ap4, sizeof(ap4)));
    assert(passport_setup_address(ap6, sizeof(ap6)));
    assert(!passport_setup_address(lan4, sizeof(lan4)));
    assert(!passport_setup_address(bad6, sizeof(bad6)));
    bad6[15] = 1; bad6[0] = 0x20;
    assert(!passport_setup_address(bad6, sizeof(bad6)));
    assert(!passport_setup_address(ap6, 15));
    assert(!passport_setup_address(NULL, 4));

    assert(passport_day_valid(20240229) && !passport_day_valid(20260229));
    assert(!passport_day_valid(20260001) && !passport_day_valid(20261301));
    assert(passport_day_next(20240228) == 20240229);
    assert(passport_day_next(20240229) == 20240301);
    assert(passport_day_next(20261231) == 20270101);
    assert(passport_day_next(20991231) == 0);
    passport_growth_t s = {0}, next, decoded;
    assert(passport_growth_drink(&s, 0, &next) && next.total == 1 && next.growth == 1);
    uint32_t day = 20261001;
    for (unsigned d = 0; d < 14; ++d) {
        for (unsigned i = 0; i < 7; ++i) {
            assert(passport_growth_drink(&s, day, &next)); s = next;
            assert(s.daily == (i < 4 ? i+1 : 4));
            assert(s.today == i+1 && s.growth == d*4 + (i < 4 ? i+1 : 4));
            uint8_t bytes[GROWTH_RECORD_SIZE];
            passport_growth_encode(bytes, &s);
            assert(passport_growth_decode(bytes, sizeof(bytes), &decoded));
            assert(passport_growth_equal(&s, &decoded)); /* Reboot retains daily quota. */
        }
        if (d < 13) assert(passport_growth_stage(s.growth) != 3);
        day = passport_day_next(day);
    }
    assert(s.growth == 56 && s.total == 98 && passport_growth_stage(s.growth) == 3);
    assert(passport_growth_drink(&s, 20261001, &next) && next.clock_mode == CLOCK_LOCAL && next.daily == 4 && next.growth == 56); /* Rollback retains the allowance. */
    assert(passport_growth_drink(&s, day, &next)); s = next;
    assert(s.today == 1 && s.daily == 1 && s.growth == 56 && s.total == 99);
    assert(passport_growth_stage(3) == 0 && passport_growth_stage(4) == 1);
    assert(passport_growth_stage(19) == 1 && passport_growth_stage(20) == 2);
    assert(passport_growth_stage(55) == 2 && passport_growth_stage(56) == 3);
    assert(passport_growth_stage_percent(0) == 0);
    assert(passport_growth_stage_percent(2) == 50);
    assert(passport_growth_stage_percent(3) == 75);
    assert(passport_growth_stage_percent(4) == 0);
    assert(passport_growth_stage_percent(8) == 25);
    assert(passport_growth_stage_percent(12) == 50);
    assert(passport_growth_stage_percent(19) == 93);
    assert(passport_growth_stage_percent(20) == 0);
    assert(passport_growth_stage_percent(38) == 50);
    assert(passport_growth_stage_percent(55) == 97);
    assert(passport_growth_stage_percent(56) == 100);
    assert(passport_growth_stage_percent(255) == 100);

    s.total = UINT32_MAX; assert(!passport_growth_drink(&s, day, &next));
    uint8_t old[WATERING_RECORD_SIZE]; passport_watering_encode(old, 7);
    assert(passport_growth_decode(old, sizeof(old), &s) && s.total == 7 && !s.growth && !s.day);
    uint8_t bytes[GROWTH_RECORD_SIZE]; passport_growth_encode(bytes, &s);
    bytes[5] = 57; assert(!passport_growth_decode(bytes, sizeof(bytes), &decoded));
    passport_growth_encode(bytes, &s); bytes[32] = 1;
    assert(!passport_growth_decode(bytes, sizeof(bytes), &decoded));
    assert(!passport_growth_decode(bytes, 23, &decoded));
    passport_wifi_credentials_t credentials;
    const char *form = "ssid=My+WiFi%26A&password=a%2Bb%26c12345";
    assert(passport_network_form(form, strlen(form), &credentials));
    assert(!strcmp(credentials.ssid, "My WiFi&A") && !strcmp(credentials.password, "a+b&c12345"));
    const char *bad[] = {"ssid=X&password=short", "ssid=X%00&password=12345678", "ssid=X%GG&password=12345678", "ssid=X&ssid=Y&password=12345678", "ssid=X&password=12345678&extra=yes"};
    for (unsigned i = 0; i < sizeof(bad)/sizeof(bad[0]); ++i)
        assert(!passport_network_form(bad[i], strlen(bad[i]), &credentials));
    puts("Cottage daily growth, migration and Wi-Fi form tests: PASS");
    return 0;
}
