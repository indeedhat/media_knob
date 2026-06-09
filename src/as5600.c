#include "as5600.h"

#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>


LOG_MODULE_REGISTER(as5600, LOG_LEVEL_DBG);


int16_t prev_pos;
int16_t prev_delta;
int16_t jitter_count;


int as5600_read(const struct device *dev)
{
	int err = sensor_sample_fetch(dev);
	if (err) {
		LOG_DBG("Sample fetch failed: %d", err);
		return 0;
	}

	struct as5600_dev_data *dev_data = dev->data;
	int16_t delta = dev_data->position - prev_pos;
	prev_pos = dev_data->position;

	if (delta > 2048) {
		delta -= 4096;
	} else if (delta < -2048) {
		delta += 4096;
	}

#if defined AS5600_JITTER_COMPENSATION
	if (delta == 0) {
		return 0;
	}

	if (jitter_count >= AS5600_JITTER_THRESHOLD) {
		if (delta <= AS5600_JITTER_DEAD_ZONE && delta >= -AS5600_JITTER_DEAD_ZONE) {
			LOG_DBG("jitter compensated");
			return 0;
		}

		jitter_count = 0;
	}

	if (prev_delta == -delta) {
		jitter_count++;
	}
#endif

#if defined AS5600_DEAD_ZONE
	if (delta <= AS5600_DEAD_ZONE && delta >= -AS5600_DEAD_ZONE) {
		LOG_DBG("DEAD ZONE %d", delta);
		return 0;
	}
#endif

	prev_delta = delta;

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

