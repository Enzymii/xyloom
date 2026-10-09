#define _POSIX_C_SOURCE 200809L
#include "network_stubs.h"
#include "passport/network.h"
#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Execute the real worker with deterministic task/driver events. */
#include "../main/passport/network.c"
struct test_queue { size_t size; bool ready; unsigned char data[128]; };
static jmp_buf done;
static void (*task)(void *);
static void (*handler)(void *, esp_event_base_t, int32_t, void *);
static void (*time_cb)(struct timeval *);
static unsigned ticks, connections, writes, flash_reads, restores, storage, failures;
static unsigned stops, deinitializations, destroyed, unregistered;
static bool fail_storage;
static wifi_config_t current;

QueueHandle_t xQueueCreate(unsigned count, size_t size) {
    assert(count == 1 && size <= 128);
    struct test_queue *q = calloc(1, sizeof(*q)); assert(q); q->size = size; return q;
}
BaseType_t xQueueSend(QueueHandle_t q, const void *p, uint32_t wait) {
    (void)wait; if (q->ready) return pdFALSE;
    memcpy(q->data, p, q->size); q->ready = true; return pdTRUE;
}
BaseType_t xQueueOverwrite(QueueHandle_t q, const void *p) {
    memcpy(q->data, p, q->size); q->ready = true; return pdTRUE;
}
BaseType_t xQueueReceive(QueueHandle_t q, void *p, uint32_t wait) {
    (void)wait; if (!q->ready) return pdFALSE;
    memcpy(p, q->data, q->size); q->ready = false; return pdTRUE;
}
void vQueueDelete(QueueHandle_t q) { free(q); }
BaseType_t xTaskCreate(void (*f)(void *), const char *name, unsigned stack, void *arg, unsigned priority, void *handle) {
    (void)name; (void)stack; (void)arg; (void)priority; (void)handle; task = f; return pdPASS;
}
int64_t esp_timer_get_time(void) { return (int64_t)ticks * 250000; }
esp_err_t esp_event_handler_instance_register(esp_event_base_t b, int32_t id,
    void (*f)(void *, esp_event_base_t, int32_t, void *), void *arg, esp_event_handler_instance_t *out) {
    (void)b; (void)id; (void)arg; handler = f; *out = (void *)1; return ESP_OK;
}
esp_err_t esp_wifi_set_storage(int mode) { storage = mode; return fail_storage ? ESP_FAIL : ESP_OK; }
esp_err_t esp_wifi_init(const wifi_init_config_t *config) {
    assert(!config->nvs_enable); return ESP_OK;
}
esp_err_t esp_wifi_get_config(int mode, wifi_config_t *out) {
    (void)mode; (void)out; ++flash_reads; return ESP_FAIL;
}
esp_err_t esp_wifi_restore(void) { ++restores; return ESP_OK; }
esp_err_t esp_wifi_set_config(int mode, const wifi_config_t *config) {
    assert(mode == WIFI_IF_STA && storage == WIFI_STORAGE_RAM);
    assert(!strcmp((const char *)config->sta.ssid, "Emulator Host Bridge"));
    assert(!config->sta.password[0] && config->sta.threshold.authmode == WIFI_AUTH_OPEN);
    if (failures) { --failures; return ESP_ERR_WIFI_STATE; }
    current = *config; ++writes; return ESP_OK;
}
esp_err_t esp_wifi_connect(void) {
    ++connections; assert(current.sta.ssid[0]); handler(NULL, IP_EVENT, IP_EVENT_STA_GOT_IP, NULL); return ESP_OK;
}
esp_err_t esp_wifi_disconnect(void) {
    handler(NULL, WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, NULL); return ESP_OK;
}
esp_err_t esp_netif_sntp_init(const esp_sntp_config_t *config) { time_cb = config->sync_cb; return ESP_OK; }
esp_err_t esp_netif_sntp_start(void) {
    struct timeval tv = {.tv_sec = 1791504000}; time_cb(&tv); return ESP_OK;
}
void esp_netif_sntp_deinit(void) {}
esp_err_t esp_wifi_stop(void) { ++stops; return ESP_OK; }
esp_err_t esp_wifi_deinit(void) { ++deinitializations; return ESP_OK; }
void esp_netif_destroy_default_wifi(esp_netif_t *netif) { assert(netif); ++destroyed; }
esp_err_t esp_event_handler_instance_unregister(esp_event_base_t base, int32_t id, esp_event_handler_instance_t instance) {
    (void)base; (void)id; assert(instance); ++unregistered; return ESP_OK;
}
void vTaskDelete(void *unused) { (void)unused; longjmp(done, 1); }
void vTaskDelay(unsigned ms) {
    assert(ms == 250);
    passport_network_status_t status;
    assert(passport_network_poll(&status));
    ++ticks;
    if (ticks == 1) {
        assert(status.connected && status.day && connections == 1);
        passport_network_forget();
    } else if (ticks <= 3) {
        assert(!status.connected && status.state == NETWORK_OFFLINE && connections == 1);
        if (ticks == 3) { failures = 1; passport_network_setup(); }
    } else if (ticks <= 5) {
        assert(!status.connected && status.state == NETWORK_CONNECTING && connections == 1);
    } else if (ticks == 6) {
        assert(status.connected && status.day && connections == 2 && writes == 2);
        passport_network_setup(); /* Repeated setup while already connected. */
    } else if (ticks == 7) {
        assert(!status.connected && status.state == NETWORK_CONNECTING && connections == 2);
    } else {
        assert(status.connected && status.day && connections == 3 && writes == 3);
        assert(flash_reads == 0 && restores == 0);
        longjmp(done, 1);
    }
}
int main(void) {
    assert(passport_network_start()); assert(!passport_network_start());
    if (!setjmp(done)) task(NULL);
    assert(ticks == 8);
    vQueueDelete(s_status); vQueueDelete(s_credentials); s_status = s_credentials = NULL;
    ticks = 0; fail_storage = true;
    assert(passport_network_start());
    if (!setjmp(done)) task(NULL);
    passport_network_status_t status;
    assert(passport_network_poll(&status) && status.state == NETWORK_ERROR && !status.connected);
    assert(!ticks && connections == 3 && flash_reads == 0 && restores == 0);
    assert(stops == 1 && deinitializations == 1 && destroyed == 2 && unregistered == 2);
    vQueueDelete(s_status); vQueueDelete(s_credentials);
    puts("Cottage simulator worker: PASS (connect, SNTP, forget, reconnect, transient retry, startup failure)");
    return 0;
}
