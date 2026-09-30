#pragma once
#include <stdbool.h>
#include <stddef.h>
typedef struct { char ssid[33], password[64]; } passport_wifi_credentials_t;
bool passport_network_form(const char *body, size_t length, passport_wifi_credentials_t *out);
