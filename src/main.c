#include "knob.h"

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zephyr/usb/class/usbd_hid.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/input/input.h>


enum op_modes {
	MODE_SCROLL,
	MODE_MEDIA,
	MODE_COUNT,
};

int current_mode = MODE_SCROLL;
int mod_state = 0;
int64_t last_seek_time = 0;

typedef struct {
	const struct device *hid;
	const struct device *as5600;
} devices;

devices device_state = {
	.hid = DEVICE_DT_GET_ONE(zephyr_hid_device),
	.as5600 = DEVICE_DT_GET_ANY(ams_as5600),
};


static void scroll_action(int16_t angle, const struct device *hid);
static void media_action(int16_t angle, const struct device *hid);
static void button_input_cb(struct input_event *evt, void *user_data);


LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);
INPUT_CALLBACK_DEFINE(NULL, button_input_cb, NULL);


#define BTN_MODE_CODE 2
#define BTN_MOD_CODE 11
#define DEBOUNCE_TIME 2000


int main(void)
{
	int err;
	struct usbd_context usbd_ctx;

	err = hid_init(device_state.hid, &usbd_ctx);
	if (err != 0) {
		return err;
	}

	err = as5600_init(device_state.as5600);
	if (err != 0) {
		return err;
	}

	while (true) {
		k_msleep(10);
		continue;
		if (!hid_ready()) {
			continue;
		}

		int16_t angle = as5600_read(device_state.as5600);
		if (angle > 127) {
			angle = 127;
		} else if (angle < -128) {
			angle = -128;
		}

		switch (current_mode) {
		case MODE_SCROLL:
			scroll_action(angle & 0xFF, device_state.hid);
			break;
		case MODE_MEDIA:
			media_action(angle & 0xFF, device_state.hid);
			break;
		};
	}

	return 0;
}


static void scroll_action(int16_t angle, const struct device *hid)
{
	return;
	int err;

	if (angle == 0) {
		return;
	}

	int8_t report[MOUSE_REPORT_SIZE];
	report[MOUSE_REPORT_IDX] = MOUSE_REPORT_ID;
	report[MOUSE_ANGLE_IDX] = angle;

	err = hid_device_submit_report(hid, MOUSE_REPORT_SIZE, report);
	if (err) {
		LOG_ERR("HID submit report error, %d", err);
	} else {
		LOG_DBG("sent angle: %d", angle);
	}
}

static void media_action(int16_t angle, const struct device *hid)
{
	int err;
	uint8_t report[MEDIA_REPORT_IDX];
	report[MEDIA_REPORT_IDX] = MEDIA_REPORT_ID;

	if (mod_state) {
		// debounce
		int64_t now = k_uptime_get();
		if (now < last_seek_time + DEBOUNCE_TIME) {
			last_seek_time = now;
			return;
		}

		last_seek_time = now;

		report[MEDIA_ACTION_IDX] = angle > 0
			? HID_MEDIA_SCAN_NEXT
			: HID_MEDIA_SCAN_PREV;
	} else {
		report[MEDIA_ACTION_IDX] = angle > 0
			? HID_MEDIA_VOL_UP
			: HID_MEDIA_VOL_DONW;
	}

	err = hid_device_submit_report(hid,MEDIA_REPORT_SIZE, report);
	if (err) {
		LOG_ERR("failed to send volume media event: %d", err);
	} else {
		LOG_INF("media volume event sent");
	}
}


static void button_input_cb(struct input_event *evt, void *user_data)
{
	if (evt->sync == 0) {
		return;
	}

	if (evt->code == BTN_MODE_CODE) {
		if (evt->value == 0) {
			current_mode = (current_mode + 1) % MODE_COUNT;
		}
		LOG_INF("Set mode to %s", current_mode == MODE_SCROLL ? "scroll" : "media");
		return;
	}

	if (evt->code == BTN_MOD_CODE) {
		mod_state = evt->value;
		if (current_mode == MODE_MEDIA) {
			// TODO handle double press play/pause

			return;
		}

		if (current_mode == MODE_SCROLL) {
			int8_t report[KEEB_REPORT_SIZE];
			report[KEEB_REPORT_IDX] = KEEB_REPORT_ID;
			report[KEEB_MODIFIER_IDX] = evt->value
				? HID_LEFT_CTRL
				: 0;

			int err = hid_device_submit_report(device_state.hid, KEEB_REPORT_SIZE, report);
			if (err) {
				LOG_ERR("failed to send hid keyboard report: %d", err);
			} else {
				LOG_INF("hid keyboard report sent");
			}
		}

		return;
	}
}
