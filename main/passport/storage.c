#include "storage.h"
#include "growth_record.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

static QueueHandle_t s_requests, s_results;
static const char *TAG = "cottage_storage";

static esp_err_t load(nvs_handle_t *handle, passport_growth_t *record) {
    esp_err_t err = nvs_flash_init();
    /* Incompatible/full NVS is preserved; never erase it automatically. */
    if (err != ESP_OK) return err;
    err = nvs_open("cottage", NVS_READWRITE, handle);
    if (err != ESP_OK) return err;
    uint8_t bytes[GROWTH_RECORD_SIZE];
    size_t size = sizeof(bytes);
    err = nvs_get_blob(*handle, "watering", bytes, &size);
    if (err == ESP_ERR_NVS_NOT_FOUND) { *record = (passport_growth_t){0}; return ESP_OK; }
    if (err != ESP_OK) return err;
    return passport_growth_decode(bytes, size, record) ? ESP_OK : ESP_ERR_INVALID_STATE;
}

static void storage_task(void *arg) {
    (void)arg;
    nvs_handle_t handle = 0;
    passport_growth_t state = {0};
    esp_err_t err = load(&handle, &state);
    bool ready = err == ESP_OK;
    if (!ready) {
        if (handle) nvs_close(handle);
        ESP_LOGE(TAG, "Watering records unavailable: %s", esp_err_to_name(err));
    }
    passport_storage_result_t result = {.loaded = true, .success = ready, .record = state};
    xQueueSend(s_results, &result, portMAX_DELAY);
    for (;;) {
        passport_growth_t requested, expected;
        if (xQueueReceive(s_requests, &requested, portMAX_DELAY) != pdTRUE) continue;
        err = ESP_ERR_INVALID_STATE;
        if (ready && (passport_growth_equal(&requested, &state) ||
            (passport_growth_drink(&state, requested.day, &expected) && passport_growth_equal(&requested, &expected)))) {
            if (passport_growth_equal(&requested, &state)) err = ESP_OK; /* Idempotent acknowledgement retry. */
            else {
                uint8_t bytes[GROWTH_RECORD_SIZE];
                passport_growth_encode(bytes, &requested);
                err = nvs_set_blob(handle, "watering", bytes, sizeof(bytes));
                if (err == ESP_OK) err = nvs_commit(handle);
                if (err == ESP_OK) state = requested;
            }
        }
        if (err != ESP_OK) ESP_LOGW(TAG, "Watering save failed: %s", esp_err_to_name(err));
        result = (passport_storage_result_t){.loaded = false, .success = err == ESP_OK, .record = requested};
        xQueueSend(s_results, &result, portMAX_DELAY);
    }
}

bool passport_storage_start(void) {
    if (s_requests || s_results) return false;
    s_requests = xQueueCreate(1, sizeof(passport_growth_t));
    s_results = xQueueCreate(2, sizeof(passport_storage_result_t));
    if (s_requests && s_results &&
        xTaskCreate(storage_task, "cottage_storage", 3072, NULL, 2, NULL) == pdPASS) return true;
    if (s_requests) vQueueDelete(s_requests);
    if (s_results) vQueueDelete(s_results);
    s_requests = s_results = NULL;
    return false;
}

bool passport_storage_save(const passport_growth_t *record) {
    return s_requests && xQueueSend(s_requests, record, 0) == pdTRUE;
}

bool passport_storage_poll(passport_storage_result_t *result) {
    return s_results && xQueueReceive(s_results, result, 0) == pdTRUE;
}
