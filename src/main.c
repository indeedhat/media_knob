#include "knob.h"
#include "as5600.h"
#include "hid.h"
#include "bluetooth.h"

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zephyr/usb/class/usbd_hid.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/input/input.h>
#include <zephyr/settings/settings.h>
#include <zephyr/bluetooth/bluetooth.h>


#define MEDIA_DEBOUNCE_TIME 100
#define MEDIA_DOUBLE_TAP_INTERVAL 500

#define SCROLL_POLL_DELAY 10
#define MEDIA_POLL_DELAY 160


enum op_modes {
	MODE_SCROLL,
	MODE_MEDIA,
	MODE_COUNT,
};

int current_mode = MODE_SCROLL;
int mod_state = 0;

int64_t last_seek_time = 0;
int64_t last_mod_up_time = 0;

typedef struct {
	const struct device *hid;
	const struct device *as5600;
} devices;

devices device_state = {
	.hid = DEVICE_DT_GET_ONE(zephyr_hid_device),
	.as5600 = DEVICE_DT_GET_ANY(ams_as5600),
};


LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);
INPUT_CALLBACK_DEFINE(NULL, button_input_cb, NULL);


int main(void)
{
	LOG_INF("got to main");
	// TODO: this might be the cause of me bricking a couple of boot loaders
	// init_settings();

	int err;
	struct usbd_context usbd_ctx;

	// err = hid_init(device_state.hid, &usbd_ctx);
	// if (err != 0) {
	// 	return err;
	// }

	err = as5600_init(device_state.as5600);
	if (err != 0) {
		return err;
	}

	k_msleep(10000);
	LOG_INF("starting bt");
	k_msleep(1000);

	err = bt_init();
	if (err != 0) {
		return err;
	}

	while (true) {
		k_msleep(current_mode == MODE_SCROLL ? SCROLL_POLL_DELAY : MEDIA_POLL_DELAY);

		if (true || !hid_ready()) {
			continue;
		}

		int16_t angle = as5600_read(device_state.as5600);

		switch (current_mode) {
		case MODE_SCROLL:
			scroll_action(angle, device_state.hid);
			break;
		case MODE_MEDIA:
			media_action(angle, device_state.hid);
			break;
		};
	}

	return 0;
}


void init_settings()
{
	int size = settings_load_one(SETTINGS_MODE, &current_mode, sizeof(current_mode));
	current_mode %= MODE_COUNT;

	if (size < 0) {
		LOG_ERR("failed to load mode from settings");
	}
}


void scroll_action(int16_t angle, const struct device *hid)
{
	if (angle == 0) {
		return;
	}

	uint8_t report[MOUSE_REPORT_SIZE];
	report[MOUSE_REPORT_IDX] = MOUSE_REPORT_ID;
	report[MOUSE_ANGLE_IDX] = angle;

	submit_report("hid scroll", hid, MOUSE_REPORT_SIZE, report);
}


void media_action(int16_t angle, const struct device *hid)
{
	int action;
	uint8_t report[MEDIA_REPORT_SIZE];
	report[MEDIA_REPORT_IDX] = MEDIA_REPORT_ID;
	
	if (angle == 0) {
		return;
	}

	if (mod_state) {
		// debounce
		int64_t now = k_uptime_get();
		if (now < last_seek_time + MEDIA_DEBOUNCE_TIME) {
			last_seek_time = now;
			return;
		}

		last_seek_time = now;

		action = angle > 0
			? HID_MEDIA_SCAN_NEXT
			: HID_MEDIA_SCAN_PREV;
	} else {
		action = angle > 0
			? HID_MEDIA_VOL_UP
			: HID_MEDIA_VOL_DONW;
	}

	trigger_media_event(HID_MEDIA_PLAY_PAUSE);
}


void button_input_cb(struct input_event *evt, void *user_data)
{
	if (evt->sync == 0) {
		return;
	}

	if (evt->code == BTN_MODE_CODE) {
		if (evt->value == 0) {
			current_mode = (current_mode + 1) % MODE_COUNT;

			int err = settings_save_one(SETTINGS_MODE, &current_mode, sizeof(current_mode));
			if (err != 0) {
				LOG_ERR("Failed to save mode to settings");
			}
		}

		LOG_INF("Set mode to %s", current_mode == MODE_SCROLL ? "scroll" : "media");
		return;
	}

	if (evt->code == BTN_MOD_CODE) {
		mod_state = evt->value;
		if (current_mode == MODE_MEDIA) {
			if (evt->value) {
				return;
			}

			int64_t now = k_uptime_get();
			if (last_mod_up_time + MEDIA_DOUBLE_TAP_INTERVAL > now) {
				last_mod_up_time = 0;

				trigger_media_event(HID_MEDIA_PLAY_PAUSE);
				return;
			}

			last_mod_up_time = now;
			return;
		}

		if (current_mode == MODE_SCROLL) {
			uint8_t report[KEEB_REPORT_SIZE];
			report[KEEB_REPORT_IDX] = KEEB_REPORT_ID;
			report[KEEB_MODIFIER_IDX] = evt->value
				? HID_KBD_MODIFIER_LEFT_CTRL
				: HID_KBD_MODIFIER_NONE;

			submit_report("hid keeyboard", device_state.hid, KEEB_REPORT_SIZE, report);
		}

		return;
	}
}

void submit_report(
	const char *trigger_name,
	const struct device *dev,
	const uint16_t size,
	const uint8_t *const report
) {
	int err;

	if (usb_connected()) {
		err = hid_device_submit_report(device_state.hid, MEDIA_REPORT_SIZE, report);
		if (err) {
			LOG_ERR("failed to send %s down event: %d", trigger_name, err);
			return;
		} else {
			LOG_INF("%s event sent", trigger_name);
		}
	}

	if (bt_connected()) {
		err = bt_submit_report(size, report);
		if (err) {
			LOG_ERR("failed to send %s down event: %d", trigger_name, err);
			return;
		} else {
			LOG_INF("%s event sent", trigger_name);
		}
	}
}

void trigger_media_event(int action)
{
	int8_t report[MEDIA_REPORT_SIZE];
	report[MEDIA_REPORT_IDX] = MEDIA_REPORT_ID;
	report[MEDIA_ACTION_IDX] = action;

	submit_report("media down", device_state.hid, MEDIA_REPORT_SIZE, report);

	k_msleep(1);

	report[MEDIA_ACTION_IDX] = 0;

	submit_report("media up", device_state.hid, MEDIA_REPORT_SIZE, report);
}
