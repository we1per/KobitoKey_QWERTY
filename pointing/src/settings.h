/* SPDX-License-Identifier: MIT */
#pragma once

#include <cormoran/zmk/custom_settings.h>

/* Attach to the existing generic settings subsystem, without a new GUI/RPC. */
#define KOBITO_SETTING_INT(name, key, initial, low, high)                                      \
    ZMK_CUSTOM_SETTING_DEFINE(                                                              \
        name, "cormoran_custom_settings", key, ZMK_CUSTOM_SETTING_VALUE_TYPE_INT32,            \
        ZMK_CUSTOM_SETTING_VALUE_INT32(initial), ZMK_CUSTOM_SETTING_CONFIDENTIALITY_RPC_PUBLIC,\
        ZMK_CUSTOM_SETTING_PERMISSION_UNSECURE, ZMK_CUSTOM_SETTING_PERMISSION_SECURE,          \
        ZMK_CUSTOM_SETTING_RANGE_INT32(low, high))

static inline int32_t kobito_setting_int(const struct zmk_custom_setting *setting,
                                         int32_t fallback) {
    struct zmk_custom_setting_value value;
    return zmk_custom_setting_read(setting, &value) == 0 ? value.int32_value : fallback;
}
