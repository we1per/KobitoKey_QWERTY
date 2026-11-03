/* SPDX-License-Identifier: MIT */
#define DT_DRV_COMPAT kobito_precision

#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/pointing/input_processor_runtime.h>
#include "settings.h"

KOBITO_SETTING_INT(precision_divisor, "pointer/precision_divisor", 4, 2, 32);
static uint32_t held_count;

static int precision_pressed(struct zmk_behavior_binding *binding,
                             struct zmk_behavior_binding_event event) {
    const struct device *mouse = zmk_input_processor_runtime_find_by_name("mouse");
    struct zmk_input_processor_runtime_config cfg;
    if (!mouse || zmk_input_processor_runtime_get_config(mouse, NULL, &cfg) < 0) {
        return -ENODEV;
    }
    if (held_count) {
        held_count++;
        return ZMK_BEHAVIOR_OPAQUE;
    }
    uint32_t divisor = kobito_setting_int(&precision_divisor, 4);
    if (cfg.scale_divisor > UINT32_MAX / divisor) {
        return -ERANGE;
    }
    int ret = zmk_input_processor_runtime_set_scaling(
        mouse, cfg.scale_multiplier, cfg.scale_divisor * divisor,
        ZMK_INPUT_PROCESSOR_RUNTIME_WRITE_MODE_TEMPORARY);
    if (ret < 0) {
        return ret;
    }
    held_count = 1;
    return ZMK_BEHAVIOR_OPAQUE;
}

static int precision_released(struct zmk_behavior_binding *binding,
                              struct zmk_behavior_binding_event event) {
    if (held_count && --held_count == 0) {
        const struct device *mouse = zmk_input_processor_runtime_find_by_name("mouse");
        if (mouse) {
            zmk_input_processor_runtime_restore_persistent(mouse);
        }
    }
    return ZMK_BEHAVIOR_OPAQUE;
}

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
static const struct behavior_parameter_metadata metadata = {.sets_len = 0, .sets = NULL};
#endif
static const struct behavior_driver_api precision_api = {
    .binding_pressed = precision_pressed,
    .binding_released = precision_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .parameter_metadata = &metadata,
#endif
};
BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL,
                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &precision_api);
