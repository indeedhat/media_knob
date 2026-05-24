#include "sample_usbd.h"

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/usb/class/usbd_hid.h>
#include <zephyr/usb/class/hid.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(scroll_wheel, LOG_LEVEL_INF);

static const uint8_t hid_scroll_report_desc[] = {
	HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),
	HID_USAGE(HID_USAGE_GEN_DESKTOP_MOUSE),
	HID_COLLECTION(HID_COLLECTION_APPLICATION),
		/* Scroll wheel - signed 8-bit relative value */
		HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),
		HID_USAGE(HID_USAGE_GEN_DESKTOP_WHEEL),
		HID_LOGICAL_MIN8(-127),
		HID_LOGICAL_MAX8(127),
		HID_REPORT_SIZE(8),
		HID_REPORT_COUNT(1),
		HID_INPUT(0x06),  /* Data, Variable, Relative */
		
		/* High precision scroll - signed 16-bit relative value */
		HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),
		HID_USAGE(HID_USAGE_GEN_DESKTOP_WHEEL),
		HID_LOGICAL_MIN16(0x00, 0x80),  /* -32768 */
		HID_LOGICAL_MAX16(0xFF, 0x7F),  /* 32767 */
		HID_REPORT_SIZE(16),
		HID_REPORT_COUNT(1),
		HID_INPUT(0x06),  /* Data, Variable, Relative */
	HID_END_COLLECTION,
};

/* HID report structure */
struct hid_scroll_report {
	int8_t scroll_wheel;       /* -128 to 127 for normal scroll */
	int16_t scroll_precise;    /* -32768 to 32767 for high precision */
} __packed;

static int32_t last_angle_raw = 0;
static int32_t angle_accumulator = 0;
static bool hid_ready = false;

static void scroll_iface_ready(const struct device *dev, const bool ready)
{
	LOG_INF("HID device %s interface is %s",
		dev->name, ready ? "ready" : "not ready");
	hid_ready = ready;
}

static int scroll_get_report(const struct device *dev,
			 const uint8_t type, const uint8_t id, const uint16_t len,
			 uint8_t *const buf)
{
	LOG_WRN("Get Report not implemented");
	return 0;
}

static const struct hid_device_ops scroll_ops = {
	.iface_ready = scroll_iface_ready,
	.get_report = scroll_get_report,
};

int main(void)
{
	LOG_INF("Starting scroll wheel HID device");

	/* Get AS5600 sensor */
	const struct device *as5600 = DEVICE_DT_GET_ANY(ams_as5600);
	if (as5600 == NULL) {
		LOG_ERR("AS5600 not found");
		return -ENODEV;
	}

	if (!device_is_ready(as5600)) {
		LOG_ERR("AS5600 not ready");
		return -ENODEV;
	}

	LOG_INF("AS5600 ready");

	/* Get HID device */
	const struct device *hid_dev = DEVICE_DT_GET_ONE(zephyr_hid_device);
	if (!device_is_ready(hid_dev)) {
		LOG_ERR("HID Device is not ready");
		return -ENODEV;
	}

	/* Register HID device */
	int ret = hid_device_register(hid_dev,
				      hid_scroll_report_desc, 
				      sizeof(hid_scroll_report_desc),
				      &scroll_ops);
	if (ret != 0) {
		LOG_ERR("Failed to register HID Device: %d", ret);
		return ret;
	}

	LOG_INF("HID device registered");

	/* Initialize USBD with sample config */
	struct usbd_context *sample_usbd = sample_usbd_init_device(NULL);
	if (sample_usbd == NULL) {
		LOG_ERR("Failed to initialize USB device");
		return -ENODEV;
	}

	ret = usbd_enable(sample_usbd);
	if (ret != 0) {
		LOG_ERR("Failed to enable device support: %d", ret);
		return ret;
	}

	LOG_INF("USB device enabled");
	k_sleep(K_MSEC(500));

	struct hid_scroll_report report = {0};

	while (1) {
		/* Read sensor */
		ret = sensor_sample_fetch(as5600);
		if (ret != 0) {
			LOG_DBG("Sample fetch failed: %d", ret);
			k_sleep(K_MSEC(10));
			continue;
		}

		struct sensor_value angle;
		sensor_channel_get(as5600, SENSOR_CHAN_ROTATION, &angle);

		/* Convert to raw position (0-4095) */
		int32_t angle_raw = (angle.val1 * 4096) / 360;
		if (angle.val2 > 0) {
			angle_raw += (angle.val2 * 4096) / (360 * 1000000);
		}

		/* Calculate delta from last position */
		int32_t delta = angle_raw - last_angle_raw;

		/* Handle wraparound (crossing 0/360 boundary) */
		if (delta > 2048) {
			delta -= 4096;
		} else if (delta < -2048) {
			delta += 4096;
		}

		last_angle_raw = angle_raw;
		angle_accumulator += delta;

		/* Create HID report */
		report.scroll_wheel = (int8_t)(angle_accumulator / 256);
		report.scroll_precise = (int16_t)angle_accumulator;

		LOG_DBG("Angle: %d.%06d°, Delta: %d, Accum: %d",
		       angle.val1, angle.val2, (int)delta, (int)angle_accumulator);

		/* Send HID report if ready */
		if (hid_ready) {
			ret = hid_device_submit_report(hid_dev, 
						      sizeof(report), 
						      (uint8_t *)&report);
			if (ret < 0) {
				LOG_ERR("HID submit failed: %d", ret);
			}
		}

		k_sleep(K_MSEC(10));
	}

	return 0;
}















