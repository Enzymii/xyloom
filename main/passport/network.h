#pragma once
#include <stdbool.h>
#include <stdint.h>
enum { NETWORK_OFFLINE, NETWORK_CONNECTING, NETWORK_CONNECTED, NETWORK_SETUP, NETWORK_ERROR };
typedef struct {
    unsigned state;
    bool connected;
    uint32_t day;
    uint32_t stamp; /* Trusted UTC seconds; zero before accepted SNTP. */
    char password[9];
} passport_network_status_t;
/* Lifetime worker, no LVGL access; call only after NVS initialization succeeds. */
bool passport_network_start(void);
void passport_network_setup(void);
void passport_network_forget(void);
bool passport_network_poll(passport_network_status_t *status);
