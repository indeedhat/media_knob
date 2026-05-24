#include "sample_usbd.h"

#include <zephyr/usb/usbd.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sample_usbd, LOG_LEVEL_INF);

static struct usbd_context sample_usbd;

struct usbd_context *sample_usbd_init_device(usbd_msg_cb_t msg_cb)
{
	int err;

	/* Get the USB device from device tree */
	const struct device *udc = device_get_binding("UDC_0");
	if (!udc) {
		LOG_ERR("USB UDC device not found");
		return NULL;
	}

	sample_usbd.dev = udc;

	err = usbd_init(&sample_usbd);
	if (err) {
		LOG_ERR("Failed to initialize device support: %d", err);
		return NULL;
	}

	if (msg_cb != NULL) {
		err = usbd_msg_register_cb(&sample_usbd, msg_cb);
		if (err) {
			LOG_ERR("Failed to register message callback: %d", err);
			return NULL;
		}
	}

	return &sample_usbd;
}