// #include <sample_usbd.h>

// #include <zephyr/usb/class/hid.h>
// #include <zephyr/device.h>
// #include <zephyr/kernel.h>
// #include <zephyr/drivers/sensor.h>

// #include <zephyr/usb/usbd.h>
// #include <zephyr/usb/class/usbd_hid.h>

// #include <zephyr/sys/printk.h>
// #include <zephyr/logging/log.h>

// #include <sys/errno.h>
// LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);


// #define SLEEP_TIME_MS   10


// static const uint8_t hid_report_desc[] = {
// 	HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),
// 	HID_USAGE(HID_USAGE_GEN_DESKTOP_MOUSE),
// 	HID_COLLECTION(HID_COLLECTION_APPLICATION),
// 		HID_COLLECTION(HID_COLLECTION_PHYSICAL),
// 			HID_REPORT_ID(1),
// 			HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),
// 			HID_USAGE(HID_USAGE_GEN_DESKTOP_WHEEL),
// 			HID_LOGICAL_MIN16(0x80, 0x00), // -32768
// 			HID_LOGICAL_MAX16(0x7F, 0xFF), // 32767
// 			HID_REPORT_SIZE(16),
// 			HID_REPORT_COUNT(1),
// 			HID_INPUT(0x06),
// 		HID_END_COLLECTION,
// 	HID_END_COLLECTION,
// };
// static bool device_ready;

// struct hid_scroll_report {
// 	uint8_t report_id;
// 	int16_t scroll_delta;
// } __packed;

// static int32_t prev_angle = 0;
// static int32_t angle_acc = 0;

// static const struct device* as5600_init();
// static const struct device* hid_init();

// static void mouse_ready(const struct device *dev, const bool ready);
// static int mouse_get_report(
// 	const struct device *dev,
// 	const uint8_t type,
// 	const uint8_t id,
// 	const uint16_t len,
// 	uint8_t *const buf
// );


// struct hid_device_ops mouse_ops = {
// 	.iface_ready = mouse_ready,
// 	.get_report = mouse_get_report,
// };


// int main(void)
// {
// 	LOG_INF("app started");
// 	const struct device *as5600 = as5600_init();
// 	if (as5600 == NULL) {
// 		return -ENODEV;
// 	}

// 	const struct device *hid = hid_init();
// 	if (hid == NULL) {
// 		return -ENODEV;
// 	}

// 	while (1) {
// 		struct sensor_value angle;
// 		sensor_sample_fetch(as5600);
// 		sensor_channel_get(as5600, SENSOR_CHAN_ROTATION, &angle);
// 		printk("Angle: %d.%06d degrees\n", angle.val1, angle.val2);

// 		k_msleep(SLEEP_TIME_MS);
// 	}

// 	return 0;
// }


// static const struct device* hid_init() {
// 	int ret;

// 	const struct device *dev = DEVICE_DT_GET_ONE(zephyr_hid_device);
// 	if (dev == NULL) {
// 		LOG_ERR("HID Device is not ready");
// 		return NULL:
// 	}

// 	ret = hid_device_register(dev, hid_report_desc,sizeof(hid_report_desc), &mouse_opts);
// 	if (ret) {
// 		LOG_ERR("Failed to register HID Device, %d", ret);
// 		return NULL;
// 	}
// }


// static const struct device* as5600_init() {
// 	const struct device *dev = DEVICE_DT_GET_ANY(ams_as5600);
// 	if (dev == NULL) {
// 		LOG_ERR("AS5600 not found");
// 		return NULL;
// 	}

// 	LOG_INF("AS5600 found");

// 	// setup the rotation sensor
// 	if (!device_is_ready(dev)) {
// 		LOG_ERR("AS5600 not ready\n");
// 		return NULL;
// 	}

// 	LOG_INF("AS5600 ready");
// 	return dev;
// }


// static void mouse_iface_ready(const struct device *dev, const bool ready)
// {
// 	LOG_INF("HID device %s interface is %s",
// 		dev->name, ready ? "ready" : "not ready"
// 	);

// 	device_ready = ready;
// }

// static int mouse_get_report(
// 	const struct device *dev,
// 	const uint8_t type,
// 	const uint8_t id,
// 	const uint16_t len,
// 	uint8_t *const buf
// ) {
// 	LOG_WRN("Get Report not implemented, Type %u ID %u", type, id);

// 	return 0;
// }
