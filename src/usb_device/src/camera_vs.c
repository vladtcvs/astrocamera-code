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

#define VS_PROBE_CONTROL_SELECTOR 0x01U
#define VS_COMMIT_CONTROL_SELECTOR 0x02U

static void VS_Req_GET_CUR(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    switch (HIBYTE(req->wValue))
    {
    case VS_PROBE_CONTROL_SELECTOR:
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_VS_INTERFACE_ID;
        USBD_CtlSendData(pdev, video_Probe_Control, sizeof(video_Probe_Control));
        break;
    case VS_COMMIT_CONTROL_SELECTOR:
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_VS_INTERFACE_ID;
        USBD_CtlSendData(pdev, video_Probe_Control, sizeof(video_Probe_Control));
        break;
    default:
        break;
    }
}

static void VS_Req_GET_INFO(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{

}

static void VS_Req_SET_CUR(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    switch (HIBYTE(req->wValue)) {
    case VS_PROBE_CONTROL_SELECTOR:
        USBD_CtlPrepareRx(pdev, video_Probe_Control, MIN(req->wLength, sizeof(video_Probe_Control)));
        break;
    case VS_COMMIT_CONTROL_SELECTOR:
        USBD_CtlPrepareRx(pdev, video_Probe_Control, MIN(req->wLength, sizeof(video_Probe_Control)));
        break;
    default:
        USBD_LL_StallEP(pdev, 0x80U);
        break;
    }
}

static void VS_SetupClass(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
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
        break;

    case UVC_GET_LEN:
        break;

    case UVC_GET_INFO:
        VS_Req_GET_INFO(pdev, req);
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
    if (pdev->dev_state == USBD_STATE_CONFIGURED) {
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_VS_INTERFACE_ID;
        USBD_CtlSendData(pdev, &USBD_CAMERA_handle.VS_alt, len);
    }
}

static void open_isoc_ep(struct _USBD_HandleTypeDef *pdev)
{
    uint8_t res = USBD_LL_OpenEP(pdev, CAMERA_UVC_EPIN, USBD_EP_TYPE_ISOC, CAMERA_UVC_EPIN_SIZE);
    pdev->ep_in[CAMERA_UVC_EPIN & 0x0FU].maxpacket = CAMERA_UVC_EPIN_SIZE;
    pdev->ep_in[CAMERA_UVC_EPIN & 0x0FU].is_used = 1U;
}

static void close_isoc_ep(struct _USBD_HandleTypeDef *pdev)
{
    USBD_LL_CloseEP(pdev, CAMERA_UVC_EPIN);
    pdev->ep_in[CAMERA_UVC_EPIN & 0x0FU].is_used = 0U;
}

static uint8_t frame[UVC_CHUNK+12U] = {12U, 0x00U};
static size_t offset = 0;
static uint8_t UVC_FID = 0x00U;
static enum {
    UVC_FRAME_IDLE = 0,
    UVC_FRAME_READY,
    UVC_FRAME_RUN,
    UVC_FRAME_NEXT,
} status = UVC_FRAME_IDLE;

static unsigned chunk_id;
static size_t payload_size;
static size_t frame_size = 640*480*2;

static void start_uvc_frame(struct _USBD_HandleTypeDef *pdev)
{
    if (status != UVC_FRAME_READY)
        return;

    offset = 0;
    status = UVC_FRAME_RUN;

    chunk_id = 0;
    UVC_FID = 0x01U;
    frame[1] = UVC_FID;

    frame[12] = 0xFFU;
    frame[13] = 0xFFU;
    frame[14] = 0xFFU;
    frame[15] = 0xFFU;

    uint8_t result = USBD_LL_Transmit(pdev, CAMERA_UVC_EPIN, frame, 12U);
    status = UVC_FRAME_RUN;
    chunk_id += 1;
}

static void continue_uvc_frame(struct _USBD_HandleTypeDef *pdev)
{
    if (status != UVC_FRAME_RUN && status != UVC_FRAME_NEXT)
        return;

    if (status == UVC_FRAME_NEXT) {
        status = UVC_FRAME_RUN;
        UVC_FID ^= 0x01U;
        offset = 0;
    }

    if (frame_size - offset > UVC_CHUNK) {
        frame[1] = UVC_FID;
        payload_size = UVC_CHUNK;
    } else {
        frame[1] = UVC_FID | 0x02;
        payload_size = frame_size - offset;
        status = UVC_FRAME_NEXT;
    }
    uint8_t result = USBD_LL_Transmit(pdev, CAMERA_UVC_EPIN, frame, payload_size + 12U);
    offset += payload_size;
    chunk_id += 1;
}

uint8_t VS_DataIn(struct _USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    if (USBD_CAMERA_handle.VS_alt == 0)
        return USBD_OK;
    continue_uvc_frame(pdev);
    return USBD_OK;
}

uint8_t VS_IsoINIncomplete(struct _USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    if (USBD_CAMERA_handle.VS_alt == 0)
        return USBD_OK;

    uint8_t result = USBD_LL_Transmit(pdev, CAMERA_UVC_EPIN, frame, payload_size + 12U);
    return USBD_OK;
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
            open_isoc_ep(pdev);
            status = UVC_FRAME_READY;
            USBD_LL_FlushEP(pdev, CAMERA_UVC_EPIN);
            if (cbs->VS_StartStream != NULL)
                return cbs->VS_StartStream();
        }
        else
        {
            close_isoc_ep(pdev);
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
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_VS_INTERFACE_ID;
        USBD_CtlSendData(pdev, status_info, len);
    }
}

uint8_t VS_SOF(struct _USBD_HandleTypeDef *pdev)
{
    if (status == UVC_FRAME_READY) {
        start_uvc_frame(pdev);
    }
    return USBD_OK;
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
