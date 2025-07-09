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

static void VS_Req_GET_CUR(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    switch (req->wValue)
    {
    case VS_PROBE_CONTROL:
        USBD_CtlSendData(pdev, video_Probe_Control, sizeof(video_Probe_Control));
        break;
    case VS_COMMIT_CONTROL:
        USBD_CtlSendData(pdev, video_Probe_Control, sizeof(video_Probe_Control));
        break;
    default:
        break;
    }
}

static void VS_Req_SET_CUR(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    switch (req->wValue) {
    case VS_PROBE_CONTROL:
        USBD_CtlPrepareRx(pdev, video_Probe_Control, MIN(req->wLength, sizeof(video_Probe_Control)));
        break;
    case VS_COMMIT_CONTROL:
        USBD_CtlPrepareRx(pdev, video_Probe_Control, MIN(req->wLength, sizeof(video_Probe_Control)));
        break;
    default:
        USBD_LL_StallEP(pdev, 0x80U);
        break;
    }
}

void VS_SetupClass(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    switch (req->bRequest)
    {
    case UVC_GET_DEF:
    case UVC_GET_CUR:
    case UVC_GET_MIN:
    case UVC_GET_MAX:
        VS_Req_GET_CUR(pdev, req);
        break;

    case UVC_GET_RES:
    case UVC_GET_LEN:
    case UVC_GET_INFO:
        break;

    case UVC_SET_CUR:
        VS_Req_SET_CUR(pdev, req);
        break;

    default:
        break;
    }
}

static void VS_GetDescriptor(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    // do nothing
}

void VS_GetInterface(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    size_t len = MIN(1U, req->wLength);
    if (pdev->dev_state == USBD_STATE_CONFIGURED)
        USBD_CtlSendData(pdev, &USBD_CAMERA_handle.VS_alt, len);
}

static void open_bulk_ep(struct _USBD_HandleTypeDef *pdev)
{
    if (pdev->dev_speed == USBD_SPEED_HIGH)
    {
        USBD_LL_OpenEP(pdev, CAMERA_UVC_IN_EP, USBD_EP_TYPE_ISOC, CAMERA_UVC_ISO_HS_MPS);
        pdev->ep_in[CAMERA_UVC_IN_EP & 0x0FU].maxpacket = CAMERA_UVC_ISO_HS_MPS;
    }
    else
    {
        USBD_LL_OpenEP(pdev, CAMERA_UVC_IN_EP, USBD_EP_TYPE_ISOC, CAMERA_UVC_ISO_FS_MPS);
        pdev->ep_in[CAMERA_UVC_IN_EP & 0x0FU].maxpacket = CAMERA_UVC_ISO_FS_MPS;
    }

    pdev->ep_in[CAMERA_UVC_IN_EP & 0x0FU].is_used = 1U;
    pdev->ep_in[CAMERA_UVC_IN_EP & 0x0FU].maxpacket = CAMERA_UVC_ISO_HS_MPS;
}

static void close_bulk_ep(struct _USBD_HandleTypeDef *pdev)
{
    USBD_LL_CloseEP(pdev, CAMERA_UVC_IN_EP);
    pdev->ep_in[CAMERA_UVC_IN_EP & 0x0FU].is_used = 0U;
}

static uint8_t VS_SetInterface(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    if (pdev->dev_state != USBD_STATE_CONFIGURED)
        return USBD_OK;

    uint8_t old_alt = USBD_CAMERA_handle.VS_alt;
    int newAlt = LOBYTE(req->wValue);
    if (newAlt != 0 && newAlt != 1)
        return USBD_FAIL;
    USBD_CAMERA_handle.VS_alt = newAlt;

    if (old_alt != USBD_CAMERA_handle.VS_alt)
    {
        struct USBD_CAMERA_callbacks_t *cbs = (struct USBD_CAMERA_callbacks_t *)(pdev->pUserData[USBD_CAMERA_handle.classId]);
        if (USBD_CAMERA_handle.VS_alt == 1)
        {
            open_bulk_ep(pdev);
            if (cbs->VS_StartStream != NULL)
                return cbs->VS_StartStream();
        }
        else
        {
            close_bulk_ep(pdev);
            if (cbs->VS_StopStream != NULL)
                return cbs->VS_StopStream();
        }
    }

    return USBD_OK;
}

static void VS_GetStatus(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    if (pdev->dev_state == USBD_STATE_CONFIGURED)
    {
        uint8_t status_info[2] = {0, 0};
        size_t len = MIN(2U, req->wLength);
        USBD_CtlSendData(pdev, status_info, len);
    }
}

void VS_Setup(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t requestType = req->bmRequest & USB_REQ_TYPE_MASK;
    switch (requestType)
    {
    case USB_REQ_TYPE_CLASS:
        VS_SetupClass(pdev, req);
        break;
    case USB_REQ_TYPE_STANDARD:
        switch (req->bRequest)
        {
        case USB_REQ_GET_STATUS:
            VS_GetStatus(pdev, req);
            break;
        case USB_REQ_GET_DESCRIPTOR:
            VS_GetDescriptor(pdev, req);
            break;
        case USB_REQ_GET_INTERFACE:
            VS_GetInterface(pdev, req);
            break;
        case USB_REQ_SET_INTERFACE:
            VS_SetInterface(pdev, req);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}
