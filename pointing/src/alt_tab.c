/* SPDX-License-Identifier: MIT */
#define DT_DRV_COMPAT kobito_alt_tab

#include <zephyr/kernel.h>
#include <drivers/behavior.h>
#include <dt-bindings/zmk/keys.h>
#include <zmk/behavior.h>
#include <zmk/events/keycode_state_changed.h>
#include "alt_tab_policy.h"

static struct alt_tab_policy policy;
K_MUTEX_DEFINE(alt_tab_lock);

static void tap_tab(bool backward, int64_t timestamp) {
    uint32_t code = backward ? LS(TAB) : TAB;
    raise_zmk_keycode_state_changed_from_encoded(code, true, timestamp);
    raise_zmk_keycode_state_changed_from_encoded(code, false, timestamp);
}

static int pressed(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    k_mutex_lock(&alt_tab_lock, K_FOREVER);
    if (alt_tab_begin(&policy)) {
        raise_zmk_keycode_state_changed_from_encoded(LALT, true, event.timestamp);
    }
    tap_tab(false, event.timestamp);
    k_mutex_unlock(&alt_tab_lock);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int released(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    k_mutex_lock(&alt_tab_lock, K_FOREVER);
    if (alt_tab_end(&policy)) {
        raise_zmk_keycode_state_changed_from_encoded(LALT, false, event.timestamp);
    }
    k_mutex_unlock(&alt_tab_lock);
    return ZMK_BEHAVIOR_OPAQUE;
}

/* Run before hid_listener, after combo/hold-tap resolution. Do not leak F/S
 * letters or an unmatched key-up when a navigation key outlives the combo. */
static int navigate(const zmk_event_t *eh) {
    const struct zmk_keycode_state_changed *ev = as_zmk_keycode_state_changed(eh);
    if (!ev || ev->usage_page != HID_USAGE_KEY ||
        (ev->keycode != ZMK_HID_USAGE_ID(F) && ev->keycode != ZMK_HID_USAGE_ID(S))) {
        return ZMK_EV_EVENT_BUBBLE;
    }
    k_mutex_lock(&alt_tab_lock, K_FOREVER);
    int action = alt_tab_navigate(&policy, ev->keycode == ZMK_HID_USAGE_ID(F) ? 0 : 1, ev->state);
    if (action >= 2) {
        tap_tab(action == 3, ev->timestamp);
    }
    k_mutex_unlock(&alt_tab_lock);
    return action ? ZMK_EV_EVENT_HANDLED : ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(kobito_alt_tab, navigate);
ZMK_SUBSCRIPTION(kobito_alt_tab, zmk_keycode_state_changed);

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
static const struct behavior_parameter_metadata metadata = {.sets_len = 0, .sets = NULL};
#endif
static const struct behavior_driver_api alt_tab_api = {
    .binding_pressed = pressed,
    .binding_released = released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .parameter_metadata = &metadata,
#endif
};
BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL,
                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &alt_tab_api);
