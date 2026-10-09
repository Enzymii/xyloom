#include "view.h"
#include "lvgl.h"
#include "network.h"
#include <string.h>
#include <stdio.h>

LV_FONT_DECLARE(passport_font_18);
LV_IMAGE_DECLARE(passport_world_day);
LV_IMAGE_DECLARE(passport_world_night);
LV_IMAGE_DECLARE(passport_momo_idle);
LV_IMAGE_DECLARE(passport_momo_sleep);
LV_IMAGE_DECLARE(passport_tulip);
LV_IMAGE_DECLARE(passport_tulip_seed);
LV_IMAGE_DECLARE(passport_tulip_sprout);
LV_IMAGE_DECLARE(passport_tulip_bud);
LV_IMAGE_DECLARE(passport_can);

static lv_obj_t *s_world, *s_background, *s_momo, *s_focus, *s_dialog, *s_text, *s_label, *s_battery;
static lv_obj_t *s_can, *s_drops[4];
static lv_obj_t *s_plant, *s_wifi, *s_daily[4];
static bool s_daily_filled[4];
static bool s_rendered;
static passport_world_t s_previous;
static passport_dialog_t s_last_dialog = (passport_dialog_t)-1;

static lv_obj_t *shape(lv_obj_t *parent, int x, int y, int width, int height, uint32_t color, int radius) {
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, width, height);
    lv_obj_set_style_bg_color(obj, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(obj, radius, 0);
    return obj;
}

static lv_obj_t *label(lv_obj_t *parent, const char *text, int x, int y) {
    lv_obj_t *obj = lv_label_create(parent);
    lv_obj_set_style_text_font(obj, &passport_font_18, 0);
    lv_obj_set_style_text_color(obj, lv_color_hex(0xFFF7E9), 0);
    lv_label_set_text(obj, text);
    lv_obj_set_pos(obj, x, y);
    return obj;
}

static void draw_daily_drop(lv_event_t *event) {
    lv_obj_t *obj = lv_event_get_target(event);
    bool filled = *(bool *)lv_event_get_user_data(event);
    lv_area_t area; lv_obj_get_coords(obj, &area);
    lv_layer_t *layer = lv_event_get_layer(event);
    for (unsigned inset = 0; inset < 2; ++inset) {
        lv_color_t color = lv_color_hex(inset ? (filled ? 0x279ACB : 0xFFF5E4) : (filled ? 0x246889 : 0x8FA3AB));
        lv_draw_triangle_dsc_t triangle; lv_draw_triangle_dsc_init(&triangle);
        triangle.color = color;
        triangle.p[0] = (lv_point_precise_t){area.x1 + 5, area.y1 + (inset ? 3 : 0)};
        triangle.p[1] = (lv_point_precise_t){area.x1 + (inset ? 2 : 0), area.y1 + 7};
        triangle.p[2] = (lv_point_precise_t){area.x1 + (inset ? 7 : 9), area.y1 + 7};
        lv_draw_triangle(layer, &triangle);
        lv_draw_rect_dsc_t body; lv_draw_rect_dsc_init(&body);
        body.bg_color = color; body.radius = LV_RADIUS_CIRCLE;
        lv_area_t circle = {area.x1 + (inset ? 1 : 0), area.y1 + (inset ? 5 : 4),
                            area.x1 + (inset ? 8 : 9), area.y1 + (inset ? 12 : 13)};
        lv_draw_rect(layer, &body, &circle);
    }
}

