#pragma once

#include <stdbool.h>
#include <stdint.h>


#define BT_KEEB_ATTR_IDX  7
#define BT_MOUSE_ATTR_IDX 11
#define BT_MEDIA_ATTR_IDX 19


// Initialize the bluetooth device
int bt_init();

// Check if the device is connected to a host machine
bool bt_connected();

// Send a hids report to the host device
int bt_submit_report(const uint16_t size, const uint8_t *const report);
