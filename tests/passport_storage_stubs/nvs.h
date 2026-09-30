#pragma once
#include <stdint.h>
#include <stddef.h>
typedef int esp_err_t;
typedef uint32_t nvs_handle_t;
enum { ESP_OK, ESP_ERR_INVALID_STATE, ESP_ERR_NVS_NOT_FOUND, ESP_ERR_NVS_INVALID_LENGTH,
       NVS_READWRITE, TEST_STORAGE_FAILURE };
esp_err_t nvs_open(const char *, int, nvs_handle_t *);
esp_err_t nvs_get_blob(nvs_handle_t, const char *, void *, size_t *);
esp_err_t nvs_set_blob(nvs_handle_t, const char *, const void *, size_t);
esp_err_t nvs_commit(nvs_handle_t);
void nvs_close(nvs_handle_t);
static inline const char *esp_err_to_name(esp_err_t err) { (void)err; return "test"; }
