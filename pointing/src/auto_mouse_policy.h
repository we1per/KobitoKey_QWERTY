/* SPDX-License-Identifier: MIT */
#pragma once

#include <stdbool.h>
#include <stdint.h>

/* Independent of Zephyr so timer precedence can be tested on a host. */
struct auto_mouse_policy {
    bool active;
    bool has_keypress;
    uint64_t held_buttons;
    int64_t last_keypress;
    int64_t deadline;
};

static inline void auto_mouse_motion(struct auto_mouse_policy *s, int64_t now,
                                     int32_t motion_ms, int32_t idle_ms) {
    if (!s->active && s->has_keypress && now - s->last_keypress < idle_ms) {
        return;
    }
    s->active = true;
    s->deadline = now + motion_ms;
}

static inline void auto_mouse_button(struct auto_mouse_policy *s, uint32_t position, bool pressed,
                                     int64_t now, int32_t click_ms) {
    if (pressed) {
        s->held_buttons |= UINT64_C(1) << position;
    } else {
        s->held_buttons &= ~(UINT64_C(1) << position);
        if (s->active && !s->held_buttons) {
            s->deadline = now + click_ms;
        }
    }
}

static inline void auto_mouse_key(struct auto_mouse_policy *s, int64_t now, bool excluded) {
    s->has_keypress = true;
    s->last_keypress = now;
    if (!excluded) {
        s->active = false;
    }
}

static inline bool auto_mouse_expired(const struct auto_mouse_policy *s, int64_t now) {
    return s->active && !s->held_buttons && now >= s->deadline;
}