void passport_view_create(void) {
    memset(s_daily_filled, 0, sizeof(s_daily_filled));
    s_rendered = false;
    s_last_dialog = (passport_dialog_t)-1;
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_remove_style_all(screen);
    lv_obj_set_size(screen, VIEW_WIDTH, VIEW_HEIGHT);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x352C2B), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    s_world = lv_obj_create(screen);
    lv_obj_remove_style_all(s_world);
    lv_obj_set_size(s_world, WORLD_WIDTH, VIEW_HEIGHT);
    lv_obj_remove_flag(s_world, LV_OBJ_FLAG_SCROLLABLE);
    s_background = lv_image_create(s_world);
    lv_image_set_src(s_background, &passport_world_day);
    /* A quiet ground marker keeps selection inside the illustrated world. */
    s_focus = shape(s_world, 0, 0, 32, 7, 0xFFE5A5, LV_RADIUS_CIRCLE);
    lv_obj_set_style_bg_opa(s_focus, LV_OPA_50, 0);
    lv_obj_set_style_border_color(s_focus, lv_color_hex(0xFFF1C7), 0);
    lv_obj_set_style_border_width(s_focus, 1, 0);
    s_plant = lv_image_create(s_world);
    lv_image_set_src(s_plant, &passport_tulip_seed);
    lv_obj_set_pos(s_plant, 51, 204);
    s_can = lv_image_create(s_world);
    lv_image_set_src(s_can, &passport_can);
    lv_image_set_pivot(s_can, 40, 22);
    lv_obj_set_pos(s_can, 140, 225);
    for (unsigned i = 0; i < 4; ++i) {
        s_drops[i] = shape(s_world, 0, 0, 4, 8, 0x44BFFF, LV_RADIUS_CIRCLE);
        lv_obj_add_flag(s_drops[i], LV_OBJ_FLAG_HIDDEN);
    }
    s_momo = lv_image_create(s_world);
    lv_image_set_src(s_momo, &passport_momo_idle);
    lv_obj_set_pos(s_momo, HOUSE_X + 45, 85);
    s_label = label(screen, "", 24, 25);
    lv_obj_set_style_text_color(s_label, lv_color_hex(0xFFF3DA), 0);
    s_battery = label(screen, "--%", 174, 25);
    s_wifi = lv_label_create(screen);
    lv_obj_set_style_text_font(s_wifi, &lv_font_montserrat_14, 0);
    lv_label_set_text(s_wifi, LV_SYMBOL_WIFI);
    lv_obj_set_pos(s_wifi, 151, 28);
    for (unsigned i = 0; i < 4; ++i) {
        s_daily[i] = lv_obj_create(screen);
        lv_obj_remove_style_all(s_daily[i]);
        lv_obj_set_pos(s_daily[i], 88 + 14 * i, 28);
        lv_obj_set_size(s_daily[i], 10, 14);
        lv_obj_add_event_cb(s_daily[i], draw_daily_drop, LV_EVENT_DRAW_MAIN, &s_daily_filled[i]);
    }
    s_dialog = lv_obj_create(screen);
    lv_obj_remove_style_all(s_dialog);
    lv_obj_set_pos(s_dialog, 16, 230);
    lv_obj_set_size(s_dialog, 208, 72);
    lv_obj_set_style_bg_color(s_dialog, lv_color_hex(0xFFE2EB), 0);
    lv_obj_set_style_bg_opa(s_dialog, LV_OPA_90, 0);
    lv_obj_set_style_radius(s_dialog, 12, 0);
    s_text = label(s_dialog, "", 12, 10);
    lv_obj_set_width(s_text, 184);
    lv_obj_set_style_text_color(s_text, lv_color_hex(0x553E48), 0);
    lv_obj_add_flag(s_dialog, LV_OBJ_FLAG_HIDDEN);
    lv_screen_load(screen);
}

/* Only visible state participates: input timestamps and queued commands must
 * not continuously invalidate a stationary screen and starve the idle task. */
static bool same_view(const passport_world_t *a, const passport_world_t *b) {
    return a->scene == b->scene && a->camera_x == b->camera_x &&
        a->transitioning == b->transitioning && a->momo == b->momo && a->night == b->night &&
        a->focus[0] == b->focus[0] && a->focus[1] == b->focus[1] &&
        a->dialog == b->dialog && a->storage_state == b->storage_state &&
        passport_growth_stage(a->record.growth) == passport_growth_stage(b->record.growth) &&
        a->record.daily == b->record.daily && a->record.day == b->record.day &&
        a->record.clock_mode == b->record.clock_mode && a->day == b->day &&
        (a->dialog != DIALOG_STATUS || (a->day == b->day && a->record.day == b->record.day)) &&
        a->wifi_connected == b->wifi_connected &&
        a->network_state == b->network_state && a->status_focus == b->status_focus &&
        !strcmp(a->setup_password, b->setup_password) &&
        a->battery_percent == b->battery_percent &&
        (a->dialog != DIALOG_STATUS || a->uptime_minutes == b->uptime_minutes) &&
        a->watering == b->watering &&
        (!a->watering || a->watering_elapsed == b->watering_elapsed) &&
        (a->water_save_pending && !a->save_is_clock) == (b->water_save_pending && !b->save_is_clock) &&
        (a->water_save_inflight && !a->save_is_clock) == (b->water_save_inflight && !b->save_is_clock);
}

