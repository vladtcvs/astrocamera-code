#pragma once

#include <stdint.h>
#include "usbd_def.h"

typedef struct
{
  int8_t (* Init)(void);
  int8_t (* DeInit)(void);
} USBD_CAMERA_ItfTypeDef;



uint8_t USBD_CAMERA_Configure(unsigned fps, unsigned width, unsigned height, const char *FourCC);

extern USBD_ClassTypeDef    USBD_CAMERA;
