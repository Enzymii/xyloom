#include "watering_record.h"
#include <string.h>

void passport_watering_encode(uint8_t record[WATERING_RECORD_SIZE], uint32_t count) {
    memcpy(record, "XYWC", 4);
    record[4] = 1;
    record[5] = record[6] = record[7] = 0;
    for (unsigned i = 0; i < 4; ++i) record[8 + i] = (uint8_t)(count >> (8 * i));
}

bool passport_watering_decode(const uint8_t *record, size_t size, uint32_t *count) {
    if (size != WATERING_RECORD_SIZE || memcmp(record, "XYWC", 4) != 0 ||
        record[4] != 1 || record[5] || record[6] || record[7]) return false;
    uint32_t value = 0;
    for (unsigned i = 0; i < 4; ++i) value |= (uint32_t)record[8 + i] << (8 * i);
    *count = value;
    return true;
}
