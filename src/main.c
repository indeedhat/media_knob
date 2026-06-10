#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/regulator.h>
#include <zephyr/input/input.h>
#include <zephyr/settings/settings.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#include "main.h"
#include "hid.h"
#include "bt.h"
#include "as5600.h"
#include "battery.h"
#include "zephyr/init.h"


static void scroll_action(int16_t angle);
static void media_action(int16_t angle);
static void button_input_cb(struct input_event *evt, void *user_data);
static void trigger_media_event(int action);
static void init_settings();
static void submit_report(const char *trigger_name, const uint16_t size, const uint8_t *const report);


int current_mode = MODE_SCROLL;
int mod_state = 0;

int64_t last_seek_time = 0;
int64_t last_mod_up_time = 0;


devices device_state = {
	.as5600 = DEVICE_DT_GET_ANY(ams_as5600),
};

static const struct gpio_dt_spec vcc_enable = {
    .port = DEVICE_DT_GET(DT_NODELABEL(gpio0)),
    .pin = 13,
    .dt_flags = GPIO_ACTIVE_HIGH,
};


LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);
INPUT_CALLBACK_DEFINE(NULL, button_input_cb, NULL);


SYS_INIT(bt_init, APPLICATION, 1);


int main(void)
{
	int err;

	gpio_pin_configure_dt(&vcc_enable, GPIO_OUTPUT_ACTIVE);

#if defined SAVE_MODE_STATE
	init_settings();
#endif


	err = init_battery_level();
	if (err) {
		LOG_ERR("Failed to init battery report loop");
	}

	err = as5600_init(device_state.as5600);
	if (err != 0) {
		return err;
	}

	while (true) {
		k_msleep(current_mode == MODE_SCROLL ? SCROLL_POLL_DELAY : MEDIA_POLL_DELAY);

		int16_t angle = as5600_read(device_state.as5600);

		switch (current_mode) {
		case MODE_SCROLL:
			scroll_action(angle);
			break;
		case MODE_MEDIA:
			media_action(angle);
			break;
		};
	}

	return 0;
}


static void init_settings()
{
	int size = settings_load_one(SETTINGS_MODE, &current_mode, sizeof(current_mode));
	current_mode %= MODE_COUNT;

	if (size < 0) {
		LOG_ERR("failed to load mode from settings");
	}
}


static void scroll_action(int16_t angle)
{
	if (angle == 0) {
		return;
	}
	LOG_INF("angle %d", angle);

	uint8_t report[MOUSE_REPORT_SIZE];
	report[MOUSE_REPORT_IDX] = MOUSE_REPORT_ID;
	report[MOUSE_ANGLE_IDX] = angle;

	submit_report("hid scroll", MOUSE_REPORT_SIZE, report);
}


static void media_action(int16_t angle)
{
	int action;
	uint8_t report[MEDIA_REPORT_SIZE];
	report[MEDIA_REPORT_IDX] = MEDIA_REPORT_ID;
	
	if (angle == 0) {
		return;
	}

	// After we skip the track we want to debounce for a short period so we
	// don't accidentally skip multiple tracks or change the volume as we
	// release the press.
	// The debounce time is purely set based on when the skip happens.
	int64_t now = k_uptime_get();
	bool should_debounce = now < last_seek_time + MEDIA_DEBOUNCE_TIME;

	if (mod_state) {
		if (should_debounce) {
			last_seek_time = now;
			return;
		}

		last_seek_time = now;

		action = angle > 0
			? HID_MEDIA_SCAN_NEXT
			: HID_MEDIA_SCAN_PREV;
	} else {
		if (should_debounce) {
			return;
		}

		action = angle > 0
			? HID_MEDIA_VOL_UP
			: HID_MEDIA_VOL_DONW;
	}

	trigger_media_event(action);
}


static void button_input_cb(struct input_event *evt, void *user_data)
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

			submit_report("hid keeyboard",KEEB_REPORT_SIZE, report);
		}

		return;
	}
}


static void submit_report(
	const char *trigger_name,
	const uint16_t size,
	const uint8_t *const report
) {
	int err;

	if (bt_connected()) {
		err = bt_submit_report(size, report);
		if (err) {
			LOG_ERR("failed to send %s event: %d", trigger_name, err);
			return;
		} else {
			LOG_DBG("%s event sent", trigger_name);
		}
	}
}


static void trigger_media_event(int action)
{
	int8_t report[MEDIA_REPORT_SIZE];
	report[MEDIA_REPORT_IDX] = MEDIA_REPORT_ID;
	report[MEDIA_ACTION_IDX] = action;

	submit_report("media down", MEDIA_REPORT_SIZE, report);

	k_msleep(1);

	report[MEDIA_ACTION_IDX] = 0;

	submit_report("media up", MEDIA_REPORT_SIZE, report);
}
