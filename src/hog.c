/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <zephyr/drivers/gpio.h>
#include <stddef.h>
#include <string.h>

#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/logging/log.h>

#include "hid.h"
#include "zephyr/bluetooth/att.h"


LOG_MODULE_REGISTER(bt_hog, LOG_LEVEL_DBG);


#define RESOLUTION_MULTIPLIER 0x0F
#define PROTOCOL_MODE 0x01


enum {
	HIDS_REMOTE_WAKE = BIT(0),
	HIDS_NORMALLY_CONNECTABLE = BIT(1),
};

struct hids_info {
	uint16_t version; /* version number of base USB HID Specification */
	uint8_t code; /* country HID Device hardware is localized for. */
	uint8_t flags;
} __packed;

struct hids_report {
	uint8_t id; /* report id */
	uint8_t type; /* report type */
} __packed;

static struct hids_info info = {
	.version = 0x0111,
	.code = 0x00,
	.flags = HIDS_NORMALLY_CONNECTABLE,
};

enum {
	HIDS_INPUT = 0x01,
	HIDS_OUTPUT = 0x02,
	HIDS_FEATURE = 0x03,
};


static struct hids_report keeb_input  = { .id = KEEB_REPORT_ID,  .type = HIDS_INPUT };
static struct hids_report media_input = { .id = MEDIA_REPORT_ID, .type = HIDS_INPUT };
static struct hids_report mouse_input = { .id = MOUSE_REPORT_ID, .type = HIDS_INPUT };
static struct hids_report mouse_feat  = { .id = MOUSE_REPORT_ID, .type = HIDS_FEATURE };

static uint8_t mouse_enabled;
static uint8_t keeb_enabled;
static uint8_t media_enabled;
static uint8_t macro_enabled;

static uint8_t ctrl_point;

static uint8_t resolution_multiplier = RESOLUTION_MULTIPLIER;
static uint8_t proto_mode = PROTOCOL_MODE;


static ssize_t read_feature_report(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
static ssize_t write_feature_report(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags);
static ssize_t read_info(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
static ssize_t read_report_map(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
static ssize_t read_input_report(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
static ssize_t read_report(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
static ssize_t write_ctrl_point(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags);
static void mouse_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value);
static void keeb_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value);
static void media_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value);
static void macro_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value);
static ssize_t read_proto_mode(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
static ssize_t write_proto_mode(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags);


/* Require encryption. */
#define SAMPLE_BT_PERM_READ BT_GATT_PERM_READ_ENCRYPT
#define SAMPLE_BT_PERM_WRITE BT_GATT_PERM_WRITE_ENCRYPT

#define MACRO_SERVICE_UUID
#define MACRO_CHARACTERISTIC_UUID

/* UUID's for my custom macro service/characteristic */
#define BT_UUID_MACRO_SERVICE_VAL \
    BT_UUID_128_ENCODE(0xF28EC1C1, 0x7BE9, 0x4776, 0xB063, 0x578239F29BFF)
#define BT_UUID_MACRO_CHARACTERISTIC_VAL \
    BT_UUID_128_ENCODE(0x29389447, 0xCCD7, 0x4E89, 0xB8C6, 0x3437651B5488)

#define BT_UUID_MACRO_SERVICE \
    BT_UUID_DECLARE_128(BT_UUID_MACRO_SERVICE_VAL)
#define BT_UUID_MACRO_CHARACTERISTIC \
    BT_UUID_DECLARE_128(BT_UUID_MACRO_CHARACTERISTIC_VAL)

/* HID Service Declaration */
BT_GATT_SERVICE_DEFINE(hog_svc,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_HIDS),
	BT_GATT_CHARACTERISTIC(
		BT_UUID_HIDS_PROTOCOL_MODE,
		BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
		BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
		read_proto_mode, write_proto_mode, &proto_mode
	),
	BT_GATT_CHARACTERISTIC(
		BT_UUID_HIDS_INFO,
		BT_GATT_CHRC_READ,
		BT_GATT_PERM_READ,
		read_info,
		NULL,
		&info
	),
	BT_GATT_CHARACTERISTIC(
		 BT_UUID_HIDS_REPORT_MAP,
		 BT_GATT_CHRC_READ,
		 BT_GATT_PERM_READ,
		read_report_map,
		NULL,
		NULL
	),

	/* Keyboard */
    BT_GATT_CHARACTERISTIC(
		BT_UUID_HIDS_REPORT,
		BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
		SAMPLE_BT_PERM_READ,
		read_input_report,
		NULL,
		NULL
	),
    BT_GATT_CCC(keeb_ccc_changed, SAMPLE_BT_PERM_READ | SAMPLE_BT_PERM_WRITE),
    BT_GATT_DESCRIPTOR(
		BT_UUID_HIDS_REPORT_REF,
		SAMPLE_BT_PERM_READ,
		read_report,
		NULL,
		&keeb_input
	),

    /* Mouse */
    BT_GATT_CHARACTERISTIC(
		BT_UUID_HIDS_REPORT,
		BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
		SAMPLE_BT_PERM_READ,
		read_input_report,
		NULL,
		NULL
	),
    BT_GATT_CCC(mouse_ccc_changed, SAMPLE_BT_PERM_READ | SAMPLE_BT_PERM_WRITE),
    BT_GATT_DESCRIPTOR(
		BT_UUID_HIDS_REPORT_REF,
		SAMPLE_BT_PERM_READ,
		read_report,
		NULL,
		&mouse_input
	),
	BT_GATT_CHARACTERISTIC(
        BT_UUID_HIDS_REPORT,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
        SAMPLE_BT_PERM_READ | SAMPLE_BT_PERM_WRITE,
        read_feature_report,
        write_feature_report,
        &resolution_multiplier
    ),
    BT_GATT_DESCRIPTOR(
        BT_UUID_HIDS_REPORT_REF,
        SAMPLE_BT_PERM_READ,
        read_report,
        NULL,
        &mouse_feat
    ),

    /* Media */
    BT_GATT_CHARACTERISTIC(
		 BT_UUID_HIDS_REPORT,
		 BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
		 SAMPLE_BT_PERM_READ,
		 read_input_report,
		 NULL,
		 NULL
	),
    BT_GATT_CCC(media_ccc_changed, SAMPLE_BT_PERM_READ | SAMPLE_BT_PERM_WRITE),
    BT_GATT_DESCRIPTOR(
		BT_UUID_HIDS_REPORT_REF,
		SAMPLE_BT_PERM_READ,
		read_report,
		NULL,
		&media_input
	),

	BT_GATT_CHARACTERISTIC(
		BT_UUID_HIDS_CTRL_POINT,
		BT_GATT_CHRC_WRITE_WITHOUT_RESP,
		BT_GATT_PERM_WRITE,
		NULL, write_ctrl_point, &ctrl_point
	),

	/* Custom Macro Service */
	BT_GATT_SECONDARY_SERVICE(BT_UUID_MACRO_SERVICE),
	BT_GATT_CHARACTERISTIC(
		BT_UUID_MACRO_CHARACTERISTIC,
		BT_GATT_CHRC_NOTIFY,
		BT_GATT_PERM_NONE,
		NULL,
		NULL,
		NULL
	),
	BT_GATT_CCC(
		macro_ccc_changed,
		BT_GATT_PERM_READ | BT_GATT_PERM_WRITE
	),
	// TODO: define report
);


static ssize_t read_info(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	void *buf,
	uint16_t len,
	uint16_t offset
) {
	LOG_INF("Getting to read_info");
    LOG_INF("read_info called, security level: %d", bt_conn_get_security(conn));
	return bt_gatt_attr_read(
		conn,
		attr,
		buf,
		len,
		offset,
		attr->user_data,
		sizeof(struct hids_info)
	);
}


static ssize_t read_report_map(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	void *buf,
	uint16_t len,
	uint16_t offset
) {
	LOG_INF("Getting to read_report_map");
    LOG_INF("read_report_map called, security level: %d", bt_conn_get_security(conn));
	return bt_gatt_attr_read(
		conn,
		attr,
		buf,
		len,
		offset,
		hid_report_desc,
		sizeof(hid_report_desc)
	);
}

static ssize_t read_report(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	void *buf,
	uint16_t len,
	uint16_t offset
) {
	LOG_INF("Getting to read_report");
    LOG_INF("read_report called, security level: %d", bt_conn_get_security(conn));
	return bt_gatt_attr_read(
		conn,
		attr,
		buf,
		len,
		offset,
		attr->user_data,
		sizeof(struct hids_report)
	);
}


static void mouse_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	mouse_enabled = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
	LOG_INF("mouse_ccc_changed %d", mouse_enabled);
}


