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

struct {
    bool expect_buf;
    uint8_t set_cur_buf[4];
    size_t  set_cur_buf_len;
    uint8_t set_cur_entity;
    uint8_t set_cur_selector;
} vc_state;

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

static void VC_Req_GET_DEF(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t entity = HIBYTE(req->wIndex);
    uint8_t cs = HIBYTE(req->wValue);
    uint8_t buf[4];
    size_t len = 0;
    switch (entity) {
    case 0x01U: // Input terminal
        break;
    case 0x02U: // Processing terminal
        {
            switch (cs) {
            case 0x04:  // Gain
                buf[0] = 0x00;
                buf[1] = 0x00;
                len = 2;
                break;
            }
        }
        break;
    case 0x03U: // Output terminal
        break;
    }
    if (len > 0) {
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_VC_INTERFACE_ID;
        USBD_CtlSendData(pdev, buf, len);
    } else {
        USBD_CtlError(pdev, req);
    }
}

static void VC_Req_GET_MIN(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t entity = HIBYTE(req->wIndex);
    uint8_t cs = HIBYTE(req->wValue);
    uint8_t buf[4];
    size_t len = 0;
    switch (entity) {
    case 0x01U: // Input terminal
        break;
    case 0x02U: // Processing terminal
        {
            switch (cs) {
            case 0x04:  // Gain
                buf[0] = 0x00;
                buf[1] = 0x00;
                len = 2;
                break;
            }
        }
        break;
    case 0x03U: // Output terminal
        break;
    }
    if (len > 0) {
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_VC_INTERFACE_ID;
        USBD_CtlSendData(pdev, buf, len);
    } else {
        USBD_CtlError(pdev, req);
    }
}

static void VC_Req_GET_MAX(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t entity = HIBYTE(req->wIndex);
    uint8_t cs = HIBYTE(req->wValue);
    uint8_t buf[4];
    size_t len = 0;
    switch (entity) {
    case 0x01U: // Input terminal
        break;
    case 0x02U: // Processing terminal
        {
            switch (cs) {
            case 0x04:  // Gain
                buf[0] = 0xFF;
                buf[1] = 0x00;
                len = 2;
                break;
            }
        }
        break;
    case 0x03U: // Output terminal
        break;
    }
    if (len > 0) {
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_VC_INTERFACE_ID;
        USBD_CtlSendData(pdev, buf, len);
    } else {
        USBD_CtlError(pdev, req);
    }
}

static void VC_Req_GET_RES(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t entity = HIBYTE(req->wIndex);
    uint8_t cs = HIBYTE(req->wValue);
    uint8_t buf[4];
    size_t len = 0;
    switch (entity) {
    case 0x01U: // Input terminal
        break;
    case 0x02U: // Processing terminal
        {
            switch (cs) {
            case 0x04:  // Gain
                buf[0] = 0x01;
                buf[1] = 0x00;
                len = 2;
                break;
            }
        }
        break;
    case 0x03U: // Output terminal
        break;
    }
    if (len > 0) {
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_VC_INTERFACE_ID;
        USBD_CtlSendData(pdev, buf, len);
    } else {
        USBD_CtlError(pdev, req);
    }
}

static void VC_Req_GET_LEN(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t entity = HIBYTE(req->wIndex);
    uint8_t cs = HIBYTE(req->wValue);
    uint8_t buf[4];
    size_t len = 0;
    switch (entity) {
    case 0x01U: // Input terminal
        break;
    case 0x02U: // Processing terminal
        {
            switch (cs) {
            case 0x04:  // Gain
                buf[0] = 0x02;
                buf[1] = 0x00;
                len = 2;
                break;
            }
        }
        break;
    case 0x03U: // Output terminal
        break;
    }
    if (len > 0) {
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_VC_INTERFACE_ID;
        USBD_CtlSendData(pdev, buf, len);
    } else {
        USBD_CtlError(pdev, req);
    }
}


static void VC_Req_GET_INFO(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t caps = 0x00U;
    uint8_t entity = HIBYTE(req->wIndex);
    uint8_t cs = HIBYTE(req->wValue);
    switch (entity) {
    case 0x01U: // Input terminal
        break;
    case 0x02U: // Processing terminal
        {
            switch (cs) {
            case 0x04:  // Gain
                caps = 0x03U;
                break;
            }
        }
        break;
    case 0x03U: // Output terminal
        break;
    }
    USBD_CAMERA_handle.ep0tx_iface = CAMERA_VC_INTERFACE_ID;
    USBD_CtlSendData(pdev, &caps, 1);
}

