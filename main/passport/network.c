#include "network.h"
#include "network_form.h"
#include "growth.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_netif_sntp.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "esp_http_server.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "lwip/sockets.h"
#include <stdatomic.h>
#include <string.h>
#include <time.h>

static atomic_uint s_commands;
static atomic_bool s_connected, s_synced;
static QueueHandle_t s_status, s_credentials;
static httpd_handle_t s_http;
static const char *TAG = "cottage_network";
enum { COMMAND_SETUP = 1, COMMAND_FORGET = 2 };

static void event(void *arg, esp_event_base_t base, int32_t id, void *data) {
    (void)arg; (void)data;
    if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) atomic_store(&s_connected, true);
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) atomic_store(&s_connected, false);
}
static void synced(struct timeval *tv) {
    /* Ignore implausible network dates; never trust RTC state inherited from reset. */
    atomic_store(&s_synced, tv->tv_sec >= 1577836800LL && tv->tv_sec < 4102444800LL);
}
static uint32_t local_day(void) {
    if (!atomic_load(&s_synced)) return 0;
    time_t stamp = time(NULL) + 8 * 3600; /* Product calendar: UTC+8. */
    struct tm date;
    if (!gmtime_r(&stamp, &date)) return 0;
    uint32_t day = (date.tm_year + 1900) * 10000 + (date.tm_mon + 1) * 100 + date.tm_mday;
    return passport_day_valid(day) ? day : 0;
}