static void keeb_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	keeb_enabled = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
	LOG_INF("keeb_ccc_changed %d", keeb_enabled);
}


static void media_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	media_enabled = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
	LOG_INF("media_ccc_changed %d", media_enabled);
}

static void macro_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	macro_enabled = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
	LOG_INF("macro_ccc_changed %d", macro_enabled);
}


static ssize_t read_input_report(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	void *buf,
	uint16_t len,
	uint16_t offset
) {
	LOG_INF("Getting to read_input_report");
    LOG_INF("read_input_report called, security level: %d", bt_conn_get_security(conn));
	return bt_gatt_attr_read(conn, attr, buf, len, offset, NULL, 0);
}


static ssize_t write_ctrl_point(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	const void *buf,
	uint16_t len,
	uint16_t offset,
	uint8_t flags
) {
	uint8_t *value = attr->user_data;

	if (offset + len > sizeof(ctrl_point)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);

	return len;
}


static ssize_t read_feature_report(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	void *buf,
	uint16_t len,
	uint16_t offset
) {
	LOG_INF("READ_FEATURE: multiplier set to max");
	return bt_gatt_attr_read(
		conn,
		attr,
		buf,
		len,
		offset,
		&resolution_multiplier,
		sizeof(resolution_multiplier)
	);
}


static ssize_t write_feature_report(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	const void *buf,
	uint16_t len,
	uint16_t offset,
	uint8_t flags
) {
	LOG_INF("WRITE_FEATURE: device reported val(%d) len(%d),", ((const uint8_t*)buf)[0], len);
    return len;
}


const struct bt_gatt_service_static hog_init()
{
	return hog_svc;
}


static ssize_t read_proto_mode(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	void *buf,
	uint16_t len,
	uint16_t offset
) {
	LOG_INF("write_proto_mode");
    return bt_gatt_attr_read(conn, attr, buf, len, offset, attr->user_data, sizeof(uint8_t));
}


static ssize_t write_proto_mode(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	const void *buf,
	uint16_t len,
	uint16_t offset,
	uint8_t flags
) {
	LOG_INF("write_proto_mode");
    if (len != 1) return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    proto_mode = ((uint8_t *)buf)[0];
    return len;
}