void passport_view_render(const passport_world_t *w) {
    if (s_rendered && same_view(w, &s_previous)) return;
    s_previous = *w;
    s_rendered = true;
    const lv_image_dsc_t *background = w->night ? &passport_world_night : &passport_world_day;
    if (lv_image_get_src(s_background) != background) lv_image_set_src(s_background, background);
    const lv_image_dsc_t *momo = w->momo == HOME_SLEEP ? &passport_momo_sleep : &passport_momo_idle;
    if (lv_image_get_src(s_momo) != momo) lv_image_set_src(s_momo, momo);
    static const lv_image_dsc_t *const stages[] = {&passport_tulip_seed, &passport_tulip_sprout, &passport_tulip_bud, &passport_tulip};
    unsigned stage = passport_growth_stage(w->record.growth);
    if (lv_image_get_src(s_plant) != stages[stage]) lv_image_set_src(s_plant, stages[stage]);
    lv_obj_set_style_text_color(s_wifi, lv_color_hex(w->wifi_connected ? 0x5D9565 : 0xA99883), 0);
    lv_obj_set_style_text_opa(s_wifi, w->wifi_connected ? LV_OPA_COVER : LV_OPA_50, 0);
    bool date_ready = passport_day_valid(w->day) && w->day >= w->record.day;
    bool calendar = date_ready && w->record.clock_mode == CLOCK_CALENDAR;
    unsigned daily = calendar && w->day != w->record.day ? 0 : w->record.daily;
    bool garden = w->scene == SCENE_GARDEN && !w->transitioning;
    for (unsigned i = 0; i < 4; ++i) {
        if (garden) lv_obj_remove_flag(s_daily[i], LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(s_daily[i], LV_OBJ_FLAG_HIDDEN);
        bool filled = w->storage_state == STORAGE_READY && i < daily;
        if (filled != s_daily_filled[i]) { s_daily_filled[i] = filled; lv_obj_invalidate(s_daily[i]); }
    }
    lv_obj_set_x(s_world, -w->camera_x);
    passport_water_pose_t pose = passport_water_pose(w->watering ? w->watering_elapsed : WATERING_MS);
    lv_obj_set_pos(s_can, pose.x, pose.y);
    lv_image_set_rotation(s_can, pose.rotation ? 3600 - pose.rotation : 0);
    for (unsigned i = 0; i < 4; ++i) {
        if (w->watering && pose.pouring) {
            uint32_t fall = (pose.pour_elapsed + i * 100) % 400;
            lv_obj_set_pos(s_drops[i], 79 + (int)(i % 2) * 3 - (int)(fall * 4 / 400),
                           218 + (int)(fall * 43 / 400));
            lv_obj_remove_flag(s_drops[i], LV_OBJ_FLAG_HIDDEN);
        } else lv_obj_add_flag(s_drops[i], LV_OBJ_FLAG_HIDDEN);
    }
    if (w->momo == OUT) lv_obj_add_flag(s_momo, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_remove_flag(s_momo, LV_OBJ_FLAG_HIDDEN);
    const bool house = w->scene == SCENE_HOUSE;
    unsigned focus = w->focus[w->scene];
    const char *name = house ? (focus ? "状态" : w->momo == OUT ? "出门了" :
        w->momo == HOME_SLEEP ? "睡着了" : "沫纯") : (focus ? "水壶" : "郁金香");
    if (w->watering) name = "浇水中";
    else if (!w->save_is_clock && (w->water_save_pending || w->water_save_inflight)) name = "保存中";
    if (w->transitioning) {
        lv_obj_add_flag(s_focus, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_label, LV_OBJ_FLAG_HIDDEN);
    } else {
        if (w->dialog == DIALOG_NONE && !passport_world_busy(w) && !(house && focus) &&
            !(house && w->momo == OUT)) lv_obj_remove_flag(s_focus, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(s_focus, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(s_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_text_decor(s_label, house && focus ? LV_TEXT_DECOR_UNDERLINE : LV_TEXT_DECOR_NONE, 0);
        /* Ground anchors follow feet / pot / can, never enclose the artwork. */
        lv_obj_set_pos(s_focus, house ? HOUSE_X + (focus ? 32 : 103) : (focus ? 154 : 64),
                       house ? (focus ? 269 : 282) : 265);
        lv_obj_set_size(s_focus, house && !focus ? 34 : 28, 7);
        if (strcmp(lv_label_get_text(s_label), name) != 0) lv_label_set_text(s_label, name);
    }
    /* Light sky needs a quiet backing for readable labels on the garden side. */
    lv_obj_t *headers[] = {s_label, s_battery};
    for (unsigned i = 0; i < 2; ++i) {
        lv_obj_set_style_text_color(headers[i], lv_color_hex(house || w->night ? 0xFFF3DA : 0x493C2D), 0);
        lv_obj_set_style_bg_color(headers[i], lv_color_hex(0xFFF1D9), 0);
        lv_obj_set_style_bg_opa(headers[i], house || w->night ? LV_OPA_TRANSP : LV_OPA_70, 0);
        lv_obj_set_style_radius(headers[i], 4, 0);
    }
    char battery[12];
    if (w->battery_percent >= 0 && w->battery_percent <= 100)
        snprintf(battery, sizeof(battery), "%d%%", w->battery_percent);
    else snprintf(battery, sizeof(battery), "--%%");
    if (strcmp(lv_label_get_text(s_battery), battery) != 0) lv_label_set_text(s_battery, battery);
    if (w->dialog == DIALOG_STATUS) {
        char text[512], date[32];
        if (date_ready) snprintf(date, sizeof(date), "%lu-%02lu-%02lu", (unsigned long)(w->day / 10000), (unsigned long)(w->day / 100 % 100), (unsigned long)(w->day % 100));
        else snprintf(date, sizeof(date), "离线计时");
        const char *mood = w->momo == OUT ? "出门了" : w->momo == HOME_SLEEP ? "睡着了" : "在家悠闲";
        const char *net = w->wifi_connected ? "已连接" : w->network_state == NETWORK_CONNECTING ? "连接中" : w->network_state == NETWORK_SETUP ? "配网中" : w->network_state == NETWORK_ERROR ? "连接失败" : "未连接";
        if (w->network_state == NETWORK_SETUP) {
            snprintf(text, sizeof(text), "连接热点\nXyloom-Setup\n密码 %s\n\n手机浏览器打开\n192.168.4.1\n选择家中 Wi-Fi\n\n%s返回  %s设置  %s清除", w->setup_password,
                w->status_focus == 0 ? ">" : "", w->status_focus == 1 ? ">" : "", w->status_focus == 2 ? ">" : "");
        } else {
            snprintf(text, sizeof(text), "设备状态\nWi-Fi  %s\n日期  %s\n电量  %s\n运行  %lu小时%lu分\n沫纯  %s\n\n%s返回\n%s设置 Wi-Fi\n%s清除 Wi-Fi", net, date, battery,
                 (unsigned long)(w->uptime_minutes / 60), (unsigned long)(w->uptime_minutes % 60),
                 mood,
                 w->status_focus == 0 ? "> " : "  ", w->status_focus == 1 ? "> " : "  ", w->status_focus == 2 ? "> " : "  ");
        }
        if (strcmp(lv_label_get_text(s_text), text) != 0) lv_label_set_text(s_text, text);
        lv_obj_set_pos(s_dialog, 16, 48);
        lv_obj_set_size(s_dialog, 208, 254);
        lv_obj_set_style_bg_color(s_dialog, lv_color_hex(0xFFF1D9), 0);
        lv_obj_set_style_bg_opa(s_dialog, LV_OPA_COVER, 0);
        lv_obj_remove_flag(s_dialog, LV_OBJ_FLAG_HIDDEN);
        s_last_dialog = w->dialog;
        return;
    }
    /* Plant descriptions follow visible stage changes while the dialog stays open. */
    if (w->dialog == s_last_dialog && w->dialog != DIALOG_TULIP && w->dialog != DIALOG_WATER_DONE) return;
    lv_obj_set_pos(s_dialog, 16, 230);
    lv_obj_set_size(s_dialog, 208, 72);
    lv_obj_set_style_bg_color(s_dialog, lv_color_hex(0xFFE2EB), 0);
    lv_obj_set_style_bg_opa(s_dialog, LV_OPA_90, 0);
    s_last_dialog = w->dialog;
    if (w->dialog == DIALOG_NONE) {
        lv_obj_add_flag(s_dialog, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    if (w->dialog == DIALOG_TULIP || w->dialog == DIALOG_WATER_DONE) {
        char text[256];
        static const char *const descriptions[] = {
            "种子在土里，\n慢慢等它长大。",
            "小小的幼苗，\n正在慢慢长大。",
            "花苞在慢慢长大，\n等它开花。",
            "橙色郁金香开花了。"
        };
        if (w->dialog == DIALOG_WATER_DONE) {
            snprintf(text, sizeof(text), "浇水完成啦～");
        } else {
            snprintf(text, sizeof(text), "橙色郁金香\n%s", descriptions[stage]);
            lv_obj_set_pos(s_dialog, 16, 208); lv_obj_set_size(s_dialog, 208, 94);
            if (w->storage_state != STORAGE_READY)
                snprintf(text, sizeof(text), "橙色郁金香\n%s", w->storage_state == STORAGE_LOADING ?
                         "记录读取中" : "记录不可用");
        }
        lv_label_set_text(s_text, text);
    } else {
        static const char *const messages[] = { "", "主人回来啦～", "",
            "沫纯出门了。", "", "选中水壶按 OK 浇水", "",
            "保存失败\n按 OK 重试", "正在读取记录", "存储不可用\n请检查后重启", "浇水记录已满",
            "慢慢陪它长大。", "清除 Wi-Fi？\nOK 确认  左右取消", "沫纯睡着了。\n晚安，主人。" };
        lv_label_set_text(s_text, messages[w->dialog]);
    }
    lv_obj_remove_flag(s_dialog, LV_OBJ_FLAG_HIDDEN);
}
