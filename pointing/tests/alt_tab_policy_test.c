/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include "../src/alt_tab_policy.h"

int main(void) {
    struct alt_tab_policy s = {0};
    assert(alt_tab_navigate(&s, 0, true) == 0);  /* ordinary F */
    assert(alt_tab_navigate(&s, 1, true) == 0);  /* ordinary S */
    assert(!alt_tab_end(&s));                  /* unmatched release */
    assert(alt_tab_begin(&s));                 /* Alt down + initial Tab */
    assert(alt_tab_navigate(&s, 0, false) == 0); /* no F down to swallow */
    assert(alt_tab_navigate(&s, 0, true) == 2);  /* F -> Tab */
    assert(alt_tab_navigate(&s, 0, false) == 1);
    assert(alt_tab_navigate(&s, 1, true) == 3);  /* S -> Shift+Tab */
    assert(alt_tab_navigate(&s, 1, false) == 1);
    assert(alt_tab_navigate(&s, 1, false) == 0); /* never swallow unmatched up */
    assert(alt_tab_begin(&s) == false);        /* another binding shares Alt */
    assert(!alt_tab_end(&s));                  /* one binding still held */
    assert(alt_tab_navigate(&s, 0, true) == 2);
    assert(alt_tab_navigate(&s, 1, true) == 3);
    assert(alt_tab_end(&s));                   /* last release -> Alt up */
    assert(s.holds == 0);
    assert(alt_tab_navigate(&s, 0, false) == 1); /* F/S up after combo release */
    assert(alt_tab_navigate(&s, 1, false) == 1);
    assert(s.swallowed == 0);
    assert(alt_tab_navigate(&s, 0, true) == 0);  /* normal typing restored */
    assert(alt_tab_navigate(&s, 1, false) == 0);
    assert(alt_tab_begin(&s));                 /* next session is independent */
    assert(alt_tab_end(&s));
    puts("alt_tab_policy: all scenarios passed");
    return 0;
}
