#ifndef SAMPLE_USBD_H
#define SAMPLE_USBD_H

#include <zephyr/usb/usbd.h>

struct usbd_context *sample_usbd_init_device(usbd_msg_cb_t msg_cb);

#endif
