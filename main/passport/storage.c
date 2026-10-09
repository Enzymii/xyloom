#include "storage.h"
#include "growth_record.h"
#include "life.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

typedef struct { passport_growth_t record; uint32_t day, life_stamp; bool drink, life; } save_request_t;
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
    uint32_t life_stamp = 0;
    bool life_ready = ready;
    if (ready) {
        uint8_t bytes[LIFE_RECORD_SIZE]; size_t size = sizeof(bytes);
        err = nvs_get_blob(handle, "life", bytes, &size);
        life_ready = err == ESP_ERR_NVS_NOT_FOUND ||
            (err == ESP_OK && passport_life_decode(bytes, size, &life_stamp));
        if (!life_ready) ESP_LOGW(TAG, "Remembered scene time unavailable; preserving record");
    }
    passport_storage_result_t result = {.loaded = true, .success = ready, .record = state,
        .life_stamp = life_stamp, .life_ready = life_ready};
    xQueueSend(s_results, &result, portMAX_DELAY);
    for (;;) {
        save_request_t request;
        passport_growth_t expected, requested;
        if (xQueueReceive(s_requests, &request, portMAX_DELAY) != pdTRUE) continue;
        if (request.life) {
            err = ESP_ERR_INVALID_STATE;
            if (life_ready && passport_life_time_valid(request.life_stamp)) {
                uint8_t bytes[LIFE_RECORD_SIZE]; passport_life_encode(bytes, request.life_stamp);
                err = nvs_set_blob(handle, "life", bytes, sizeof(bytes));
                if (err == ESP_OK) err = nvs_commit(handle);
            }
            if (err != ESP_OK) ESP_LOGW(TAG, "Scene time save failed: %s", esp_err_to_name(err));
            result = (passport_storage_result_t){.life_saved = true, .success = err == ESP_OK,
                .life_stamp = request.life_stamp};
            xQueueSend(s_results, &result, portMAX_DELAY);
            continue;
        }
        requested = request.record;
        err = ESP_ERR_INVALID_STATE;
        bool valid = ready && requested.runtime_seconds >= state.runtime_seconds &&
            passport_growth_clock(&state, request.day, requested.runtime_seconds - state.runtime_seconds, &expected);
        if (valid && request.drink) valid = passport_growth_drink(&expected, request.day, &expected);
        if (ready && (passport_growth_equal(&requested, &state) ||
            (valid && passport_growth_equal(&requested, &expected)))) {
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
    s_requests = xQueueCreate(1, sizeof(save_request_t));
    s_results = xQueueCreate(2, sizeof(passport_storage_result_t));
    if (s_requests && s_results &&
        xTaskCreate(storage_task, "cottage_storage", 3072, NULL, 2, NULL) == pdPASS) return true;
    if (s_requests) vQueueDelete(s_requests);
    if (s_results) vQueueDelete(s_results);
    s_requests = s_results = NULL;
    return false;
}

bool passport_storage_save(const passport_growth_t *record, uint32_t day, bool drink) {
    save_request_t request = {.record = *record, .day = day, .drink = drink};
    return s_requests && xQueueSend(s_requests, &request, 0) == pdTRUE;
}

bool passport_storage_poll(passport_storage_result_t *result) {
    return s_results && xQueueReceive(s_results, result, 0) == pdTRUE;
}

bool passport_storage_save_life(uint32_t stamp) {
    save_request_t request = {.life = true, .life_stamp = stamp};
    return s_requests && xQueueSend(s_requests, &request, 0) == pdTRUE;
}
