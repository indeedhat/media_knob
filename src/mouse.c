#include "knob.h"
#include <sample_usbd.h>
#include <zephyr/logging/log.h>


LOG_MODULE_REGISTER(mouse, LOG_LEVEL_DBG);


static void mouse_iface_ready(const struct device *dev, const bool ready);
static int mouse_get_report(
	const struct device *dev,
	const uint8_t type,
	const uint8_t id,
	const uint16_t len,
	uint8_t *const buf
);
static int mouse_set_report(
	const struct device *dev,
	const uint8_t type,
	const uint8_t id,
	const uint16_t len,
	const uint8_t *const buf
);


struct hid_device_ops mouse_ops = {
	.iface_ready = mouse_iface_ready,
	.get_report = mouse_get_report,
	.set_report = mouse_set_report,
};

static bool is_ready = false;


int mouse_init(const struct device *dev, struct usbd_context *ctx) {
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


bool mouse_ready()
{
	return is_ready;
}


static void mouse_iface_ready(const struct device *dev, const bool ready)
{
	LOG_INF("HID device %s interface is %s",
		 dev->name, ready ? "ready" : "not ready"
	);

	is_ready = ready;
}


static int mouse_get_report(
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

static int mouse_set_report(
	const struct device *dev,
	const uint8_t type,
	const uint8_t id,
	const uint16_t len,
	const uint8_t *const buf
) {
	LOG_INF("got report %d -- %d", buf[0], type);

    if (type == HID_REPORT_TYPE_FEATURE) {
        LOG_INF("Resolution multiplier feature report received: %d", buf[0]);
        return 0;
    }

    return -ENOTSUP;
}
