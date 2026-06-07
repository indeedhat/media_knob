#pragma once

#include <stdbool.h>
#include <stdint.h>


#define KEEB_ATTR_IDX  6
#define MOUSE_ATTR_IDX 10
#define MEDIA_ATTR_IDX 16


int bt_init();
bool bt_connected();
int bt_submit_report(const uint16_t size, const uint8_t *const report);
