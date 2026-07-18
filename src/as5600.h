#pragma once


#include <zephyr/usb/usbd.h>


// In some cases if the magnet is not 100% aligned to the sensor then in some
// orientations the dial will output a steady stream of 1, -1, 1, -1
// Enabling JITTER_COMPENSATION will help to account for this
#define AS5600_JITTER_COMPENSATION
#define AS5600_JITTER_THRESHOLD 4
#define AS5600_JITTER_DEAD_ZONE 1
// the amount of time in ms to sleep between reads when jitter compensation is
// enabled this allows for a larger accumulation of angle to avoid jitter
// control being "sticky"
#define AS5600_JITTER_DELAY_MS 80
// when defined this will replace the accumulated value for the first return
// after jitter compensation is broken
// the sign used will come from the sign of the real accumulated value
#define AS5600_JITTER_POST_RESULT 1


// in the case where jitter control doesn't cut it we can set a deadzone
// any angle smaller <= the dead zone will be ignored
// its a far from ideal solution as it hinders the operation and makes
// the dial less sensitive
//#define AS5600_DEAD_ZONE 1


struct as5600_dev_data {
	int16_t position;
};


/**
 * @brief as5600_init initializes the sensor device
 *
 * @param[in] dev the device struct
 *
 * @return error code
 */
int as5600_init(const struct device* dev);

// Read the angle change from the sensor since the last read
/**
 * @brief as5600_read reads the angle change from the sensor since the last read
 *
 * @param[in] dev the device struct
 *
 * @return error code
 */
int as5600_read(const struct device *dev);

/**
 * @brief as5600_jitter_compensation_enabled returns the current state of jitter compensation
 *
 * @return [TODO:description]
 */
bool as5600_jitter_compensation_enabled();
