#pragma once


#include <zephyr/device.h>
#include <zephyr/input/input.h>


#define SETTINGS_MODE "mode"


void scroll_action(int16_t angle, const struct device *hid);
void media_action(int16_t angle, const struct device *hid);
void button_input_cb(struct input_event *evt, void *user_data);
void trigger_media_event(int action);
void init_settings();
void submit_report(
	const char *trigger_name,
	const struct device *dev,
	const uint16_t size,
	const uint8_t *const report
);
