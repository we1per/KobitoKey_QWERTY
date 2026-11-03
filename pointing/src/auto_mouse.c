/* SPDX-License-Identifier: MIT */
#define DT_DRV_COMPAT kobito_auto_mouse

#include <zephyr/kernel.h>
#include <drivers/input_processor.h>
#include <zmk/behavior.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/keymap.h>
#include <zmk/matrix.h>

#include "auto_mouse_policy.h"
#include "settings.h"

#define AUTO_NODE DT_INST(0, kobito_auto_mouse)
#define AUTO_LAYER DT_PROP(AUTO_NODE, layer)
BUILD_ASSERT(DT_NUM_INST_STATUS_OKAY(kobito_auto_mouse) == 1);
BUILD_ASSERT(ZMK_KEYMAP_LEN <= 64);

KOBITO_SETTING_INT(motion_timeout, "auto_mouse/motion_timeout_ms", 5000, 1, 60000);
KOBITO_SETTING_INT(click_timeout, "auto_mouse/click_timeout_ms", 800, 1, 60000);
KOBITO_SETTING_INT(prior_idle, "auto_mouse/prior_idle_ms", 350, 0, 10000);
ZMK_CUSTOM_SETTING_DEFINE(
    auto_enabled, "cormoran_custom_settings", "auto_mouse/enabled",
    ZMK_CUSTOM_SETTING_VALUE_TYPE_BOOL, ZMK_CUSTOM_SETTING_VALUE_BOOL(true),
    ZMK_CUSTOM_SETTING_CONFIDENTIALITY_RPC_PUBLIC, ZMK_CUSTOM_SETTING_PERMISSION_UNSECURE,
    ZMK_CUSTOM_SETTING_PERMISSION_SECURE, ZMK_CUSTOM_SETTING_NO_CONSTRAINT);

static struct auto_mouse_policy policy;
static bool owned_layer;
static bool enabled = true;
static int32_t motion_ms = 5000, click_ms = 800, idle_ms = 350;
static const uint32_t excluded[] = DT_PROP(AUTO_NODE, excluded_positions);
K_MUTEX_DEFINE(auto_lock);

