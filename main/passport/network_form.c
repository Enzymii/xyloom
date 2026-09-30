#include "network_form.h"
#include <string.h>

static int hex(char c) {
    return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 :
           c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
}
static bool decode(const char *in, size_t n, char *out, size_t capacity) {
    size_t used = 0;
    for (size_t i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)in[i];
        if (c == '%') {
            if (i + 2 >= n || hex(in[i+1]) < 0 || hex(in[i+2]) < 0) return false;
            c = (unsigned char)(hex(in[i+1]) * 16 + hex(in[i+2])); i += 2;
        } else if (c == '+') c = ' ';
        if (!c || c < 32 || c == 127 || used + 1 >= capacity) return false;
        out[used++] = (char)c;
    }
    out[used] = 0; return true;
}
bool passport_network_form(const char *body, size_t length, passport_wifi_credentials_t *out) {
    passport_wifi_credentials_t result = {0};
    bool ssid = false, password = false;
    size_t start = 0;
    while (start < length) {
        size_t end = start;
        while (end < length && body[end] != '&') ++end;
        size_t eq = start;
        while (eq < end && body[eq] != '=') ++eq;
        if (eq == end) return false;
        if (eq - start == 4 && !memcmp(body + start, "ssid", 4)) {
            if (ssid || !decode(body + eq + 1, end - eq - 1, result.ssid, sizeof(result.ssid))) return false;
            ssid = true;
        } else if (eq - start == 8 && !memcmp(body + start, "password", 8)) {
            if (password || !decode(body + eq + 1, end - eq - 1, result.password, sizeof(result.password))) return false;
            password = true;
        } else return false;
        start = end + 1;
    }
    if (!ssid || !password || !result.ssid[0] || strlen(result.password) < 8) return false;
    *out = result; return true;
}
