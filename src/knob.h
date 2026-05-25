#pragma once

#include <zephyr/usb/class/hid.h>
#include <zephyr/usb/class/usbd_hid.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/logging/log.h>


#define DEAD_ZONE 1

#define KEEB_REPORT_ID   0x01
#define MOUSE_REPORT_ID  0x02
#define MEDIA_REPORT_ID  0x03

#define HID_MEDIA_SCAN_NEXT  0xB5
#define HID_MEDIA_SCAN_PREV  0xB6
#define HID_MEDIA_PLAY_PAUSE 0xCD
#define HID_MEDIA_VOL_UP     0xE9
#define HID_MEDIA_VOL_DONW   0xEA

#define HID_LEFT_CTRL 0xE0
#define HID_RIGHT_GUI 0xE7


static const uint8_t hid_report_desc[] = 	{

	/*
	 * Keyboard
	 */
	HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),
	HID_USAGE(HID_USAGE_GEN_DESKTOP_KEYBOARD),
	HID_COLLECTION(HID_COLLECTION_APPLICATION),
		HID_REPORT_ID(KEEB_REPORT_ID),
		
		// Modifier keys
		HID_USAGE_PAGE(HID_USAGE_GEN_KEYBOARD),
		HID_USAGE_MIN8(HID_LEFT_CTRL),
		HID_USAGE_MAX8(HID_RIGHT_GUI),
		HID_LOGICAL_MIN8(0),
		HID_LOGICAL_MAX8(1),
		HID_REPORT_SIZE(1),
		HID_REPORT_COUNT(8),
		HID_INPUT(0x02),  // (Data, Var, Abs)

		// Reserved byte, not really sure why i need this but apparently i do
		HID_REPORT_SIZE(8),
		HID_REPORT_COUNT(1),
		HID_INPUT(0x3),  // Const, Ary, Abs
		
		// Key codes (not needed if only using modifiers)
		HID_LOGICAL_MIN8(0),
		HID_LOGICAL_MAX8(101),
		HID_REPORT_SIZE(8),
		HID_REPORT_COUNT(6),
		HID_INPUT(0x00),  // Data, Ary, Abs
	HID_END_COLLECTION,

	/*
	 * Mouse
	 */
	HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),
	HID_USAGE(HID_USAGE_GEN_DESKTOP_MOUSE),
	HID_COLLECTION(HID_COLLECTION_APPLICATION),
		HID_REPORT_ID(MOUSE_REPORT_ID),
		HID_USAGE(HID_USAGE_GEN_DESKTOP_POINTER),
		HID_COLLECTION(HID_COLLECTION_PHYSICAL),
			// high res scroll
			HID_COLLECTION(HID_COLLECTION_LOGICAL),
				HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),

				// RESOLUTION_MULTIPLIER
				HID_USAGE(0x48),
				HID_LOGICAL_MIN8(0),
				HID_LOGICAL_MAX8(15),
				HID_ITEM(HID_ITEM_TAG_PHYSICAL_MIN, HID_ITEM_TYPE_GLOBAL, 1), 1,
				HID_ITEM(HID_ITEM_TAG_PHYSICAL_MAX, HID_ITEM_TYPE_GLOBAL, 1), 16,
				HID_REPORT_SIZE(8),
				HID_REPORT_COUNT(1),
				HID_FEATURE(0x02), // (Const, Var, Rel)

				// Scroll Position
				HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),
				HID_USAGE(HID_USAGE_GEN_DESKTOP_WHEEL),
				HID_LOGICAL_MIN8(-127),
				HID_LOGICAL_MAX8(127),
				HID_REPORT_COUNT(1),
				HID_REPORT_SIZE(8),
				HID_INPUT(0x06), // (Data, Var, Rel)
			HID_END_COLLECTION,
		HID_END_COLLECTION,
	HID_END_COLLECTION,

	/*
	 * Media Controls
	 */
	HID_USAGE_PAGE(0x0C), // (Consumer Page)
	HID_USAGE(0x01), // (Consumer)
	HID_COLLECTION(HID_COLLECTION_APPLICATION),
		HID_REPORT_ID(MEDIA_REPORT_ID),
		HID_LOGICAL_MIN8(0),
		HID_LOGICAL_MAX16(0x3C, 0x02), // (572)
		HID_REPORT_SIZE(16),
		HID_REPORT_COUNT(1),
		HID_INPUT(0x0), // (Data, Ary, Abs)
	HID_END_COLLECTION
};


enum mouse_report_idx {
	MOUSE_REPORT_IDX,
	MOUSE_ANGLE_IDX,
	MOUSE_REPORT_SIZE,
};

enum keeb_report_idx {
	KEEB_REPORT_IDX,
	KEEB_MODIFIER_IDX,
	KEEB_KEY_0_IDX,
	KEEB_KEY_1_IDX,
	KEEB_KEY_2_IDX,
	KEEB_KEY_3_IDX,
	KEEB_KEY_4_IDX,
	KEEB_KEY_5_IDX,
	KEEB_REPORT_SIZE,
};

enum media_report_idx {
	MEDIA_REPORT_IDX,
	MEDIA_ACTION_IDX,
	MEDIA_REPORT_SIZE,
};


struct as5600_dev_data {
	int16_t position;
};


int hid_init(const struct device *dev, struct usbd_context *ctx);
bool hid_ready();

int as5600_init(const struct device* dev);
int as5600_read(const struct device *dev);
