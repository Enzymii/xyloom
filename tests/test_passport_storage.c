/* Execute the actual NVS worker with injected storage failures, without hardware. */
#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "passport/watering_record.h"
#include "passport/storage.c"

struct test_queue { size_t size; unsigned head, length; uint8_t data[4][128]; };
static jmp_buf stop_worker;
static void (*worker)(void *);
static uint8_t disk[GROWTH_RECORD_SIZE], pending[GROWTH_RECORD_SIZE];
static size_t disk_size = GROWTH_RECORD_SIZE;
static bool exists, initialized;
static esp_err_t init_error, commit_error;
static unsigned writes, commits;
static uint8_t life_disk[LIFE_RECORD_SIZE], life_pending[LIFE_RECORD_SIZE];
static bool life_exists, life_write;

QueueHandle_t xQueueCreate(unsigned capacity, size_t size) {
    (void)capacity;
    assert(size <= 128);
    QueueHandle_t queue = calloc(1, sizeof(*queue));
    assert(queue); queue->size = size; return queue;
}
void vQueueDelete(QueueHandle_t queue) { free(queue); }
BaseType_t xQueueSend(QueueHandle_t queue, const void *data, uint32_t timeout) {
    (void)timeout;
    assert(queue->length < 4);
    memcpy(queue->data[(queue->head + queue->length) % 4], data, queue->size);
    ++queue->length; return pdTRUE;
}
BaseType_t xQueueReceive(QueueHandle_t queue, void *data, uint32_t timeout) {
    if (!queue->length) {
        if (timeout == portMAX_DELAY) longjmp(stop_worker, 1);
        return pdFALSE;
    }
    memcpy(data, queue->data[queue->head], queue->size);
    queue->head = (queue->head + 1) % 4; --queue->length; return pdTRUE;
}
BaseType_t xTaskCreate(void (*entry)(void *), const char *name, unsigned stack,
                      void *arg, unsigned priority, void *handle) {
    (void)name; (void)stack; (void)arg; (void)priority; (void)handle;
    worker = entry; return pdPASS;
}
esp_err_t nvs_flash_init(void) { initialized = true; return init_error; }
esp_err_t nvs_open(const char *space, int mode, nvs_handle_t *handle) {
    assert(initialized && strcmp(space, "cottage") == 0 && mode == NVS_READWRITE);
    *handle = 1; return ESP_OK;
}
esp_err_t nvs_get_blob(nvs_handle_t handle, const char *key, void *data, size_t *size) {
    assert(handle == 1);
    if (!strcmp(key, "life")) {
        if (!life_exists) return ESP_ERR_NVS_NOT_FOUND;
        assert(*size >= sizeof(life_disk)); memcpy(data, life_disk, sizeof(life_disk));
        *size = sizeof(life_disk); return ESP_OK;
    }
    assert(strcmp(key, "watering") == 0);
    if (!exists) return ESP_ERR_NVS_NOT_FOUND;
    assert(*size >= disk_size); memcpy(data, disk, disk_size); *size = disk_size; return ESP_OK;
}
esp_err_t nvs_set_blob(nvs_handle_t handle, const char *key, const void *data, size_t size) {
    assert(handle == 1);
    life_write = !strcmp(key, "life");
    if (life_write) {
        assert(size == sizeof(life_pending)); memcpy(life_pending, data, size); ++writes; return ESP_OK;
    }
    assert(strcmp(key, "watering") == 0 && size == sizeof(pending));
    memcpy(pending, data, size); ++writes; return ESP_OK;
}
esp_err_t nvs_commit(nvs_handle_t handle) {
    assert(handle == 1); ++commits;
    if (commit_error != ESP_OK) return commit_error;
    if (life_write) { memcpy(life_disk, life_pending, sizeof(life_disk)); life_exists = true; return ESP_OK; }
    memcpy(disk, pending, sizeof(disk)); disk_size = sizeof(disk); exists = true; return ESP_OK;
}
void nvs_close(nvs_handle_t handle) { assert(handle == 1); }

static void boot(void) {
    if (s_requests) vQueueDelete(s_requests);
    if (s_results) vQueueDelete(s_results);
    s_requests = s_results = NULL;
    writes = commits = 0; initialized = false;
    assert(passport_storage_start());
    assert(!passport_storage_start());
}
static void run(void) { if (setjmp(stop_worker) == 0) worker(NULL); }
static passport_storage_result_t result(void) {
    passport_storage_result_t r;
    assert(passport_storage_poll(&r)); return r;
}

