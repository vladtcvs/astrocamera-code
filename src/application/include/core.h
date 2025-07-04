#pragma once

#include <stdbool.h>


void core_init(struct usb_context_s *ctx);

void core_sensors_poll_function(void *ctx);

void core_read_ccd_completed_cb(void);

void core_process_exposure_cb(int exposure);
void core_process_exposure_mode_cb(int mode);
void core_process_target_temperature_cb(int target_temperature);
void core_process_window_heater_cb(int window_heater);
void core_process_fan_cb(bool fan);
void core_process_tec_cb(bool tec);
