#include "usbd_core.h"
#include "usbd_def.h"

#include "usbd_conf.h"
#include <camera_descriptor.h>
#include "camera.h"
#include "camera_internal.h"

#define HID_REPORT 0x22U
#define UVC_CS_DEVICE 0x21U

static void Get_HID_Descriptor(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t descType = HIBYTE(req->wValue);
    switch (descType)
    {
    case HID_REPORT:
    {
        size_t len = MIN(USBD_CAMERA_HID_Report_len, req->wLength);
        const uint8_t *pbuf = USBD_CAMERA_HID_Report;
        USBD_CtlSendData(pdev, pbuf, len);
        break;
    }
    default:
        break;;
    }
}

static void Get_VIDEO_Descriptor(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t descType = HIBYTE(req->wValue);
    switch (descType)
    {
    case UVC_CS_DEVICE:
    {
        size_t len = 0;
        const void *pbuf = camera_get_video_descriptor(&len);
        len = MIN(len, req->wLength);
        USBD_CtlSendData(pdev, pbuf, len);
        break;
    }
    default:
        break;
    }
}

void GetDescriptor(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    switch (req->wIndex)
    {
    case CAMERA_HID_INTERFACE_ID:
        Get_HID_Descriptor(pdev, req);
        break;
    case CAMERA_VC_INTERFACE_ID:
        Get_VIDEO_Descriptor(pdev, req);
        break;
    case CAMERA_VS_INTERFACE_ID:
        break;
    default:
    {
        // USBD_CtlSendData(pdev, NULL, 0);
        break;
    }
    }
}

void GetInterface(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    switch (req->wIndex)
    {
    case CAMERA_VS_INTERFACE_ID:
    {
        size_t len = MIN(1U, req->wLength);
        if (pdev->dev_state == USBD_STATE_CONFIGURED)
            USBD_CtlSendData(pdev, &USBD_CAMERA_handle.VS_alt, len);
    }
    break;
    case CAMERA_VC_INTERFACE_ID:
    case CAMERA_HID_INTERFACE_ID:
    {
        uint8_t zero[1] = {0};
        size_t len = MIN(1U, req->wLength);
        USBD_CtlSendData(pdev, zero, len);
    }
    break;
    }
}

void SetInterface(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    switch (req->wIndex)
    {
    case CAMERA_VS_INTERFACE_ID:
        if (pdev->dev_state == USBD_STATE_CONFIGURED)
        {
            USBD_CAMERA_handle.VS_alt = LOBYTE(req->wValue);
        }
        break;
    case CAMERA_VC_INTERFACE_ID:
        break;
    case CAMERA_HID_INTERFACE_ID:
        break;
    }
}

void GetStatus(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    if (pdev->dev_state == USBD_STATE_CONFIGURED)
    {
        uint8_t status_info[2] = {0, 0};
        size_t len = MIN(2U, req->wLength);
        USBD_CtlSendData(pdev, status_info, len);
    }
}
