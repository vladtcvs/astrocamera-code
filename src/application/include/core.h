#pragma once

#include <stdbool.h>

void core_set_usbctx(struct usb_context_s *ctx);

void core_sensors_poll_function(void *ctx);