static bool setup_interface(httpd_req_t *request) {
    struct sockaddr_in address = {0};
    socklen_t length = sizeof(address);
    /* The setup server must not accept credential writes from the home LAN. */
    return getsockname(httpd_req_to_sockfd(request), (struct sockaddr *)&address, &length) == 0 &&
        address.sin_family == AF_INET && address.sin_addr.s_addr == htonl(0xC0A80401);
}
static esp_err_t page(httpd_req_t *request) {
    if (!setup_interface(request)) return httpd_resp_send_err(request, HTTPD_403_FORBIDDEN, "Connect to the setup hotspot");
    httpd_resp_set_type(request, "text/html; charset=utf-8");
    httpd_resp_set_hdr(request, "Cache-Control", "no-store");
    const char *head = "<!doctype html><meta charset=utf-8><meta name=viewport content='width=device-width,initial-scale=1'>"
        "<title>Xyloom Wi-Fi</title><style>body{font:18px sans-serif;max-width:420px;margin:32px auto;padding:16px;background:#fff1d9;color:#493c2d}input,button{font:inherit;width:100%;box-sizing:border-box;padding:12px;margin:8px 0}</style>"
        "<h1>Xyloom · 梦隅</h1><p>连接家里的 2.4 GHz Wi-Fi，自动同步日期。</p>"
        "<form method=post action=/connect><label>Wi-Fi 名称<input name=ssid list=networks maxlength=32 required></label><datalist id=networks>";
    if (httpd_resp_send_chunk(request, head, HTTPD_RESP_USE_STRLEN) != ESP_OK) return ESP_FAIL;
    /* HTTP server task may block while scanning; input/LVGL tasks never do. */
    if (esp_wifi_scan_start(NULL, true) == ESP_OK) {
        uint16_t count = 12;
        wifi_ap_record_t aps[12];
        if (esp_wifi_scan_get_ap_records(&count, aps) == ESP_OK) {
            for (unsigned i = 0; i < count; ++i) {
                char option[256]; size_t used = 0;
                memcpy(option, "<option value=\"", 15); used = 15;
                for (unsigned j = 0; j < sizeof(aps[i].ssid) && aps[i].ssid[j]; ++j) {
                    char c = (char)aps[i].ssid[j];
                    const char *escape = c == '&' ? "&amp;" : c == '<' ? "&lt;" : c == '>' ? "&gt;" : c == '"' ? "&quot;" : NULL;
                    if (escape) { size_t n = strlen(escape); memcpy(option + used, escape, n); used += n; }
                    else option[used++] = c;
                }
                memcpy(option + used, "\"></option>", 11); used += 11;
                if (httpd_resp_send_chunk(request, option, used) != ESP_OK) return ESP_FAIL;
            }
        }
    }
    const char *tail = "</datalist><label>密码<input type=password name=password minlength=8 maxlength=63 required autocomplete=off></label>"
        "<button>连接并保存</button></form><p>找不到网络时可手动输入名称。密码仅保存在设备本地。</p>";
    httpd_resp_send_chunk(request, tail, HTTPD_RESP_USE_STRLEN);
    return httpd_resp_send_chunk(request, NULL, 0);
}
static esp_err_t connect_form(httpd_req_t *request) {
    if (!setup_interface(request)) return httpd_resp_send_err(request, HTTPD_403_FORBIDDEN, "Connect to the setup hotspot");
    char body[384];
    if (request->content_len <= 0 || request->content_len >= sizeof(body))
        return httpd_resp_send_err(request, HTTPD_400_BAD_REQUEST, "Invalid form size");
    size_t received = 0;
    while (received < request->content_len) {
        int n = httpd_req_recv(request, body + received, request->content_len - received);
        if (n <= 0) return httpd_resp_send_err(request, HTTPD_408_REQ_TIMEOUT, "Please retry");
        received += n;
    }
    passport_wifi_credentials_t credentials;
    if (!passport_network_form(body, received, &credentials)) {
        memset(body, 0, sizeof(body));
        return httpd_resp_send_err(request, HTTPD_400_BAD_REQUEST, "Use a network name and an 8-63 byte password");
    }
    bool sent = xQueueSend(s_credentials, &credentials, 0) == pdTRUE;
    memset(body, 0, sizeof(body)); memset(&credentials, 0, sizeof(credentials));
    if (!sent) {
        httpd_resp_set_status(request, "503 Service Unavailable");
        return httpd_resp_sendstr(request, "Please retry shortly");
    }
    httpd_resp_set_type(request, "text/html; charset=utf-8");
    httpd_resp_set_hdr(request, "Cache-Control", "no-store");
    return httpd_resp_sendstr(request, "<meta charset=utf-8><p>正在连接。请查看 Passport 上的 Wi-Fi 与日期状态；连接成功后热点会关闭。失败时返回此页重试。</p><a href=/>返回</a>");
}
static void stop_setup(void) {
    if (s_http) { httpd_stop(s_http); s_http = NULL; }
    esp_wifi_set_mode(WIFI_MODE_STA);
}
static esp_err_t start_setup(char password[9]) {
    static const char alphabet[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    for (unsigned i = 0; i < 8; ++i) password[i] = alphabet[esp_random() % (sizeof(alphabet) - 1)];
    password[8] = 0;
    esp_wifi_disconnect();
    esp_err_t err = esp_wifi_set_mode(WIFI_MODE_APSTA);
    if (err != ESP_OK) return err;
    wifi_config_t ap = {0};
    memcpy(ap.ap.ssid, "Xyloom-Setup", 12); ap.ap.ssid_len = 12;
    memcpy(ap.ap.password, password, 8); ap.ap.authmode = WIFI_AUTH_WPA2_PSK;
    ap.ap.max_connection = 1; ap.ap.channel = 1;
    err = esp_wifi_set_config(WIFI_IF_AP, &ap);
    memset(&ap, 0, sizeof(ap));
    if (err != ESP_OK) return err;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_open_sockets = 2; config.stack_size = 5120;
    config.recv_wait_timeout = 5; config.send_wait_timeout = 5;
    err = httpd_start(&s_http, &config);
    if (err != ESP_OK) return err;
    const httpd_uri_t get = {.uri = "/", .method = HTTP_GET, .handler = page};
    const httpd_uri_t post = {.uri = "/connect", .method = HTTP_POST, .handler = connect_form};
    err = httpd_register_uri_handler(s_http, &get);
    if (err == ESP_OK) err = httpd_register_uri_handler(s_http, &post);
    return err;
}

static void worker(void *arg) {
    (void)arg;
    passport_network_status_t status = {.state = NETWORK_OFFLINE};
    bool wifi_ready = false, setup = false, credentials_saved = false, sntp_ready = false;
    bool config_pending = false;
    wifi_config_t pending_config = {0};
    unsigned retries = 0;
    int64_t retry_at = 0, setup_until = 0, connected_at = 0, config_at = 0, config_until = 0;
    esp_event_handler_instance_t wifi_handler = NULL, ip_handler = NULL;
    esp_netif_t *sta = NULL, *ap = NULL;
    esp_err_t err = esp_netif_init();
    if (err == ESP_OK) err = esp_event_loop_create_default();
    if (err == ESP_ERR_INVALID_STATE) err = ESP_OK;
    if (err != ESP_OK) goto failed;
    esp_netif_config_t sta_cfg = ESP_NETIF_DEFAULT_WIFI_STA();
    esp_netif_config_t ap_cfg = ESP_NETIF_DEFAULT_WIFI_AP();
    sta = esp_netif_new(&sta_cfg); ap = esp_netif_new(&ap_cfg);
    if (!sta || !ap) { err = ESP_ERR_NO_MEM; goto failed; }
    err = esp_netif_attach_wifi_station(sta);
    if (err == ESP_OK) err = esp_wifi_set_default_wifi_sta_handlers();
    if (err == ESP_OK) err = esp_netif_attach_wifi_ap(ap);
    if (err == ESP_OK) err = esp_wifi_set_default_wifi_ap_handlers();
    if (err != ESP_OK) goto failed;
    wifi_init_config_t wifi = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&wifi);
    if (err != ESP_OK) goto failed;
    wifi_ready = true;
    err = esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, event, NULL, &wifi_handler);
    if (err == ESP_OK) err = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, event, NULL, &ip_handler);
    if (err == ESP_OK) err = esp_wifi_set_storage(WIFI_STORAGE_FLASH);
    if (err == ESP_OK) err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err == ESP_OK) err = esp_wifi_start();
    if (err != ESP_OK) goto failed;
    wifi_config_t saved = {0};
    err = esp_wifi_get_config(WIFI_IF_STA, &saved);
    credentials_saved = err == ESP_OK && saved.sta.ssid[0];
    memset(&saved, 0, sizeof(saved));
    esp_sntp_config_t ntp = ESP_NETIF_SNTP_DEFAULT_CONFIG("ntp.aliyun.com");
    ntp.sync_cb = synced; ntp.start = false;
    err = esp_netif_sntp_init(&ntp);
    if (err != ESP_OK) goto failed;
    sntp_ready = true;
    for (;;) {
        int64_t now = esp_timer_get_time() / 1000;
        unsigned commands = atomic_exchange(&s_commands, 0);
        if (commands & COMMAND_FORGET) {
            stop_setup(); setup = false;
            config_pending = false; memset(&pending_config, 0, sizeof(pending_config));
            passport_wifi_credentials_t discarded;
            while (xQueueReceive(s_credentials, &discarded, 0) == pdTRUE) memset(&discarded, 0, sizeof(discarded));
            esp_wifi_disconnect(); atomic_store(&s_connected, false);
            /* Clear only Wi-Fi configuration, never application watering records. */
            err = esp_wifi_restore();
            if (err == ESP_OK) err = esp_wifi_set_mode(WIFI_MODE_STA);
            if (err == ESP_OK) { credentials_saved = false; status.state = NETWORK_OFFLINE; }
            else status.state = NETWORK_ERROR;
            memset(status.password, 0, sizeof(status.password));
        } else if ((commands & COMMAND_SETUP) && !setup) {
            err = start_setup(status.password);
            setup = err == ESP_OK;
            if (!setup) { stop_setup(); status.state = NETWORK_ERROR; }
            else { setup_until = now + 300000; status.state = NETWORK_SETUP; }
            retries = 0; connected_at = 0;
        }
        passport_wifi_credentials_t credentials;
        if (xQueueReceive(s_credentials, &credentials, 0) == pdTRUE) {
            memset(&pending_config, 0, sizeof(pending_config));
            memcpy(pending_config.sta.ssid, credentials.ssid, strlen(credentials.ssid));
            memcpy(pending_config.sta.password, credentials.password, strlen(credentials.password));
            pending_config.sta.pmf_cfg.capable = true;
            esp_wifi_disconnect(); atomic_store(&s_connected, false);
            memset(&credentials, 0, sizeof(credentials));
            config_pending = true; config_at = now + 250; config_until = now + 5000;
            connected_at = 0;
        }
        if (config_pending && now >= config_at) {
            /* Disconnect is asynchronous: retry only the transient connecting state. */
            err = esp_wifi_set_config(WIFI_IF_STA, &pending_config);
            if (err != ESP_ERR_WIFI_STATE || now >= config_until) {
                config_pending = false; memset(&pending_config, 0, sizeof(pending_config));
                if (err == ESP_OK) { credentials_saved = true; retries = 1; retry_at = now + 10000; esp_wifi_connect(); }
                else { status.state = NETWORK_ERROR; retries = 0; }
            }
        }
        bool connected = atomic_load(&s_connected);
        if (connected) {
            if (!connected_at) { connected_at = now; esp_netif_sntp_start(); }
            retries = 0;
            if (setup && now - connected_at >= 5000) { stop_setup(); setup = false; memset(status.password, 0, sizeof(status.password)); }
            status.state = setup ? NETWORK_SETUP : NETWORK_CONNECTED;
        } else {
            connected_at = 0;
            if (!config_pending && credentials_saved && now >= retry_at && (!setup || retries > 0 || !status.password[0])) {
                esp_wifi_connect();
                ++retries;
                retry_at = now + (retries < 5 ? 10000 : 60000);
                status.state = setup ? NETWORK_SETUP : retries <= 5 ? NETWORK_CONNECTING : NETWORK_ERROR;
            }
        }
        if (setup && now >= setup_until) {
            stop_setup(); setup = false; retries = 0; retry_at = now;
            memset(status.password, 0, sizeof(status.password));
            status.state = connected ? NETWORK_CONNECTED : NETWORK_OFFLINE;
        }
        status.connected = connected; status.day = local_day();
        xQueueOverwrite(s_status, &status);
        vTaskDelay(pdMS_TO_TICKS(250));
    }
