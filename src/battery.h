
#define BAS_CLAMP_MAX 4200
#define BAS_CLAMP_MIN 3000
#define BAS_USB_CONNECTED_MV 4400
#define BAS_VOLTAGE_DIVIDER 5
#define BAS_POLL_INTERVAL_S 300

/**
 * @brief init_battery_level initializes the periodic task to update the host with the devices battery level
 *
 * @return error code
 */
int init_battery_level();
