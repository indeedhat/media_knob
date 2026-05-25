#include "knob.h"

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zephyr/usb/class/usbd_hid.h>
#include <zephyr/usb/usbd.h>


LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);


int main(void)
{
	int err;
	struct usbd_context usbd_ctx;
	const struct device *as5600_dev = DEVICE_DT_GET_ANY(ams_as5600);
	const struct device *hid_dev = DEVICE_DT_GET_ONE(zephyr_hid_device);

	err = mouse_init(hid_dev, &usbd_ctx);
	if (err != 0) {
		return err;
	}

	err = as5600_init(as5600_dev);
	if (err != 0) {
		return err;
	}

	while (true) {
		int8_t report[MOUSE_REPORT_COUNT];

		if (!mouse_ready()) {
			continue;
		}

		int16_t angle = as5600_read(as5600_dev);
		if (!angle) {
			continue;
		}

		report[1] = angle;

		err = hid_device_submit_report(hid_dev, MOUSE_REPORT_COUNT, report);
		if (err) {
			LOG_ERR("HID submit report error, %d", err);
		} else {
			LOG_DBG("sent angle: %d", angle);
		}
	}

	return 0;
}

