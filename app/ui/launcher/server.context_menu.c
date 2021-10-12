#include <backend/types.h>
#include <app.h>
#include "server.context_menu.h"

typedef struct context_menu_t {
    lv_obj_controller_t base;
    PSERVER_LIST node;
} context_menu_t;

static void menu_ctor(lv_obj_controller_t *self, void *arg);

static lv_obj_t *create_obj(lv_obj_controller_t *self, lv_obj_t *parent);

static void context_menu_key_cb(lv_event_t *e);

static void context_menu_click_cb(lv_event_t *e);

const lv_obj_controller_class_t server_menu_class = {
        .constructor_cb = menu_ctor,
        .create_obj_cb = create_obj,
        .instance_size = sizeof(context_menu_t)
};

static void menu_ctor(lv_obj_controller_t *self, void *arg) {
    context_menu_t *controller = (context_menu_t *) self;
    controller->node = arg;
}

static lv_obj_t *create_obj(lv_obj_controller_t *self, lv_obj_t *parent) {
    context_menu_t *controller = (context_menu_t *) self;
    lv_obj_t *msgbox = lv_msgbox_create(NULL, controller->node->server->hostname, NULL, NULL, false);
    lv_obj_t *content = lv_msgbox_get_content(msgbox);
    lv_obj_add_flag(content, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);

    lv_obj_add_event_cb(content, context_menu_key_cb, LV_EVENT_KEY, controller);
    lv_obj_add_event_cb(content, context_menu_click_cb, LV_EVENT_SHORT_CLICKED, controller);

//    lv_obj_set_user_data(resume_btn, launcher_launch_game);

    lv_obj_t *quit_btn = lv_list_add_btn(content, NULL, "Test Connection");
    lv_obj_add_flag(quit_btn, LV_OBJ_FLAG_EVENT_BUBBLE);
//    lv_obj_set_user_data(quit_btn, launcher_quit_game);


    lv_obj_t *unpair_btn = lv_list_add_btn(content, NULL, "Unpair");
    lv_obj_add_flag(unpair_btn, LV_OBJ_FLAG_EVENT_BUBBLE);
//    lv_obj_set_user_data(unpair_btn, launcher_quit_game);

    lv_obj_t *cancel_btn = lv_list_add_btn(content, NULL, "Cancel");
    lv_obj_add_flag(cancel_btn, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_center(msgbox);
    return msgbox;
}

static void context_menu_key_cb(lv_event_t *e) {
    lv_obj_t *target = lv_event_get_target(e);
    if (target->parent != lv_event_get_current_target(e)) return;
    if (lv_event_get_key(e) == LV_KEY_ESC) {
        lv_msgbox_close_async(lv_event_get_current_target(e)->parent);
    }
}

static void context_menu_click_cb(lv_event_t *e) {
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t *current_target = lv_event_get_current_target(e);
    if (target->parent != current_target) return;
    lv_obj_t *mbox = lv_event_get_current_target(e)->parent;
    lv_msgbox_close_async(mbox);
}