static bool save(uint32_t total) {
    passport_growth_t previous = {0}, next;
    if (exists) assert(passport_growth_decode(disk, disk_size, &previous));
    if (total == previous.total) next = previous;
    else { assert(passport_growth_drink(&previous, 20261001, &next)); next.total = total; }
    return passport_storage_save(&next, 20261001, true);
}

static void restart_date_worker_tests(void) {
    passport_growth_t saved = {.total = 15, .today = 3, .day = 20261008,
        .growth = 12, .daily = 3, .clock_mode = CLOCK_CALENDAR}, clock;
    passport_growth_encode(disk, &saved); disk[4] = 3; exists = true;
    boot(); run(); passport_storage_result_t r = result();
    assert(r.success && r.record.daily == 3 && r.record.day == 20261008);
    assert(passport_growth_clock(&r.record, 0, 6, &clock));
    commit_error = TEST_STORAGE_FAILURE;
    assert(passport_storage_save(&clock, 0, false)); run(); result(); r = result();
    assert(!r.success && disk[4] == 3);
    commit_error = ESP_OK;
    assert(passport_storage_save(&clock, 0, false)); run(); result(); r = result();
    assert(r.success && r.record.clock_mode == CLOCK_AWAITING && disk[4] == 4);
    boot(); run(); r = result(); /* Waiting state survives another cold boot. */
    assert(r.success && r.record.daily == 3 && r.record.phase_seconds == 6);
    assert(passport_growth_clock(&r.record, 20261009, 0, &clock));
    commit_error = TEST_STORAGE_FAILURE;
    assert(passport_storage_save(&clock, 20261009, false)); run(); result(); r = result();
    assert(!r.success && passport_growth_decode(disk, disk_size, &saved) && saved.daily == 3);
    commit_error = ESP_OK;
    assert(passport_storage_save(&clock, 20261009, false)); run(); result(); r = result();
    assert(r.success && !r.record.daily && r.record.growth == 12 && r.record.total == 15);
    assert(passport_storage_save(&clock, 20261009, false)); run(); result(); r = result();
    assert(r.success && writes == 2); /* Acknowledgement retry writes nothing. */
    assert(passport_growth_drink(&clock, 20261009, &clock));
    assert(passport_storage_save(&clock, 20261009, true)); run(); result(); r = result();
    assert(r.success && r.record.daily == 1 && r.record.growth == 13);
    boot(); run(); r = result(); saved = r.record;
    assert(passport_growth_clock(&saved, 0, 6, &clock));
    assert(passport_storage_save(&clock, 0, false)); run(); result(); r = result();
    assert(r.success);
    assert(passport_growth_clock(&clock, 20261009, 0, &clock));
    assert(passport_storage_save(&clock, 20261009, false)); run(); result(); r = result();
    assert(r.success && r.record.daily == 1 && r.record.total == 16);
    exists = false;
}