failed:
    ESP_LOGW(TAG, "Network unavailable: %s", esp_err_to_name(err));
    if (s_http) { httpd_stop(s_http); s_http = NULL; }
    if (sntp_ready) esp_netif_sntp_deinit();
    if (wifi_handler) esp_event_handler_instance_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, wifi_handler);
    if (ip_handler) esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, ip_handler);
    if (wifi_ready) { esp_wifi_stop(); esp_wifi_deinit(); }
    if (sta) esp_netif_destroy_default_wifi(sta);
    if (ap) esp_netif_destroy_default_wifi(ap);
    status.state = NETWORK_ERROR;
    xQueueOverwrite(s_status, &status);
    vTaskDelete(NULL);
}
bool passport_network_start(void) {
    if (s_status) return false;
    s_status = xQueueCreate(1, sizeof(passport_network_status_t));
    s_credentials = xQueueCreate(1, sizeof(passport_wifi_credentials_t));
    if (s_status && s_credentials && xTaskCreate(worker, "cottage_network", 5120, NULL, 2, NULL) == pdPASS) return true;
    if (s_status) vQueueDelete(s_status);
    if (s_credentials) vQueueDelete(s_credentials);
    s_status = s_credentials = NULL;
    return false;
}
void passport_network_setup(void) { atomic_fetch_or(&s_commands, COMMAND_SETUP); }
void passport_network_forget(void) { atomic_fetch_or(&s_commands, COMMAND_FORGET); }
bool passport_network_poll(passport_network_status_t *status) {
    return s_status && xQueueReceive(s_status, status, 0) == pdTRUE;
}
