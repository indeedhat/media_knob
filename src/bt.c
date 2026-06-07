
#include <zephyr/types.h>
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
#include <zephyr/bluetooth/services/bas.h>
#include <zephyr/logging/log.h>

#include "bt.h"
#include "hid.h"
#include "hog.h"


LOG_MODULE_REGISTER(bluetooth, LOG_LEVEL_DBG);


static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(
		BT_DATA_UUID16_ALL,
		BT_UUID_16_ENCODE(BT_UUID_HIDS_VAL),
		BT_UUID_16_ENCODE(BT_UUID_BAS_VAL)
	),
};

static const struct bt_data sd[] = {
	BT_DATA(
		BT_DATA_NAME_COMPLETE,
		CONFIG_BT_DEVICE_NAME,
		sizeof(CONFIG_BT_DEVICE_NAME) - 1
	),
};


static bool is_connected;

struct bt_gatt_service_static hog_ctx;

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
	is_connected = true;

	if (bt_conn_set_security(conn, BT_SECURITY_L2)) {
		LOG_ERR("Failed to set security\n");
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
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

static void bt_ready(int err)
{
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
		return;
	}

	LOG_INF("Bluetooth initialized\n");

	hog_ctx = hog_init();

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

bool bt_connected()
{
	return is_connected;
}

int bt_submit_report(const uint16_t size, const uint8_t *const report)
{
	uint8_t tmp[size - 1];
	memcpy(tmp, report + 1, size - 1);

	if (report[0] == KEEB_REPORT_ID) {
		return bt_gatt_notify(NULL, &hog_ctx.attrs[KEEB_ATTR_IDX], tmp, size);
	} else if (report[0] == MOUSE_REPORT_ID) {
		return bt_gatt_notify(NULL, &hog_ctx.attrs[MOUSE_ATTR_IDX], tmp, size);
	} else if (report[0] == MEDIA_REPORT_ID) {
		return bt_gatt_notify(NULL, &hog_ctx.attrs[MEDIA_ATTR_IDX], tmp, size);
	}

	return 0;
}


int bt_init()
{
	int err;

	err = bt_enable(bt_ready);
	if (err) {
		return err;
	}

	if (IS_ENABLED(CONFIG_SAMPLE_BT_USE_AUTHENTICATION)) {
		bt_conn_auth_cb_register(&auth_cb_display);
		LOG_INF("Bluetooth authentication callbacks registered.\n");
	}

	return 0;
}

