#pragma once


#include <zephyr/usb/usbd.h>


// In some cases if the magnet is not 100% aligned to the sensor then in some
// orientations the dial will output a steady stream of 1, -1, 1, -1
// Enabling JITTER_COMPENSATION will help to account for this
#define AS5600_JITTER_COMPENSATION
#define AS5600_JITTER_THRESHOLD 4
#define AS5600_JITTER_DEAD_ZONE 1

// in the case where jitter control doesn't cut it we can set a deadzone
// any angle smaller <= the dead zone will be ignored
// its a far from ideal solution as it hinders the operation and makes
// the dial less sensitive
//#define AS5600_DEAD_ZONE 1


struct as5600_dev_data {
	int16_t position;
};


// Initialize the sensor device
int as5600_init(const struct device* dev);

// Read the angle change from the sensor since the last read
int as5600_read(const struct device *dev);
