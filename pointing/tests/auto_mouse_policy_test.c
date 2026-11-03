/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include "../src/auto_mouse_policy.h"

int main(void) {
    struct auto_mouse_policy s = {0};

    /* Movement arms/replaces the 10 second timer. */
    auto_mouse_motion(&s, 1000, 10000, 350);
    assert(s.active && s.deadline == 11000);
    assert(!auto_mouse_expired(&s, 10999));
    assert(auto_mouse_expired(&s, 11000));
    auto_mouse_motion(&s, 2000, 10000, 350);
    assert(!auto_mouse_expired(&s, 11000));
    assert(auto_mouse_expired(&s, 12000));

    /* A held click prevents expiry even during a long drag. */
    auto_mouse_button(&s, 6, true, 2500, 800);
    assert(!auto_mouse_expired(&s, 50000));
    auto_mouse_motion(&s, 50000, 10000, 350);
    assert(!auto_mouse_expired(&s, 60000));
    auto_mouse_button(&s, 6, false, 60000, 800);
    assert(!auto_mouse_expired(&s, 60799));
    assert(auto_mouse_expired(&s, 60800));

    /* Movement after releasing a click switches back to the movement timer. */
    auto_mouse_motion(&s, 60500, 10000, 350);
    assert(s.deadline == 70500);
    assert(!auto_mouse_expired(&s, 60800));
    assert(auto_mouse_expired(&s, 70500));

    /* For overlapping buttons, start the click timer after the last release. */
    auto_mouse_button(&s, 6, true, 61000, 800);
    auto_mouse_button(&s, 8, true, 61010, 800);
    auto_mouse_button(&s, 6, false, 62000, 800);
    assert(!auto_mouse_expired(&s, 100000));
    auto_mouse_button(&s, 8, false, 100000, 800);
    assert(s.deadline == 100800);

    /* Excluded positions preserve the layer; an ordinary position cancels it. */
    auto_mouse_key(&s, 100100, true);
    assert(s.active);
    auto_mouse_key(&s, 100200, false);
    assert(!s.active);
    assert(!auto_mouse_expired(&s, 200000));

    /* Prevent re-entry while typing, including the exact idle boundary. */
    auto_mouse_motion(&s, 100549, 10000, 350);
    assert(!s.active);
    auto_mouse_motion(&s, 100550, 10000, 350);
    assert(s.active);

    /* Releasing a button after key cancellation must not reactivate the layer. */
    auto_mouse_button(&s, 7, true, 100600, 800);
    auto_mouse_key(&s, 100650, false);
    auto_mouse_button(&s, 7, false, 100700, 800);
    assert(!s.active && !s.held_buttons);

    /* GUI-adjusted timeout values take effect on the next matching activity. */
    auto_mouse_motion(&s, 101000, 2000, 0);
    assert(s.deadline == 103000);
    auto_mouse_button(&s, 6, true, 101100, 300);
    auto_mouse_button(&s, 6, false, 101200, 300);
    assert(s.deadline == 101500);
    puts("auto_mouse_policy: all scenarios passed");
    return 0;
}
