#include "view.h"
#include "lvgl.h"
#include <string.h>
#include <stdio.h>

LV_FONT_DECLARE(passport_font_18);
LV_IMAGE_DECLARE(passport_world_day);
LV_IMAGE_DECLARE(passport_momo_idle);
LV_IMAGE_DECLARE(passport_tulip);
LV_IMAGE_DECLARE(passport_can);

static lv_obj_t *s_world, *s_momo, *s_focus, *s_dialog, *s_text, *s_label, *s_battery;
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

void passport_view_create(void) {
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
    lv_obj_t *background = lv_image_create(s_world);
    lv_image_set_src(background, &passport_world_day);
    /* A quiet ground marker keeps selection inside the illustrated world. */
    s_focus = shape(s_world, 0, 0, 32, 7, 0xFFE5A5, LV_RADIUS_CIRCLE);
    lv_obj_set_style_bg_opa(s_focus, LV_OPA_50, 0);
    lv_obj_set_style_border_color(s_focus, lv_color_hex(0xFFF1C7), 0);
    lv_obj_set_style_border_width(s_focus, 1, 0);
    lv_obj_t *plant = lv_image_create(s_world);
    lv_image_set_src(plant, &passport_tulip);
    lv_obj_set_pos(plant, 51, 204);
    lv_obj_t *can = lv_image_create(s_world);
    lv_image_set_src(can, &passport_can);
    lv_obj_set_pos(can, 140, 225);
    s_momo = lv_image_create(s_world);
    lv_image_set_src(s_momo, &passport_momo_idle);
    lv_obj_set_pos(s_momo, HOUSE_X + 45, 85);
    s_label = label(screen, "", 24, 25);
    lv_obj_set_style_text_color(s_label, lv_color_hex(0xFFF3DA), 0);
    s_battery = label(screen, "--%", 174, 25);
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

void passport_view_render(const passport_world_t *w) {
    lv_obj_set_x(s_world, -w->camera_x);
    if (w->momo == OUT) lv_obj_add_flag(s_momo, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_remove_flag(s_momo, LV_OBJ_FLAG_HIDDEN);
    const bool house = w->scene == SCENE_HOUSE;
    unsigned focus = w->focus[w->scene];
    const char *name = house ? (focus ? "状态" : "沫纯") : (focus ? "水壶" : "郁金香");
    if (w->transitioning) {
        lv_obj_add_flag(s_focus, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_label, LV_OBJ_FLAG_HIDDEN);
    } else {
        if (w->dialog == DIALOG_NONE && !(house && focus)) lv_obj_remove_flag(s_focus, LV_OBJ_FLAG_HIDDEN);
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
        lv_obj_set_style_text_color(headers[i], lv_color_hex(house ? 0xFFF3DA : 0x493C2D), 0);
        lv_obj_set_style_bg_color(headers[i], lv_color_hex(0xFFF1D9), 0);
        lv_obj_set_style_bg_opa(headers[i], house ? LV_OPA_TRANSP : LV_OPA_70, 0);
        lv_obj_set_style_radius(headers[i], 4, 0);
    }
    char battery[12];
    if (w->battery_percent >= 0 && w->battery_percent <= 100)
        snprintf(battery, sizeof(battery), "%d%%", w->battery_percent);
    else snprintf(battery, sizeof(battery), "--%%");
    if (strcmp(lv_label_get_text(s_battery), battery) != 0) lv_label_set_text(s_battery, battery);
    if (w->dialog == DIALOG_STATUS) {
        char text[256];
        const char *mood = w->momo == OUT ? "出门了" : w->momo == HOME_SLEEP ? "睡着了" : "在家悠闲";
        snprintf(text, sizeof(text), "设备与沫纯\n\n电量  %s\n运行  %lu小时%lu分\n沫纯  %s\n\n按 OK 返回", battery,
                 (unsigned long)(w->uptime_minutes / 60),
                 (unsigned long)(w->uptime_minutes % 60), mood);
        if (strcmp(lv_label_get_text(s_text), text) != 0) lv_label_set_text(s_text, text);
        lv_obj_set_pos(s_dialog, 16, 67);
        lv_obj_set_size(s_dialog, 208, 235);
        lv_obj_set_style_bg_color(s_dialog, lv_color_hex(0xFFF1D9), 0);
        lv_obj_set_style_bg_opa(s_dialog, LV_OPA_COVER, 0);
        lv_obj_remove_flag(s_dialog, LV_OBJ_FLAG_HIDDEN);
        s_last_dialog = w->dialog;
        return;
    }
    if (w->dialog == s_last_dialog) return;
    lv_obj_set_pos(s_dialog, 16, 230);
    lv_obj_set_size(s_dialog, 208, 72);
    lv_obj_set_style_bg_color(s_dialog, lv_color_hex(0xFFE2EB), 0);
    lv_obj_set_style_bg_opa(s_dialog, LV_OPA_90, 0);
    s_last_dialog = w->dialog;
    if (w->dialog == DIALOG_NONE) {
        lv_obj_add_flag(s_dialog, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    static const char *const messages[] = { "", "主人回来啦～", "",
        "沫纯出门了。", "橙色郁金香", "喝水将在后续阶段开放" };
    lv_label_set_text(s_text, messages[w->dialog]);
    lv_obj_remove_flag(s_dialog, LV_OBJ_FLAG_HIDDEN);
}
