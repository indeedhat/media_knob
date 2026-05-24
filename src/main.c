#include "knob.h"

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_core.h>
#include <zephyr/sys/util.h>
#include <zephyr/usb/class/usbd_hid.h>
#include <zephyr/usb/usbd.h>

#include <sample_usbd.h>


LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);


static bool mouse_ready;
int16_t as5600_prev_pos;


struct hid_device_ops mouse_ops = {
	.iface_ready = mouse_iface_ready,
	.get_report = mouse_get_report,
	.set_report = mouse_set_report,
};


int main(void)
{
	int err;
	struct usbd_context usbd_ctx;
	const struct device *as5600_dev = DEVICE_DT_GET_ANY(ams_as5600);
	const struct device *hid_dev = DEVICE_DT_GET_ONE(zephyr_hid_device);

	err = hid_init(hid_dev, &usbd_ctx);
	if (err != 0) {
		return err;
	}

	err = as5600_init(as5600_dev);
	if (err != 0) {
		return err;
	}

	while (true) {
		int8_t report[MOUSE_REPORT_COUNT];

		if (!mouse_ready) {
			continue;
		}

		int16_t angle = as5600_read(as5600_dev);
		if (!angle) {
			continue;
		}

		report[1] = 0xFF;
		report[2] = angle;

		err = hid_device_submit_report(hid_dev, MOUSE_REPORT_COUNT, report);
		if (err) {
			LOG_ERR("HID submit report error, %d", err);
		} else {
			LOG_DBG("sent angle: %d - %d", angle, angle / 32);
		}
	}

	return 0;
}


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


int hid_init(const struct device *dev, struct usbd_context *ctx) {
	int err;

	if (!device_is_ready(dev)) {
		LOG_ERR("HID Device is not ready");
		return -EIO;
	}

	err = hid_device_register(
		dev,
		hid_report_desc,
		sizeof(hid_report_desc),
		&mouse_ops
	);
	if (err != 0) {
		LOG_ERR("Failed to register HID Device, %d", err);
		return err;
	}

	ctx = sample_usbd_init_device(NULL);
	if (ctx == NULL) {
		LOG_ERR("Failed to initialize USB device");
		return -ENODEV;
	}

	err = usbd_enable(ctx);
	if (err != 0) {
		LOG_ERR("Failed to enable device support");
		return err;
	}

	LOG_DBG("USB device support enabled");

	return 0;
}


void mouse_iface_ready(const struct device *dev, const bool ready)
{
	LOG_INF("HID device %s interface is %s",
		dev->name, ready ? "ready" : "not ready");
	mouse_ready = ready;
}


int mouse_get_report(
	const struct device *dev,
	const uint8_t type,
	const uint8_t id,
	const uint16_t len,
	uint8_t *const buf
) {
	LOG_INF("GET_REPORT: type=%d, id=%d, len=%d", type, id, len);
	if (type == HID_REPORT_TYPE_FEATURE) {
		buf[0] = 0x0F;
		return 1;
	}

	LOG_WRN("Get Report not implemented, Type %u ID %u", type, id);

	return 0;
}

int mouse_set_report(
	const struct device *dev,
	const uint8_t type,
	const uint8_t id,
	const uint16_t len,
	const uint8_t *const buf
) {
	LOG_INF("got report %d -- %d", buf[0], type);

    if (type == HID_REPORT_TYPE_FEATURE) {
        // buf[0] contains the multiplier value sent by the host
        LOG_INF("Resolution multiplier feature report received: %d", buf[0]);
        // Store the multiplier value if needed
        return 0;  // Return 0 on success
    }

    return -ENOTSUP;
}
