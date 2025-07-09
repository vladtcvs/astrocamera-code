#include "usbd_core.h"
#include "usbd_def.h"

#include "usbd_conf.h"
#include <camera_descriptor.h>
#include "camera.h"
#include "camera_internal.h"

#define HID_REPORT 0x22U
#define UVC_CS_DEVICE 0x21U

static void HID_GetDescriptor(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
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

static void HID_GetInterface(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t zero[1] = {0};
    size_t len = MIN(1U, req->wLength);
    USBD_CtlSendData(pdev, zero, len);
}

static void HID_SetInterface(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    // do nothing
}

static void HID_GetStatus(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    if (pdev->dev_state == USBD_STATE_CONFIGURED)
    {
        uint8_t status_info[2] = {0, 0};
        size_t len = MIN(2U, req->wLength);
        USBD_CtlSendData(pdev, status_info, len);
    }
}

void HID_Setup(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t requestType = req->bmRequest & USB_REQ_TYPE_MASK;
    switch (requestType)
    {
    case USB_REQ_TYPE_CLASS:
        HID_SetupClass(pdev, req);
        break;
    case USB_REQ_TYPE_STANDARD:
        switch (req->bRequest)
        {
        case USB_REQ_GET_STATUS:
            HID_GetStatus(pdev, req);
            break;
        case USB_REQ_GET_DESCRIPTOR:
            HID_GetDescriptor(pdev, req);
            break;
        case USB_REQ_GET_INTERFACE:
            HID_GetInterface(pdev, req);
            break;
        case USB_REQ_SET_INTERFACE:
            HID_SetInterface(pdev, req);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}
