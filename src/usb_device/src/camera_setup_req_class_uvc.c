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
