#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef int esp_err_t;
enum { ESP_OK, ESP_FAIL, ESP_ERR_INVALID_STATE, ESP_ERR_NO_MEM, ESP_ERR_WIFI_STATE };
typedef int esp_event_base_t;
typedef void *esp_event_handler_instance_t;
enum { WIFI_EVENT = 1, IP_EVENT, WIFI_EVENT_STA_DISCONNECTED, IP_EVENT_STA_GOT_IP };
typedef struct { int unused; } esp_netif_t;
typedef struct { int unused; } esp_netif_config_t;
typedef struct { bool nvs_enable; } wifi_init_config_t;
#define ESP_NETIF_DEFAULT_WIFI_STA() ((esp_netif_config_t){0})
#define ESP_NETIF_DEFAULT_WIFI_AP() ((esp_netif_config_t){0})
#define WIFI_INIT_CONFIG_DEFAULT() ((wifi_init_config_t){0})
enum { WIFI_STORAGE_FLASH, WIFI_STORAGE_RAM, WIFI_MODE_STA, WIFI_MODE_APSTA,
       WIFI_IF_STA, WIFI_IF_AP, WIFI_AUTH_OPEN, WIFI_AUTH_WPA2_PSK };
typedef struct {
    struct { uint8_t ssid[32], password[64]; struct { bool capable; } pmf_cfg;
             struct { int authmode; } threshold; } sta;
    struct { uint8_t ssid[32], password[64]; int ssid_len, authmode, max_connection, channel; } ap;
} wifi_config_t;
typedef struct { uint8_t ssid[33]; } wifi_ap_record_t;
typedef struct { void (*sync_cb)(struct timeval *); bool start; } esp_sntp_config_t;
#define ESP_NETIF_SNTP_DEFAULT_CONFIG(server) ((esp_sntp_config_t){0})
typedef void *httpd_handle_t;
typedef struct { int content_len; } httpd_req_t;
typedef struct { int max_open_sockets, stack_size, recv_wait_timeout, send_wait_timeout; } httpd_config_t;
#define HTTPD_DEFAULT_CONFIG() ((httpd_config_t){0})
typedef struct { const char *uri; int method; esp_err_t (*handler)(httpd_req_t *); } httpd_uri_t;
enum { HTTP_GET, HTTP_POST, HTTPD_403_FORBIDDEN, HTTPD_400_BAD_REQUEST,
       HTTPD_408_REQ_TIMEOUT, HTTPD_RESP_USE_STRLEN = -1 };
typedef int BaseType_t;
enum { pdFALSE, pdTRUE, pdPASS = pdTRUE };
typedef struct test_queue *QueueHandle_t;
#define pdMS_TO_TICKS(ms) (ms)
QueueHandle_t xQueueCreate(unsigned, size_t);
BaseType_t xQueueSend(QueueHandle_t, const void *, uint32_t);
BaseType_t xQueueReceive(QueueHandle_t, void *, uint32_t);
BaseType_t xQueueOverwrite(QueueHandle_t, const void *);
void vQueueDelete(QueueHandle_t);
BaseType_t xTaskCreate(void (*)(void *), const char *, unsigned, void *, unsigned, void *);
void vTaskDelay(unsigned);
void vTaskDelete(void *);
int64_t esp_timer_get_time(void);
esp_err_t esp_event_handler_instance_register(esp_event_base_t, int32_t,
    void (*)(void *, esp_event_base_t, int32_t, void *), void *, esp_event_handler_instance_t *);
esp_err_t esp_wifi_set_storage(int);
esp_err_t esp_wifi_set_config(int, const wifi_config_t *);
esp_err_t esp_wifi_connect(void);
esp_err_t esp_wifi_disconnect(void);
esp_err_t esp_wifi_get_config(int, wifi_config_t *);
esp_err_t esp_wifi_restore(void);
esp_err_t esp_netif_sntp_init(const esp_sntp_config_t *);
esp_err_t esp_netif_sntp_start(void);
void esp_netif_sntp_deinit(void);
#define ESP_LOGI(tag, ...) ((void)(tag))
#define ESP_LOGW(tag, ...) ((void)(tag))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
static inline esp_err_t esp_netif_init(void) { return ESP_OK; }
static inline esp_err_t esp_event_loop_create_default(void) { return ESP_OK; }
static inline esp_netif_t *esp_netif_new(const esp_netif_config_t *c) { static esp_netif_t n; return &n; }
static inline esp_err_t esp_netif_attach_wifi_station(esp_netif_t *n) { return ESP_OK; }
static inline esp_err_t esp_netif_attach_wifi_ap(esp_netif_t *n) { return ESP_OK; }
static inline esp_err_t esp_wifi_set_default_wifi_sta_handlers(void) { return ESP_OK; }
static inline esp_err_t esp_wifi_set_default_wifi_ap_handlers(void) { return ESP_OK; }
esp_err_t esp_wifi_init(const wifi_init_config_t *c);
static inline esp_err_t esp_wifi_set_mode(int m) { return ESP_OK; }
static inline esp_err_t esp_wifi_start(void) { return ESP_OK; }
esp_err_t esp_wifi_stop(void);
esp_err_t esp_wifi_deinit(void);
void esp_netif_destroy_default_wifi(esp_netif_t *n);
esp_err_t esp_event_handler_instance_unregister(esp_event_base_t b, int32_t i, esp_event_handler_instance_t h);
static inline const char *esp_err_to_name(esp_err_t e) { return "stub"; }
static inline esp_err_t httpd_stop(httpd_handle_t h) { return ESP_OK; }
#pragma GCC diagnostic pop
