#include "passport/input.h"
#include "passport/world.h"
#include "passport/view.h"
#include "passport/storage.h"
#include "passport/network.h"
#include "passport/life.h"
#include "bsp_button.h"
#include "bsp_battery.h"
#include <stdatomic.h>
#include <string.h>
#include "bsp_display.h"
#include "bsp_pins.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "sdkconfig.h"
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
#if CONFIG_COTTAGE_SIMULATOR_NETWORK
    static int last_mv = -100;
    static int64_t last_log;
    int64_t now = esp_timer_get_time();
    if (mv != last_mv && now - last_log >= 1000000) {
        ESP_LOGI(TAG, "Button ADC=%dmV", mv); last_mv = mv; last_log = now;
    }
#endif
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
    if (!passport_storage_start()) passport_world_storage_loaded(&world, false, NULL);
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
    bool life_ready = false, life_inflight = false;
    uint32_t life_stamp = 0, life_saved = 0, life_retry_at = 0;
    uint32_t life_tick = (uint32_t)(esp_timer_get_time() / 1000), life_remainder = 0;
    uint32_t rendered_at = 0;
    for (;;) {
        uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
        passport_storage_result_t stored;
        while (passport_storage_poll(&stored)) {
            if (stored.loaded) {
                passport_world_storage_loaded(&world, stored.success, &stored.record);
                life_stamp = life_saved = stored.life_stamp;
                life_ready = stored.life_ready;
                if (stored.success && !passport_network_start()) world.network_state = NETWORK_ERROR;
            }
            else if (stored.life_saved) {
                life_inflight = false;
                if (stored.success) life_saved = stored.life_stamp;
                life_retry_at = now;
            } else passport_world_water_saved(&world, stored.success, now);
        }
        uint32_t elapsed = now - life_tick;
        life_tick = now;
        if (passport_life_time_valid(life_stamp)) {
            uint64_t total = (uint64_t)life_remainder + elapsed;
            uint64_t next = (uint64_t)life_stamp + total / 1000;
            life_stamp = next < 4102444800u ? (uint32_t)next : 0;
            life_remainder = total % 1000;
        }
        passport_network_status_t network;
        if (passport_network_poll(&network)) {
            world.network_state = network.state;
            world.wifi_connected = network.connected;
            world.day = network.day;
            if (passport_life_time_valid(network.stamp)) { life_stamp = network.stamp; life_remainder = 0; }
            memcpy(world.setup_password, network.password, sizeof(world.setup_password));
        }
        passport_world_clock_tick(&world, now);
        passport_life_state_t life = passport_life_at(life_stamp);
        world.night = life.night;
        world.momo = life.away ? OUT : life.asleep ? HOME_SLEEP : HOME_IDLE;
        world.battery_percent = atomic_load(&s_battery);
        world.uptime_minutes = (uint32_t)(esp_timer_get_time() / 60000000);
        passport_key_t key = read_key();
        passport_input_event_t event = passport_input_sample(&input, key, now,
            world.sleeping, passport_world_busy(&world));
#if CONFIG_COTTAGE_SIMULATOR_NETWORK
        static uint32_t diagnostic_at;
        if (event != INPUT_NONE || now - diagnostic_at >= 10000) {
            ESP_LOGI(TAG, "Input sample key=%d event=%d blocked=%d uptime=%lu", key, event,
                passport_world_busy(&world), (unsigned long)now);
            diagnostic_at = now;
        }
#endif
        passport_world_handle(&world, event, now);
        passport_world_tick(&world, now, key != KEY_NONE && key != KEY_INVALID);
        passport_growth_t record;
        if (!life_inflight && passport_world_take_water_save(&world, &record) && !passport_storage_save(&record, world.save_calendar_day, !world.save_is_clock))
            passport_world_water_saved(&world, false, now);
        passport_life_state_t saved_life = passport_life_at(life_saved);
        uint32_t difference = life_stamp > life_saved ? life_stamp - life_saved : life_saved - life_stamp;
        if (life_ready && !life_inflight && !passport_world_busy(&world) &&
            passport_life_time_valid(life_stamp) && (uint32_t)(now - life_retry_at) >= 5000 &&
            (!life_saved || difference >= 900 || saved_life.night != life.night ||
             saved_life.asleep != life.asleep || saved_life.away != life.away)) {
            if (passport_storage_save_life(life_stamp)) life_inflight = true;
            life_retry_at = now;
        }
        if (world.setup_requested) { passport_network_setup(); world.setup_requested = false; }
        if (world.forget_requested) { passport_network_forget(); world.forget_requested = false; }
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
