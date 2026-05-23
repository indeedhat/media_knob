
#include <zephyr/device.h>
#include <sys/errno.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/sensor.h>


#define SLEEP_TIME_MS   10


int main(void)
{
	printk("app started");

	const struct device *as5600 = DEVICE_DT_GET_ANY(ams_as5600);
	if (as5600 == NULL) {
		printk("AS5600 not found");
		return -ENODEV;
	}

	printk("AS5600 found");

	// setup the rotation sensor
	if (!device_is_ready(as5600)) {
		printk("AS5600 not ready\n");
		return -ENODEV;
	}

	printk("AS5600 ready");

	while (1) {
		struct sensor_value angle;
		sensor_sample_fetch(as5600);
		sensor_channel_get(as5600, SENSOR_CHAN_ROTATION, &angle);
		printk("Angle: %d.%06d degrees\n", angle.val1, angle.val2);

		k_msleep(SLEEP_TIME_MS);
	}

	return 0;
}
