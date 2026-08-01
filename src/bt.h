#pragma once

#include <stdbool.h>
#include <stdint.h>


#define BT_KEEB_ATTR_IDX  7
#define BT_MOUSE_ATTR_IDX 11
#define BT_MEDIA_ATTR_IDX 19
#define BT_MACRO_ATTR_IDX 26


/**
 * @brief bt_init initializes the bluetooth subsystem
 *
 * @return error code
 */
int bt_init();

/**
 * @brief bt_connected returns the connection state of the bluetooth connection
 *
 * @return connection state
 */
bool bt_connected();

/**
 * @brief bt_submit_report sends off an HID report to the bluetooth subsystem
 *
 * @param[in] size the size of the report
 * @param[in] report the report bytes
 *
 * @return error code
 */
int bt_submit_report(const uint16_t size, const uint8_t *const report);
