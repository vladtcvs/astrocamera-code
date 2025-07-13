#pragma once

#include <unistd.h>
#include <stdint.h>

size_t camera_generate_descriptor(uint8_t *pConf,
                                  uint8_t fps,
                                  uint16_t width,
                                  uint16_t height,
                                  const char *FourCC,
                                  size_t maxlen);

size_t camera_hid_report_descriptor(uint8_t *pConf, size_t maxlen);

const void *camera_get_video_descriptor(size_t *len);
const void *camera_get_dfu_descriptor(size_t *len);

void camera_fill_probe_control(uint8_t *probe, uint16_t width, uint16_t height);
