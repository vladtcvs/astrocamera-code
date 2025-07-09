#pragma once

#include "camera.h"

#include <stdbool.h>
#include <stdint.h>

struct USBD_CAMERA_handle_t
{
    uint8_t VS_alt;
    bool hidBusy;
    int ep0rx_iface;
};

#define UVC_CS_DEVICE                                  0x21U

#define UVC_INTERVAL(n)                               (10000000U/(n))

#define UVC_MIN_BIT_RATE(w,h,n)                           (w * h * 16U * (n)) /* 16 bit */
#define UVC_MAX_BIT_RATE(w,h,n)                           (w * h * 16U * (n)) /* 16 bit */

#define WBVAL(x) ((x) & 0xFFU),(((x) >> 8) & 0xFFU)
#define DBVAL(x) ((x) & 0xFFU),(((x) >> 8) & 0xFFU), (((x) >> 16) & 0xFFU), (((x) >> 24) & 0xFFU)


void VC_Setup(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
void VS_Setup(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
void HID_Setup(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

uint8_t HID_Init(struct _USBD_HandleTypeDef *pdev, uint8_t cfgidx);
void HID_DeInit(struct _USBD_HandleTypeDef *pdev, uint8_t cfgidx);

uint8_t HID_DataOut(struct _USBD_HandleTypeDef *pdev, uint8_t epnum);
uint8_t HID_DataIn(struct _USBD_HandleTypeDef *pdev, uint8_t epnum);

uint8_t HID_EP0_RxReady(USBD_HandleTypeDef *pdev);
void USBD_CAMERA_ExpectRx(uint8_t interface);

extern struct USBD_CAMERA_handle_t USBD_CAMERA_handle;
extern size_t USBD_CAMERA_CfgDesc_len;
extern size_t USBD_CAMERA_HID_Report_len;

extern uint8_t USBD_CAMERA_HID_Report[512];
extern uint8_t USBD_CAMERA_CfgDesc[1024];
extern uint8_t video_Probe_Control[48];
extern uint8_t video_Commit_Control[48];
