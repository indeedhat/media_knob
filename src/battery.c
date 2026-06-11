
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/bluetooth/services/bas.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>

#include "battery.h"


static void update_cb(struct k_work *work);
static int read_battery_percent();


static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));
static K_WORK_DELAYABLE_DEFINE(battery_update_work, update_cb);


int init_battery_level()
{
	int err = k_work_schedule(&battery_update_work, K_NO_WAIT);
	if (err <= 1) {
		return 0;
	}

	return err;
};


static void update_cb(struct k_work *work)
{
	int level = read_battery_percent();
	if (level >= 0) {
		bt_bas_set_battery_level(level);
	}

	k_work_schedule(&battery_update_work, K_SECONDS(BAS_POLL_INTERVAL_S));
}


static int read_battery_percent()
{
	int16_t sample;
	int32_t mv;

	struct adc_sequence seq = {
		.buffer = &sample,
		.buffer_size = sizeof(sample),
	};

	adc_sequence_init_dt(&adc_channel, &seq);
	adc_channel_setup_dt(&adc_channel);
	adc_read(adc_channel.dev, &seq);

	mv = sample;
	adc_raw_to_millivolts(adc_ref_internal(adc_channel.dev), ADC_GAIN_1_6, 12, &mv);
	mv *= BAS_VOLTAGE_DIVIDER;

	if (mv > BAS_USB_CONNECTED_MV) {
		return -1;
	}

	mv = CLAMP(mv, BAS_CLAMP_MIN, BAS_CLAMP_MAX);
	return (mv - BAS_CLAMP_MIN) * 100 / (BAS_CLAMP_MAX - BAS_CLAMP_MIN);
}



