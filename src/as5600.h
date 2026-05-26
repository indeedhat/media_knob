#pragma once


#include <zephyr/usb/usbd.h>


#define DEAD_ZONE 1


struct as5600_dev_data {
	int16_t position;
};


int as5600_init(const struct device* dev);
int as5600_read(const struct device *dev);
