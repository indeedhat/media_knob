/** @file
 *  @brief HoG Service sample
 */

/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef __cplusplus
extern "C" {
#endif

const struct bt_gatt_service_static hog_init();

void hog_button_loop(void);

#ifdef __cplusplus
}
#endif
