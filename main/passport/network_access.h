#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Network-order address bytes; accept only the setup AP's IPv4 address,
 * including the dual-stack server's IPv4-mapped IPv6 representation. */
static inline bool passport_setup_address(const uint8_t *address, size_t size) {
    if (!address) return false;
    if (size == 16) {
        for (unsigned i = 0; i < 10; ++i) if (address[i]) return false;
        if (address[10] != 0xff || address[11] != 0xff) return false;
        address += 12;
    } else if (size != 4) return false;
    return address[0] == 192 && address[1] == 168 && address[2] == 4 && address[3] == 1;
}
