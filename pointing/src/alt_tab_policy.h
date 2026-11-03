/* SPDX-License-Identifier: MIT */
#pragma once

#include <stdbool.h>
#include <stdint.h>

struct alt_tab_policy {
    uint32_t holds;
    uint8_t swallowed;
};

/* True means the caller must press Alt; every activation also taps Tab. */
static inline bool alt_tab_begin(struct alt_tab_policy *s) { return s->holds++ == 0; }

/* True means the last invoking combo/key has released its Alt hold. */
static inline bool alt_tab_end(struct alt_tab_policy *s) {
    return s->holds && --s->holds == 0;
}

/* key 0 = forward, key 1 = backward. Return 0 to pass, 1 to consume,
 * 2 to consume and send Tab, 3 to consume and send Shift+Tab.
 * Remember consumed downs beyond combo release to swallow their matching ups. */
static inline int alt_tab_navigate(struct alt_tab_policy *s, unsigned int key, bool pressed) {
    uint8_t bit = 1U << key;
    if (!pressed) {
        bool swallowed = (s->swallowed & bit) != 0;
        s->swallowed &= ~bit;
        return swallowed ? 1 : 0;
    }
    if (!s->holds) {
        return 0;
    }
    s->swallowed |= bit;
    return key == 0 ? 2 : 3;
}
