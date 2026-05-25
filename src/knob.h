#pragma once

#include <zephyr/usb/class/hid.h>
#include <zephyr/usb/class/usbd_hid.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/logging/log.h>



#define DEAD_ZONE 0


static const uint8_t hid_report_desc[] = 	{
	HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),
	HID_USAGE(HID_USAGE_GEN_DESKTOP_MOUSE),
	HID_COLLECTION(HID_COLLECTION_APPLICATION),
		HID_USAGE(HID_USAGE_GEN_DESKTOP_POINTER),
		HID_COLLECTION(HID_COLLECTION_PHYSICAL),
			HID_USAGE_PAGE(HID_USAGE_GEN_BUTTON),
			HID_USAGE_MIN8(1),
			HID_USAGE_MAX8(1),
			HID_LOGICAL_MIN8(0),
			HID_LOGICAL_MAX8(1),
			HID_REPORT_SIZE(1),
			HID_REPORT_COUNT(1),
			// HID_INPUT (Data,Var,Abs)
			HID_INPUT(0x02),
			// Unused bits
			HID_REPORT_SIZE(7),
			HID_REPORT_COUNT(1),
			// HID_INPUT (Cnst,Ary,Abs)
			HID_INPUT(1),

			// high res scroll
			HID_COLLECTION(HID_COLLECTION_LOGICAL),
				HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),
				HID_USAGE(0x48), // RESOLUTION_MULTIPLIER
				HID_LOGICAL_MIN8(0),
				HID_LOGICAL_MAX8(15),
				HID_ITEM(HID_ITEM_TAG_PHYSICAL_MIN, HID_ITEM_TYPE_GLOBAL, 1), 1,
				HID_ITEM(HID_ITEM_TAG_PHYSICAL_MAX, HID_ITEM_TYPE_GLOBAL, 1), 16,
				HID_REPORT_SIZE(8),
				HID_REPORT_COUNT(1),
				// HID_FEATURE (Const,Var,Rel)
				HID_FEATURE(0x02),

				HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),
				HID_USAGE(HID_USAGE_GEN_DESKTOP_WHEEL),
				HID_LOGICAL_MIN8(-127),
				HID_LOGICAL_MAX8(127),
				HID_REPORT_COUNT(1),
				HID_REPORT_SIZE(8),
				// HID_INPUT (Data,Var,Rel)
				HID_INPUT(0x06),
			HID_END_COLLECTION,
		HID_END_COLLECTION,
	HID_END_COLLECTION,
};


enum mouse_report_idx {
	MOUSE_BTN_REPORT_IDX = 0,
	MOUSE_WHEEL_REPORT_IDX = 1,
	MOUSE_REPORT_COUNT = 2,
};


struct as5600_dev_data {
	int16_t position;
};


int mouse_init(const struct device *dev, struct usbd_context *ctx);
bool mouse_ready();
int as5600_init(const struct device* dev);
int as5600_read(const struct device *dev);