static void timeout_handler(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(timeout_work, timeout_handler);

/* Called under auto_lock. Set ownership before raising nested layer events. */
static void synchronize_layer(void) {
    if (policy.active && !owned_layer) {
        if (zmk_keymap_layer_active(AUTO_LAYER)) {
            /* A manually enabled layer belongs to its toggle/hold key. */
            policy.active = false;
        } else {
            owned_layer = true;
            zmk_keymap_layer_activate(AUTO_LAYER, false);
        }
    } else if (!policy.active && owned_layer) {
        owned_layer = false;
        zmk_keymap_layer_deactivate(AUTO_LAYER, false);
    }

    if (policy.active && !policy.held_buttons) {
        k_work_reschedule(&timeout_work, K_MSEC(MAX(1, policy.deadline - k_uptime_get())));
    } else {
        k_work_cancel_delayable(&timeout_work);
    }
}

static void layer_work_handler(struct k_work *work) {
    k_mutex_lock(&auto_lock, K_FOREVER);
    synchronize_layer();
    k_mutex_unlock(&auto_lock);
}
static K_WORK_DEFINE(layer_work, layer_work_handler);

static void timeout_handler(struct k_work *work) {
    k_mutex_lock(&auto_lock, K_FOREVER);
    if (auto_mouse_expired(&policy, k_uptime_get())) {
        policy.active = false;
    }
    synchronize_layer();
    k_mutex_unlock(&auto_lock);
}

static bool position_excluded(uint32_t position) {
    for (size_t i = 0; i < ARRAY_SIZE(excluded); i++) {
        if (excluded[i] == position) {
            return true;
        }
    }
    return false;
}

/* Resolve live Studio bindings too, so relocating &mkp keeps working. */
static const char *binding_at(uint32_t position) {
    for (int i = zmk_keymap_highest_layer_active(); i >= 0; i--) {
        zmk_keymap_layer_id_t layer = zmk_keymap_layer_index_to_id(i);
        if (!zmk_keymap_layer_active(layer)) {
            continue;
        }
        const struct zmk_behavior_binding *binding =
            zmk_keymap_get_layer_binding_at_idx(layer, position);
        if (!binding || !binding->behavior_dev ||
            !strcmp(binding->behavior_dev, DEVICE_DT_NAME(DT_NODELABEL(trans)))) {
            continue;
        }
        return binding->behavior_dev;
    }
    return NULL;
}

static int position_changed(const struct zmk_position_state_changed *ev) {
    if (ev->position >= ZMK_KEYMAP_LEN) {
        return ZMK_EV_EVENT_BUBBLE;
    }
    k_mutex_lock(&auto_lock, K_FOREVER);
    if (!ev->state) {
        if (policy.held_buttons & (UINT64_C(1) << ev->position)) {
            auto_mouse_button(&policy, ev->position, false, k_uptime_get(), click_ms);
            synchronize_layer();
        }
    } else {
        const char *behavior = binding_at(ev->position);
        bool mouse = behavior && !strcmp(behavior, DEVICE_DT_NAME(DT_NODELABEL(mkp)));
        bool precision = behavior && !strcmp(behavior, DEVICE_DT_NAME(DT_NODELABEL(precision)));
        if (mouse) {
            auto_mouse_button(&policy, ev->position, true, k_uptime_get(), click_ms);
        } else {
            auto_mouse_key(&policy, k_uptime_get(), position_excluded(ev->position) || precision);
        }
        /* Must happen before the keymap consumes this same press (also for &none). */
        synchronize_layer();
    }
    k_mutex_unlock(&auto_lock);
    return ZMK_EV_EVENT_BUBBLE;
}

static void reload_settings(void) {
    /* Read before auto_lock: custom-settings notifications may hold its own lock. */
    int32_t new_motion = kobito_setting_int(&motion_timeout, 5000);
    int32_t new_click = kobito_setting_int(&click_timeout, 800);
    int32_t new_idle = kobito_setting_int(&prior_idle, 350);
    struct zmk_custom_setting_value value;
    bool new_enabled = zmk_custom_setting_read(&auto_enabled, &value) == 0 ? value.bool_value : true;
    k_mutex_lock(&auto_lock, K_FOREVER);
    motion_ms = new_motion;
    click_ms = new_click;
    idle_ms = new_idle;
    enabled = new_enabled;
    if (!enabled) {
        policy.active = false;
    }
    synchronize_layer();
    k_mutex_unlock(&auto_lock);
}

static int event_handler(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *position = as_zmk_position_state_changed(eh);
    if (position) {
        return position_changed(position);
    }
    if (as_zmk_custom_settings_initialized(eh) || as_zmk_custom_setting_changed(eh)) {
        reload_settings();
    } else if (as_zmk_layer_state_changed(eh)) {
        k_mutex_lock(&auto_lock, K_FOREVER);
        if (owned_layer && !zmk_keymap_layer_active(AUTO_LAYER)) {
            owned_layer = false;
            policy.active = false;
            k_work_cancel_delayable(&timeout_work);
        }
        k_mutex_unlock(&auto_lock);
    } else {
        const struct zmk_keycode_state_changed *key = as_zmk_keycode_state_changed(eh);
        if (key && key->state) {
            k_mutex_lock(&auto_lock, K_FOREVER);
            policy.last_keypress = k_uptime_get();
            policy.has_keypress = true;
            k_mutex_unlock(&auto_lock);
        }
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(kobito_auto_mouse, event_handler);
ZMK_SUBSCRIPTION(kobito_auto_mouse, zmk_position_state_changed);
ZMK_SUBSCRIPTION(kobito_auto_mouse, zmk_keycode_state_changed);
ZMK_SUBSCRIPTION(kobito_auto_mouse, zmk_layer_state_changed);
ZMK_SUBSCRIPTION(kobito_auto_mouse, zmk_custom_settings_initialized);
ZMK_SUBSCRIPTION(kobito_auto_mouse, zmk_custom_setting_changed);

static int process_motion(const struct device *dev, struct input_event *event, uint32_t p1,
                          uint32_t p2, struct zmk_input_processor_state *state) {
    if (event->type == INPUT_EV_REL && event->value != 0 &&
        (event->code == INPUT_REL_X || event->code == INPUT_REL_Y)) {
        k_mutex_lock(&auto_lock, K_FOREVER);
        if (enabled) {
            auto_mouse_motion(&policy, k_uptime_get(), motion_ms, idle_ms);
            k_work_submit(&layer_work);
        }
        k_mutex_unlock(&auto_lock);
    }
    return ZMK_INPUT_PROC_CONTINUE;
}

static const struct zmk_input_processor_driver_api processor_api = {.handle_event = process_motion};
DEVICE_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL,
                      CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &processor_api);
