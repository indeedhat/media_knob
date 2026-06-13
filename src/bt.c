
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
#include "zephyr/bluetooth/addr.h"


static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);
static void bt_ready(int err);
static void auth_cancel(struct bt_conn *conn);
static void pairing_complete(struct bt_conn *conn, bool bonded);
static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason);
static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err);
static void advertise();
static void advertise_worker_cb(struct k_work *work);
static void unpair_worker_cb(struct k_work *work);
static void le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout);
static enum bt_security_err auth_pairing_accept(struct bt_conn *conn, const struct bt_conn_pairing_feat *const feat);


LOG_MODULE_REGISTER(bluetooth, LOG_LEVEL_DBG);

K_WORK_DEFINE(advertise_worker, advertise_worker_cb);
K_WORK_DEFINE(unpair_worker, unpair_worker_cb);


static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(
        BT_DATA_GAP_APPEARANCE,
        (CONFIG_BT_DEVICE_APPEARANCE & 0xff),
        (CONFIG_BT_DEVICE_APPEARANCE >> 8)
    ),
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


static const bt_addr_le_t *unpair_addr;
static bool is_connected;

struct bt_gatt_service_static hog_ctx;


static struct bt_conn_cb conn_cb = {
	.connected = connected,
	.disconnected = disconnected,
	.security_changed = security_changed,
	.le_param_updated = le_param_updated,
};

static struct bt_conn_auth_info_cb auth_info_cb = {
    .pairing_complete = pairing_complete,
    .pairing_failed = pairing_failed,
};

static struct bt_conn_auth_cb auth_cb_display = {
	.pairing_accept = auth_pairing_accept,
	.cancel = auth_cancel,
};


int bt_init()
{
	int err;

	err = bt_enable(bt_ready);
	if (err) {
		return err;
	}

    bt_conn_cb_register(&conn_cb);
    bt_conn_auth_cb_register(&auth_cb_display);
	bt_conn_auth_info_cb_register(&auth_info_cb);

	return 0;
}


bool bt_connected()
{
	return is_connected;
}


int bt_submit_report(const uint16_t size, const uint8_t *const report)
{
	uint8_t tmp[size - 1];
	memcpy(tmp, report + 1, size - 1);

	if (report[0] == KEEB_REPORT_ID) {
		return bt_gatt_notify(NULL, &hog_ctx.attrs[BT_KEEB_ATTR_IDX], tmp, size - 1);
	} else if (report[0] == MOUSE_REPORT_ID) {
		return bt_gatt_notify(NULL, &hog_ctx.attrs[BT_MOUSE_ATTR_IDX], tmp, size - 1);
	} else if (report[0] == MEDIA_REPORT_ID) {
		return bt_gatt_notify(NULL, &hog_ctx.attrs[BT_MEDIA_ATTR_IDX], tmp, size - 1);
	}

	return 0;
}


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

	// macOS and Windows expect the peripheral to initiate encryption; without
	// this call both platforms stall because GATT attrs require BT_SECURITY_L2.
	int sec_err = bt_conn_set_security(conn, BT_SECURITY_L2);
	if (sec_err) {
		LOG_ERR("Security request failed (err %d)\n", sec_err);
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	LOG_INF("Disconnected from %s, reason 0x%02x %s\n",
		bt_conn_dst_str(conn),
		reason,
		bt_hci_err_to_str(reason)
	);

	is_connected = false;

	k_work_submit(&advertise_worker);
}


static void security_changed(
	struct bt_conn *conn,
	bt_security_t level,
	enum bt_security_err err
) {
	if (!err) {
		LOG_INF("Security changed: %s level %u\n", bt_conn_dst_str(conn), level);
	} else {
		LOG_ERR("Security failed: level %u err=%d",
			level, (int)err
		);
	}
}


static void bt_ready(int err)
{
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
		return;
	}

	LOG_INF("Bluetooth initialized\n");

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();

#if defined(CONFIG_KNOBLET_BT_CLEAR_BONDS_ON_START)
		LOG_INF("clearing bt pairs");
		bt_unpair(BT_ID_DEFAULT, NULL);
#endif
	}

	hog_ctx = hog_init();

	advertise();
}


static void auth_cancel(struct bt_conn *conn)
{
	LOG_INF("Pairing cancelled: %s\n", bt_conn_dst_str(conn));
}


static void pairing_complete(struct bt_conn *conn, bool bonded)
{
    LOG_INF("Pairing completed. bonded=%d", bonded);
}


static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
    LOG_ERR("Pairing failed. reason=%d", reason);

	// if (reason == 4) {
	// 	unpair_addr = bt_conn_get_dst(conn);

	// 	k_work_submit(&unpair_worker);
	// }
}


static void advertise()
{
	// Don't care if this fails
	bt_le_adv_stop();

	int err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return;
	}

	LOG_INF("Advertising successfully started\n");
}


static void advertise_worker_cb(struct k_work *work)
{
    advertise();
}


static void le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency,
                             uint16_t timeout) {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG_DBG("%s: interval %d latency %d timeout %d", addr, interval, latency, timeout);
}


static enum bt_security_err auth_pairing_accept(struct bt_conn *conn, const struct bt_conn_pairing_feat *const feat) {
    struct bt_conn_info info;
    bt_conn_get_info(conn, &info);

    LOG_DBG("role %d", info.role);

    return BT_SECURITY_ERR_SUCCESS;
};


static void unpair_worker_cb(struct k_work *work)
{
	if (unpair_addr == NULL) {
		return;
	}

	LOG_INF("unpairing");
    bt_unpair(BT_ID_DEFAULT, unpair_addr);
	unpair_addr = NULL;
}
