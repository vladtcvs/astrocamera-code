#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system_config.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

// Common USB options
#define USE_USB_HS 1U

#define USBD_MAX_NUM_INTERFACES 4U
#define USBD_MAX_NUM_CONFIGURATION 1U
#define USBD_MAX_STR_DESC_SIZ 512U
#define USBD_DEBUG_LEVEL 0U
#define USBD_LPM_ENABLED 0U
#define USBD_SELF_POWERED 0U
#define USBD_MAX_POWER 250U

#define USBD_SUPPORT_USER_STRING_DESC                   1U

#define CAMERA_TOTAL_INTERFACES                         0x04U

// Camera options
#define USBD_UVC_FORMAT_UNCOMPRESSED

#define CAMERA_VC_INTERFACE_ID                          0x00U
#define CAMERA_VS_INTERFACE_ID                          0x01U

#define CAMERA_UVC_MATRIX_COEFFICIENTS                  0x04U
#define CAMERA_UVC_TFR_CHARACTERISTICS                  0x01U
#define CAMERA_UVC_COLOR_PRIMARIE                       0x01U

#define CAMERA_UVC_EPIN                                 0x81U
#define CAMERA_UVC_EPIN_SIZE                            1024U
#define CAMERA_UVC_TXFIFO                               ((unsigned)(CAMERA_UVC_EPIN_SIZE/4+1))
#define UVC_CHUNK                                       512U

// HID options
#define CAMERA_HID_INTERFACE_ID                         0x02U
#define CAMERA_HID_EPIN                                 0x82U
#define CAMERA_HID_EPIN_SIZE                            16U

#define CAMERA_HID_EPOUT                                0x01U
#define CAMERA_HID_EPOUT_SIZE                           16U

#define CAMERA_HID_OUTREPORT_BUF_SIZE 3U
#define CAMERA_HID_INREPORT_BUF_SIZE 3U

// DFU options
#define CAMERA_DFU_RUNTIME_INTERFACE_ID                 0x03U
#define CAMERA_DFU_DFU_INTERFACE_ID                     0x00U
#define DFU_TRANSFER_SIZE                               256U

#define UVC_CAM_FPS_HS 2U
#define UVC_CAM_FPS_FS 1U
#define UVC_WIDTH 640U
#define UVC_HEIGHT 480U
#define UVC_BITS_PER_PIXEL 16U



/****************************************/
/* #define for FS and HS identification */
#define DEVICE_FS 0
#define DEVICE_HS 1

#define USBD_malloc (void *)USBD_static_malloc

/** Alias for memory release. */
#define USBD_free USBD_static_free

/** Alias for memory set. */
#define USBD_memset memset

/** Alias for memory copy. */
#define USBD_memcpy memcpy

/** Alias for delay. */
#define USBD_Delay HAL_Delay

/* DEBUG macros */

#if (USBD_DEBUG_LEVEL > 0)
#define USBD_UsrLog(...) \
    printf(__VA_ARGS__); \
    printf("\n");
#else
#define USBD_UsrLog(...)
#endif /* (USBD_DEBUG_LEVEL > 0U) */

#if (USBD_DEBUG_LEVEL > 1)

#define USBD_ErrLog(...) \
    printf("ERROR: ");   \
    printf(__VA_ARGS__); \
    printf("\n");
#else
#define USBD_ErrLog(...)
#endif /* (USBD_DEBUG_LEVEL > 1U) */

#if (USBD_DEBUG_LEVEL > 2)
#define USBD_DbgLog(...) \
    printf("DEBUG : ");  \
    printf(__VA_ARGS__); \
    printf("\n");
#else
#define USBD_DbgLog(...)
#endif /* (USBD_DEBUG_LEVEL > 2U) */

/* Exported functions -------------------------------------------------------*/
void *USBD_static_malloc(uint32_t size);
void USBD_static_free(void *p);

#ifdef __cplusplus
}
#endif