int main(void) {
    restart_date_worker_tests();
    boot(); assert(save(1)); run();
    passport_storage_result_t r = result();
    assert(r.loaded && r.success && r.record.total == 0);
    r = result(); assert(!r.loaded && r.success && writes == 1 && commits == 1);
    passport_growth_t count;
    assert(passport_growth_decode(disk, sizeof(disk), &count) && count.total == 1);

    boot(); assert(save(1)); run();
    r = result(); assert(r.loaded && r.record.total == 1);
    r = result(); assert(r.success && writes == 0 && commits == 0); /* Idempotent retry. */

    boot(); assert(save(3)); run();
    result(); r = result(); assert(!r.success && writes == 0); /* Reject skipped counts. */

    commit_error = TEST_STORAGE_FAILURE;
    boot(); assert(save(2)); run();
    result(); r = result(); assert(!r.success);
    assert(passport_growth_decode(disk, sizeof(disk), &count) && count.total == 1);
    commit_error = ESP_OK;
    boot(); assert(save(2)); run();
    result(); r = result(); assert(r.success && writes == 1 && commits == 1);
    assert(passport_growth_decode(disk, sizeof(disk), &count) && count.total == 2);

    passport_watering_encode(disk, 7); disk_size = WATERING_RECORD_SIZE;
    boot(); assert(save(8)); run();
    r = result(); assert(r.loaded && r.record.total == 7 && r.record.growth == 0);
    r = result(); assert(r.success && disk_size == GROWTH_RECORD_SIZE);
    assert(passport_growth_decode(disk, disk_size, &count) && count.total == 8 && count.growth == 1);

    count = (passport_growth_t){.total = 4, .today = 4, .day = 20261001, .growth = 4, .daily = 4};
    passport_growth_encode(disk, &count);
    boot(); assert(save(5)); run();
    r = result(); assert(r.record.daily == 4); r = result(); assert(r.success);
    assert(passport_growth_decode(disk, disk_size, &count) && count.today == 5 && count.growth == 4);

    /* Execute clock-only commits, cold boot and failed writes in the actual worker. */
    count = (passport_growth_t){.total = 5, .today = 5, .growth = 4, .daily = 4,
        .phase_seconds = 86340, .runtime_seconds = 86340};
    passport_growth_encode(disk, &count);
    passport_growth_t clock;
    assert(passport_growth_clock(&count, 0, 60, &clock));
    commit_error = TEST_STORAGE_FAILURE;
    boot(); assert(passport_storage_save(&clock, 0, false)); run();
    result(); r = result(); assert(!r.success);
    assert(passport_growth_decode(disk, disk_size, &count) && count.daily == 4 && count.phase_seconds == 86340);
    commit_error = ESP_OK;
    boot(); assert(passport_storage_save(&clock, 0, false)); run();
    result(); r = result(); assert(r.success && r.record.daily == 0 && r.record.total == 5);
    boot(); run(); r = result();
    assert(r.record.runtime_seconds == 86400 && r.record.daily == 0 && r.record.growth == 4);
    count = r.record;
    assert(passport_growth_drink(&count, 0, &clock));
    assert(passport_storage_save(&clock, 0, true)); run(); result(); r = result();
    assert(r.success && r.record.total == 6 && r.record.growth == 5);
    count = r.record;
    assert(passport_growth_clock(&count, 20261008, 0, &clock));
    assert(passport_storage_save(&clock, 20261008, false)); run(); result(); r = result();
    assert(r.success && r.record.clock_mode == CLOCK_BRIDGED && r.record.daily == 1);
    count = r.record; clock.total++; /* A checkpoint must not smuggle in a cup. */
    assert(passport_storage_save(&clock, 20261008, false)); run(); result(); r = result();
    assert(!r.success && passport_growth_decode(disk, disk_size, &clock) && passport_growth_equal(&clock, &count));

    boot(); assert(passport_storage_save_life(1790784000u)); run();
    r = result(); assert(r.loaded && r.life_ready && !r.life_stamp);
    r = result(); assert(r.life_saved && r.success);
    boot(); run(); r = result(); assert(r.life_stamp == 1790784000u && r.record.total == 6);
    commit_error = TEST_STORAGE_FAILURE;
    assert(passport_storage_save_life(1790784900u)); run(); result(); r = result();
    assert(r.life_saved && !r.success);
    commit_error = ESP_OK;
    boot(); run(); r = result(); assert(r.life_stamp == 1790784000u);
    assert(passport_storage_save_life(1790784900u)); run(); result(); r = result(); assert(r.success);
    life_disk[4] = 2;
    boot(); assert(passport_storage_save_life(1790785800u)); run();
    r = result(); assert(r.success && !r.life_ready && r.record.total == 6);
    r = result(); assert(r.life_saved && !r.success && writes == 0 && life_disk[4] == 2);

    disk[4] = 5; /* Unknown version must not reset or overwrite the record. */
    boot(); passport_growth_t invalid_request = {.total = 1, .today = 1, .day = 20261001, .growth = 1, .daily = 1};
    assert(passport_storage_save(&invalid_request, 20261001, true)); run();
    r = result(); assert(!r.success && r.loaded);
    r = result(); assert(!r.success && writes == 0 && disk[4] == 5);

    init_error = TEST_STORAGE_FAILURE;
    boot();
    assert(passport_storage_save(&invalid_request, 20261001, true)); run();
    r = result(); assert(!r.success && r.loaded);
    r = result(); assert(!r.success && writes == 0);
    puts("Cottage NVS worker fault/retry tests: PASS");
    return 0;
}