static void VC_Req_GET_CUR(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    struct USBD_CAMERA_callbacks_t *cbs = (struct USBD_CAMERA_callbacks_t *)(pdev->pUserData[USBD_CAMERA_handle.classId]);

    uint8_t entity = HIBYTE(req->wIndex);
    uint8_t cs = HIBYTE(req->wValue);
    uint8_t buf[4];
    size_t len = 0;
    switch (entity) {
    case 0x01U: // Input terminal
        break;
    case 0x02U: // Processing terminal
        {
            switch (cs) {
            case 0x04:  // Gain
                if (cbs != NULL && cbs->VC_GetGain != NULL) {
                    unsigned gain;
                    cbs->VC_GetGain(&gain);
                    buf[0] = MIN(gain, 0xFFU) & 0xFFU;
                    buf[1] = 0x00U;
                } else {
                    buf[0] = 0x00U;
                    buf[1] = 0x00U;
                }
                len = 2;
                break;
            }
        }
        break;
    case 0x03U: // Output terminal
        break;
    }
    if (len > 0) {
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_VC_INTERFACE_ID;
        USBD_CtlSendData(pdev, buf, len);
    } else {
        USBD_CtlError(pdev, req);
    }
}

static void VC_Req_SET_CUR(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t cs = HIBYTE(req->wValue);
    uint8_t entity = HIBYTE(req->wIndex);
    switch (entity) {
    case 0x01U: // Input terminal
        break;
    case 0x02U: // Processing terminal
        {
            switch (cs) {
            case 0x04:  // Gain
                vc_state.expect_buf = true;
                vc_state.set_cur_buf_len = 2;
                vc_state.set_cur_entity = 0x02U;
                vc_state.set_cur_selector = 0x04U;
                USBD_CAMERA_ExpectRx(CAMERA_VC_INTERFACE_ID);
                USBD_CtlPrepareRx(pdev, vc_state.set_cur_buf, 2);
                break;
            default:
                USBD_LL_StallEP(pdev, 0x80U);
                break;
            }
        }
        break;
    case 0x03U: // Output terminal
        break;
    }
}

uint8_t VC_EP0_RxReady(struct _USBD_HandleTypeDef *pdev)
{
    if (!vc_state.expect_buf)
        return USBD_FAIL;
    
    struct USBD_CAMERA_callbacks_t *cbs = (struct USBD_CAMERA_callbacks_t *)(pdev->pUserData[USBD_CAMERA_handle.classId]);
    switch (vc_state.set_cur_entity)
    {
        case 0x01U: // Camera terminal
            break;
        case 0x02U: // Processing terminal
            switch (vc_state.set_cur_selector)
            {
                case 0x04U:  // Gain
                    {
                        unsigned gain = vc_state.set_cur_buf[0];
                        if (cbs != NULL && cbs->VC_SetGain != NULL) {
                            cbs->VC_SetGain(gain);
                        }
                    }
                    break;
            }
            break;
        case 0x03U: // Output terminal
            break;
    }
    vc_state.expect_buf = false;
    return USBD_OK;
}

static void VC_SetupClass(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    switch (req->bRequest)
    {
    case UVC_GET_DEF:
        VC_Req_GET_DEF(pdev, req);
        break;
    case UVC_GET_CUR:
        VC_Req_GET_CUR(pdev, req);
        break;
    case UVC_GET_MIN:
        VC_Req_GET_MIN(pdev, req);
        break;
    case UVC_GET_MAX:
        VC_Req_GET_MAX(pdev, req);
        break;
    case UVC_GET_RES:
        VC_Req_GET_RES(pdev, req);
        break;
    case UVC_GET_LEN:
        VC_Req_GET_LEN(pdev, req);
        break;
    case UVC_GET_INFO:
        VC_Req_GET_INFO(pdev, req);
        break;
    case UVC_SET_CUR:
        VC_Req_SET_CUR(pdev, req);
        break;

    default:
        break;
    }
}

void VC_Setup(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t requestType = req->bmRequest & USB_REQ_TYPE_MASK;
    switch (requestType)
    {
    case USB_REQ_TYPE_CLASS:
        VC_SetupClass(pdev, req);
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
