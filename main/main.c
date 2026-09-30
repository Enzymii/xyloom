#include "passport/input.h"
#include "passport/world.h"
#include "passport/view.h"
#include "bsp_button.h"
#include "bsp_battery.h"
#include <stdatomic.h>
#include "bsp_display.h"
#include "bsp_pins.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "passport";
static const uint16_t s_windows[BSP_BTN_COUNT][2] = BSP_BTN_MV_TABLE;

static atomic_int s_battery = ATOMIC_VAR_INIT(-1);

/* I2C initialization and reads must not stall the input or LVGL tasks. */
static void battery_task(void *arg) {
    (void)arg;
    if (bsp_battery_init() != ESP_OK) {
        ESP_LOGW(TAG, "Battery unavailable");
        vTaskDelete(NULL);
        return;
    }
    for (;;) {
        atomic_store(&s_battery, bsp_battery_soc());
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

static passport_key_t read_key(void) {
    int mv = bsp_button_read_mv();
    if (mv < 0) return KEY_INVALID;
    static const passport_key_t keys[] = {KEY_LEFT, KEY_RIGHT, KEY_OK};
    for (unsigned i = 0; i < BSP_BTN_COUNT; ++i) {
        if (mv >= s_windows[i][0] && mv < s_windows[i][1]) return keys[i];
    }
    return KEY_NONE;
}

void app_main(void) {
    ESP_ERROR_CHECK(bsp_display_init());
    bsp_display_backlight(0);
    if (!bsp_lvgl_init()) {
        ESP_LOGE(TAG, "LVGL initialization failed");
        return;
    }
    /* BSP retains its sole ADC owner. Application polls the shared calibrated
     * reader, defining 700ms holds and release-based taps independently of
     * the baseline demo's double-click and 500ms callback semantics. */
    ESP_ERROR_CHECK(bsp_button_init(NULL, NULL));
    passport_input_t input;
    passport_world_t world;
    passport_input_init(&input);
    passport_world_init(&world, (uint32_t)(esp_timer_get_time() / 1000));
    if (!bsp_lvgl_lock(1000)) {
        ESP_LOGE(TAG, "Cannot create initial view");
        return;
    }
    passport_view_create();
    passport_view_render(&world);
    bsp_lvgl_unlock();
    bsp_display_backlight(80);
    ESP_LOGI(TAG, "Milestone A; UP=LEFT DOWN=RIGHT; hold=%dms; heap=%u largest=%u",
        INPUT_LONG_MS, (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
        (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    if (xTaskCreate(battery_task, "passport_battery", 3072, NULL, 2, NULL) != pdPASS)
        ESP_LOGW(TAG, "Battery worker unavailable");
    TickType_t wake = xTaskGetTickCount();
    bool backlight_sleeping = false;
    uint32_t rendered_at = 0;
    for (;;) {
        uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
        world.battery_percent = atomic_load(&s_battery);
        world.uptime_minutes = (uint32_t)(esp_timer_get_time() / 60000000);
        passport_key_t key = read_key();
        passport_input_event_t event = passport_input_sample(&input, key, now,
            world.sleeping, world.transitioning);
        passport_world_handle(&world, event, now);
        passport_world_tick(&world, now, key != KEY_NONE && key != KEY_INVALID);
        if (world.sleeping != backlight_sleeping) {
            backlight_sleeping = world.sleeping;
            bsp_display_backlight(world.sleeping ? 0 : 80);
        }
        if (!world.sleeping && (uint32_t)(now - rendered_at) >= 20 && bsp_lvgl_lock(5)) {
            passport_view_render(&world);
            bsp_lvgl_unlock();
            rendered_at = now;
        }
        vTaskDelayUntil(&wake, pdMS_TO_TICKS(10));
    }
}
