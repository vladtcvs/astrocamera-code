#include "camera_internal.h"
#include "usbd_core.h"
#include "usbd_def.h"

#include "usbd_conf.h"
#include <camera_descriptor.h>

#define UVC_SET_CUR 0x01U
#define UVC_GET_CUR 0x81U
#define UVC_GET_MIN 0x82U
#define UVC_GET_MAX 0x83U
#define UVC_GET_RES 0x84U
#define UVC_GET_LEN 0x85U
#define UVC_GET_INFO 0x86U
#define UVC_GET_DEF 0x87U

#define VS_PROBE_CONTROL 0x100U
#define VS_COMMIT_CONTROL 0x200U


static void VC_GetDescriptor(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t descType = HIBYTE(req->wValue);
    switch (descType)
    {
    case UVC_CS_DEVICE:
    {
        size_t len = 0;
        const void *pbuf = camera_get_video_descriptor(&len);
        len = MIN(len, req->wLength);
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_VC_INTERFACE_ID;
        USBD_CtlSendData(pdev, pbuf, len);
        break;
    }
    default:
        break;
    }
}

static void VC_GetInterface(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t zero[1] = {0};
    size_t len = MIN(1U, req->wLength);
    USBD_CAMERA_handle.ep0tx_iface = CAMERA_VC_INTERFACE_ID;
    USBD_CtlSendData(pdev, zero, len);
}


static void VC_SetInterface(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    // do nothing
}

static void VC_GetStatus(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    if (pdev->dev_state == USBD_STATE_CONFIGURED)
    {
        uint8_t status_info[2] = {0, 0};
        size_t len = MIN(2U, req->wLength);
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_VC_INTERFACE_ID;
        USBD_CtlSendData(pdev, status_info, len);
    }
}

void VC_Setup(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t requestType = req->bmRequest & USB_REQ_TYPE_MASK;
    switch (requestType)
    {
    case USB_REQ_TYPE_CLASS:
        break;
    case USB_REQ_TYPE_STANDARD:
        switch (req->bRequest)
        {
        case USB_REQ_GET_STATUS:
            VC_GetStatus(pdev, req);
            break;
        case USB_REQ_GET_DESCRIPTOR:
            VC_GetDescriptor(pdev, req);
            break;
        case USB_REQ_GET_INTERFACE:
            VC_GetInterface(pdev, req);
            break;
        case USB_REQ_SET_INTERFACE:
            VC_SetInterface(pdev, req);
            break;
        default:
            break;
        }

        break;
    default:
        break;
    }
}
