#include "passport/growth.h"
#include "passport/growth_record.h"
#include "passport/watering_record.h"
#include "passport/network_form.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    assert(passport_day_valid(20240229) && !passport_day_valid(20260229));
    assert(!passport_day_valid(20260001) && !passport_day_valid(20261301));
    assert(passport_day_next(20240228) == 20240229);
    assert(passport_day_next(20240229) == 20240301);
    assert(passport_day_next(20261231) == 20270101);
    assert(passport_day_next(20991231) == 0);
    passport_growth_t s = {0}, next, decoded;
    assert(!passport_growth_drink(&s, 0, &next));
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
    assert(!passport_growth_drink(&s, 20261001, &next)); /* Clock rollback. */
    assert(passport_growth_drink(&s, day, &next)); s = next;
    assert(s.today == 1 && s.daily == 1 && s.growth == 56 && s.total == 99);
    assert(passport_growth_stage(3) == 0 && passport_growth_stage(4) == 1);
    assert(passport_growth_stage(19) == 1 && passport_growth_stage(20) == 2);
    assert(passport_growth_stage(55) == 2 && passport_growth_stage(56) == 3);
    s.total = UINT32_MAX; assert(!passport_growth_drink(&s, day, &next));
    uint8_t old[WATERING_RECORD_SIZE]; passport_watering_encode(old, 7);
    assert(passport_growth_decode(old, sizeof(old), &s) && s.total == 7 && !s.growth && !s.day);
    uint8_t bytes[GROWTH_RECORD_SIZE]; passport_growth_encode(bytes, &s);
    bytes[5] = 57; assert(!passport_growth_decode(bytes, sizeof(bytes), &decoded));
    passport_growth_encode(bytes, &s); bytes[20] = 1;
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
