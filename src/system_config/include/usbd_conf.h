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

#define USBD_MAX_NUM_INTERFACES 1U
#define USBD_MAX_NUM_CONFIGURATION 1U
#define USBD_MAX_STR_DESC_SIZ 512U
#define USBD_DEBUG_LEVEL 0U
#define USBD_LPM_ENABLED 0U
#define USBD_SELF_POWERED 0U
#define USBD_MAX_POWER 250U

// Test options
#define HID_EPIN_ADDR            0x83U

// Composite options
#define USE_USBD_COMPOSITE

#define USBD_CMPSIT_ACTIVATE_VIDEO 1U
#define USBD_CMPSIT_ACTIVATE_CUSTOMHID 1U
#define USBD_CMPSIT_ACTIVATE_HID 0U

#define USBD_MAX_CLASS_INTERFACES 3U
#define USBD_MAX_SUPPORTED_CLASS 4U


// Custom HID options
#define CUSTOM_HID_EPIN_ADDR  0x82
#define CUSTOM_HID_EPOUT_ADDR 0x02

#define CUSTOM_HID_HS_BINTERVAL 0x07U
#define CUSTOM_HID_FS_BINTERVAL 0x0AU

#define USBD_CUSTOM_HID_REPORT_DESC_SIZE            181U

#define USBD_CUSTOMHID_CTRL_REQ_GET_REPORT_ENABLED
#define USBD_CUSTOMHID_CTRL_REQ_COMPLETE_CALLBACK_ENABLED
#define USBD_CUSTOMHID_REPORT_BUFFER_EVENT_ENABLED

#define CUSTOM_HID_EPIN_SIZE    0x02U
#define CUSTOM_HID_EPOUT_SIZE   0x02U
#define USBD_CUSTOMHID_OUTREPORT_BUF_SIZE 3U
//#define USBD_CUSTOMHID_REPORT_DESC_SIZE_ENABLED

// UVC options
#define USBD_UVC_FORMAT_UNCOMPRESSED
#define UVC_IN_EP 0x81
//#define UVC_UNCOMPRESSED_GUID 0x32315752 // RW12
#define UVC_UNCOMPRESSED_GUID UVC_GUID_YUY2
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
