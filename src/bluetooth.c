#include "hid.h"

#include <zephyr/types.h>
#include <zephyr/drivers/gpio.h>
#include <stddef.h>
#include <string.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>

#include <zephyr/settings/settings.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/logging/log.h>


LOG_MODULE_REGISTER(bluetooth, LOG_LEVEL_DBG);


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
	.version = 0x0000,
	.code = 0x00,
	.flags = HIDS_NORMALLY_CONNECTABLE,
};


enum {
	HIDS_INPUT = 0x01,
	HIDS_OUTPUT = 0x02,
	HIDS_FEATURE = 0x03,
};


static struct hids_report input = {
	.id = 0x01,
	.type = HIDS_INPUT,
};

static uint8_t simulate_input;
static uint8_t ctrl_point;
static bool is_connected;


static ssize_t read_info(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	void *buf,
	uint16_t len,
	uint16_t offset
) {
	return bt_gatt_attr_read(conn, attr, buf, len, offset, attr->user_data, sizeof(struct hids_info));
}


static ssize_t read_report_map(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	void *buf,
	uint16_t len,
	uint16_t offset
) {
	return bt_gatt_attr_read(conn, attr, buf, len, offset, hid_report_desc, sizeof(hid_report_desc));
}


static ssize_t read_report(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	void *buf,
	uint16_t len,
	uint16_t offset
) {
	return bt_gatt_attr_read(conn, attr, buf, len, offset, attr->user_data, sizeof(struct hids_report));
}


static void input_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	simulate_input = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}


static ssize_t read_input_report(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	void *buf,
	uint16_t len,
	uint16_t offset
) {
	return bt_gatt_attr_read(conn, attr, buf, len, offset, NULL, 0);
}


static ssize_t write_ctrl_point(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	const void *buf, uint16_t len,
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


#if CONFIG_SAMPLE_BT_USE_AUTHENTICATION
/* Require encryption using authenticated link-key. */
#define SAMPLE_BT_PERM_READ BT_GATT_PERM_READ_AUTHEN
#define SAMPLE_BT_PERM_WRITE BT_GATT_PERM_WRITE_AUTHEN
#else
/* Require encryption. */
#define SAMPLE_BT_PERM_READ BT_GATT_PERM_READ_ENCRYPT
#define SAMPLE_BT_PERM_WRITE BT_GATT_PERM_WRITE_ENCRYPT
#endif


/* HID Service Declaration */
BT_GATT_SERVICE_DEFINE(hog_svc,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_HIDS),
	BT_GATT_CHARACTERISTIC(
		BT_UUID_HIDS_INFO,
		BT_GATT_CHRC_READ,
		BT_GATT_PERM_READ,
		read_info,
		NULL,
		&info
	),
	BT_GATT_CHARACTERISTIC(
		BT_UUID_HIDS_REPORT_MAP, BT_GATT_CHRC_READ,
		BT_GATT_PERM_READ,
		read_report_map,
		NULL,
		NULL
	),
	BT_GATT_CHARACTERISTIC(
		BT_UUID_HIDS_REPORT,
		BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
		SAMPLE_BT_PERM_READ,
		read_input_report,
		NULL,
		NULL
	),
	BT_GATT_CCC(
		input_ccc_changed,
		SAMPLE_BT_PERM_READ | SAMPLE_BT_PERM_WRITE
	),
	BT_GATT_DESCRIPTOR(
		BT_UUID_HIDS_REPORT_REF,
		BT_GATT_PERM_READ,
		read_report,
		NULL,
		&input
	),
	BT_GATT_CHARACTERISTIC(
		BT_UUID_HIDS_CTRL_POINT,
		BT_GATT_CHRC_WRITE_WITHOUT_RESP,
		BT_GATT_PERM_WRITE,
		NULL,
		write_ctrl_point,
		&ctrl_point
	),
);


static void hog_init()
{
}


static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL,
		      BT_UUID_16_ENCODE(BT_UUID_HIDS_VAL),
		      BT_UUID_16_ENCODE(BT_UUID_BAS_VAL)),
};


static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};


static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		LOG_ERR("Failed to connect to %s, err 0x%02x %s\n",
			bt_conn_dst_str(conn),
			err,
			bt_hci_err_to_str(err)
	  	);
		return;
	}

	LOG_INF("Connected %s\n", bt_conn_dst_str(conn));

	if (bt_conn_set_security(conn, BT_SECURITY_L2)) {
		LOG_ERR("Failed to set security\n");
		is_connected = false;
	} else {
		is_connected = true;
	}
}


static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	is_connected = false;

	LOG_INF("Disconnected from %s, reason 0x%02x %s\n",
		bt_conn_dst_str(conn),
		reason,
		bt_hci_err_to_str(reason)
	);
}


static void security_changed(
	struct bt_conn *conn,
	bt_security_t level,
	enum bt_security_err err
) {
	if (!err) {
		LOG_INF("Security changed: %s level %u\n", bt_conn_dst_str(conn), level);
	} else {
		LOG_ERR("Security failed: %s level %u err %s(%d)\n",
			bt_conn_dst_str(conn),
			level,
			bt_security_err_to_str(err),
			err
		);
	}
}


BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
	.security_changed = security_changed,
};


void bt_ready(int err)
{
	LOG_INF("Got to bt_ready");
	return;

	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
		return;
	}

	LOG_INF("Bluetooth initialized\n");

	hog_init();

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return;
	}

	LOG_INF("Advertising successfully started\n");
}


static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
	LOG_INF("Passkey for %s: %06u\n", bt_conn_dst_str(conn), passkey);
}


static void auth_cancel(struct bt_conn *conn)
{
	LOG_INF("Pairing cancelled: %s\n", bt_conn_dst_str(conn));
}


static struct bt_conn_auth_cb auth_cb_display = {
	.passkey_display = auth_passkey_display,
	.passkey_entry = NULL,
	.cancel = auth_cancel,
};


void bt_register_auth()
{
	if (IS_ENABLED(CONFIG_SAMPLE_BT_USE_AUTHENTICATION)) {
		bt_conn_auth_cb_register(&auth_cb_display);
		printk("Bluetooth authentication callbacks registered.\n");
	}
}


bool bt_connected()
{
	return is_connected;
}

int bt_submit_report(const uint16_t size, const uint8_t *const report)
{
	return bt_gatt_notify(NULL, &hog_svc.attrs[5], report, sizeof(report));
}

int bt_init()
{
	LOG_INF("got to bt_init");
	int err = bt_enable(bt_ready);
	if (err != 0) {
		return err;
	}

	bt_register_auth();
	return 0;
}
