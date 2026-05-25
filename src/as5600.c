#include "knob.h"

#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>


LOG_MODULE_REGISTER(as5600, LOG_LEVEL_DBG);


int16_t as5600_prev_pos;


int as5600_read(const struct device *dev)
{
	int err;
	err = sensor_sample_fetch(dev);
	if (err) {
		LOG_DBG("Sample fetch failed: %d", err);
		return 0;
	}

	struct as5600_dev_data *dev_data = dev->data;
	int16_t delta = dev_data->position - as5600_prev_pos;
	as5600_prev_pos = dev_data->position;

	if (delta > 2048) {
		delta -= 4096;
	} else if (delta < -2048) {
		delta += 4096;
	}

	if (delta <= DEAD_ZONE && delta >= -DEAD_ZONE) {
		return 0;
	}

	return delta;
}


int as5600_init(const struct device* dev) {
	if (dev == NULL) {
		LOG_ERR("AS5600 not found");
		return -ENODEV;
	}

	LOG_DBG("AS5600 found");

	// setup the rotation sensor
	if (!device_is_ready(dev)) {
		LOG_ERR("AS5600 not ready\n");
		return -EIO;
	}

	LOG_DBG("AS5600 ready");

	return 0;
}

