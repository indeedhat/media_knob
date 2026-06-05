
#include <stdbool.h>
#include <stdint.h>

int bt_init();
bool bt_connected();
int bt_submit_report(const uint16_t size, const uint8_t *const report);
