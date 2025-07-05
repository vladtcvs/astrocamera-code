/**
  ******************************************************************************
  * @file    usbd_video.c
  * @author  MCD Application Team
  * @brief   This file provides the Video core functions.
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  * @verbatim
  *
  *          ===================================================================
  *                                VIDEO Class  Description
  *          ===================================================================
  *           This driver manages the Video Class 1.1 following the "USB Device Class Definition for
  *           Video Devices V1.0 Mar 18, 98".
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Interface Association Descriptor
  *             -Standard VC Interface Descriptor  = interface 0
  *             -Standard Vs Interface Descriptor  = interface 1
  *             - 1 Video Streaming Interface
  *             - 1 Video Streaming Endpoint
  *             - 1 Video Terminal Input (camera)
  *             - Video Class-Specific AC Interfaces
  *             - Video Class-Specific AS Interfaces
  *             - VideoControl Requests
  *             - Video Synchronization type: Asynchronous
  *          The current  Video class version supports the following Video features:
  *             - image JPEG format
  *             - Asynchronous Endpoints
  *
  * @note     In HS mode and when the USB DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *
  *
  *  @endverbatim
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbd_video.h"
#include "usbd_ctlreq.h"
#include "usbd_core.h"

/* VIDEO Device library callbacks */
static uint8_t USBD_VIDEO_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_VIDEO_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_VIDEO_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

static uint8_t USBD_VIDEO_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_VIDEO_SOF(USBD_HandleTypeDef *pdev);
static uint8_t USBD_VIDEO_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum);

/* VIDEO Requests management functions */
static void VIDEO_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void VIDEO_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

static void *USBD_VIDEO_GetVideoHeaderDesc(uint8_t *pConfDesc);

USBD_ClassTypeDef  USBD_VIDEO =
{
  USBD_VIDEO_Init,
  USBD_VIDEO_DeInit,
  USBD_VIDEO_Setup,
  NULL,
  NULL,
  USBD_VIDEO_DataIn,
  NULL,
  USBD_VIDEO_SOF,
  USBD_VIDEO_IsoINIncomplete,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
};

static uint8_t VIDEOinEpAdd = UVC_IN_EP;

/* Video Commit data structure */
static USBD_VideoControlTypeDef video_Commit_Control =
{
  .bmHint = 0x0000U,
  .bFormatIndex = 0x01U,
  .bFrameIndex = 0x01U,
  .dwFrameInterval = UVC_INTERVAL(UVC_CAM_FPS_FS),
  .wKeyFrameRate = 0x0000U,
  .wPFrameRate = 0x0000U,
  .wCompQuality = 0x0000U,
  .wCompWindowSize = 0x0000U,
  .wDelay = 0x0000U,
  .dwMaxVideoFrameSize = 0x0000U,
  .dwMaxPayloadTransferSize = 0x00000000U,
  .dwClockFrequency = 0x00000000U,
  .bmFramingInfo = 0x00U,
  .bPreferedVersion = 0x00U,
  .bMinVersion = 0x00U,
  .bMaxVersion = 0x00U,
};

/* Video Probe data structure */
static USBD_VideoControlTypeDef video_Probe_Control =
{
  .bmHint = 0x0000U,
  .bFormatIndex = 0x01U,
  .bFrameIndex = 0x01U,
  .dwFrameInterval = UVC_INTERVAL(UVC_CAM_FPS_FS),
  .wKeyFrameRate = 0x0000U,
  .wPFrameRate = 0x0000U,
  .wCompQuality = 0x0000U,
  .wCompWindowSize = 0x0000U,
  .wDelay = 0x0000U,
  .dwMaxVideoFrameSize = 0x0000U,
  .dwMaxPayloadTransferSize = 0x00000000U,
  .dwClockFrequency = 0x00000000U,
  .bmFramingInfo = 0x00U,
  .bPreferedVersion = 0x00U,
  .bMinVersion = 0x00U,
  .bMaxVersion = 0x00U,
};

/**
  * @brief  USBD_VIDEO_Init
  *         Initialize the VIDEO interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_VIDEO_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  USBD_VIDEO_HandleTypeDef *hVIDEO;

  /* Allocate memory for the video control structure */
  hVIDEO = (USBD_VIDEO_HandleTypeDef *)USBD_malloc(sizeof(USBD_VIDEO_HandleTypeDef));

  /* Check if allocated point is NULL, then exit with error code */
  if (hVIDEO == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  /* Assign the pClassData pointer to the allocated structure */
  pdev->pClassDataCmsit[pdev->classId] = (void *)hVIDEO;
  pdev->pClassData = pdev->pClassDataCmsit[pdev->classId];

  /* Get the Endpoints addresses allocated for this class instance */
  VIDEOinEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_ISOC, (uint8_t)pdev->classId);

  /* Open EP IN */
  if (pdev->dev_speed == USBD_SPEED_HIGH)
  {
    (void)USBD_LL_OpenEP(pdev, VIDEOinEpAdd, USBD_EP_TYPE_ISOC, UVC_ISO_HS_MPS);

    pdev->ep_in[VIDEOinEpAdd & 0xFU].is_used = 1U;
    pdev->ep_in[VIDEOinEpAdd & 0xFU].maxpacket = UVC_ISO_HS_MPS;
  }
  else
  {
    (void)USBD_LL_OpenEP(pdev, VIDEOinEpAdd, USBD_EP_TYPE_ISOC, UVC_ISO_FS_MPS);

    pdev->ep_in[VIDEOinEpAdd & 0xFU].is_used = 1U;
    pdev->ep_in[VIDEOinEpAdd & 0xFU].maxpacket = UVC_ISO_FS_MPS;
  }

  /* Init  physical Interface components */
  ((USBD_VIDEO_ItfTypeDef *)pdev->pUserData[pdev->classId])->Init();

  /* Init Xfer states */
  hVIDEO->interface = 0U;

  /* Some calls to unused variables, to comply with MISRA-C 2012 rules */
  UNUSED(cfgidx);

  /* Exit with no error code */
  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_VIDEO_DeInit
  *         DeInitialize the VIDEO layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_VIDEO_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);

  /* Check if the video structure pointer is valid */
  if (pdev->pClassDataCmsit[pdev->classId] == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  /* Get the Endpoints addresses allocated for this class instance */
  VIDEOinEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_ISOC, (uint8_t)pdev->classId);

  /* Close EP IN */
  (void)USBD_LL_CloseEP(pdev, VIDEOinEpAdd);
  pdev->ep_in[VIDEOinEpAdd & 0xFU].is_used = 0U;

  /* DeInit  physical Interface components */
  ((USBD_VIDEO_ItfTypeDef *)pdev->pUserData[pdev->classId])->DeInit();
  USBD_free(pdev->pClassDataCmsit[pdev->classId]);
  pdev->pClassDataCmsit[pdev->classId] = NULL;
  pdev->pClassData = NULL;

  /* Exit with no error code */
  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_VIDEO_Setup
  *         Handle the VIDEO specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t USBD_VIDEO_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_VIDEO_HandleTypeDef *hVIDEO = (USBD_VIDEO_HandleTypeDef *) pdev->pClassDataCmsit[pdev->classId];
  uint8_t ret = (uint8_t)USBD_OK;
  uint16_t status_info = 0U;
  uint16_t len;
  uint8_t *pbuf;

  /* Get the Endpoints addresses allocated for this class instance */
  VIDEOinEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_ISOC, (uint8_t)pdev->classId);

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    /* Class Requests -------------------------------*/
    case USB_REQ_TYPE_CLASS:
      switch (req->bRequest)
      {
        case UVC_GET_CUR:
        case UVC_GET_DEF:
        case UVC_GET_MIN:
        case UVC_GET_MAX:
          VIDEO_REQ_GetCurrent(pdev, req);
          break;

        case UVC_GET_RES:
        case UVC_GET_LEN:
        case UVC_GET_INFO:
          break;

        case UVC_SET_CUR:
          VIDEO_REQ_SetCurrent(pdev, req);
          break;

        default:
          (void) USBD_CtlError(pdev, req);
          ret = (uint8_t)USBD_FAIL;
          break;
      }
      break;

    /* Standard Requests -------------------------------*/
    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        case USB_REQ_GET_STATUS:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            (void) USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = (uint8_t)USBD_FAIL;
          }
          break;

        case USB_REQ_GET_DESCRIPTOR:
          if ((req->wValue >> 8) == CS_DEVICE)
          {
            pbuf = (uint8_t *)USBD_VIDEO_GetVideoHeaderDesc(pdev->pConfDesc);
            if (pbuf != NULL)
            {
              len = MIN((uint16_t)USB_CONF_DESC_SIZE, (uint16_t)req->wLength);
              (void)USBD_CtlSendData(pdev, pbuf, len);
            }
            else
            {
              USBD_CtlError(pdev, req);
              ret = (uint8_t)USBD_FAIL;
            }
          }
          break;

        case USB_REQ_GET_INTERFACE :
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            (void)USBD_CtlSendData(pdev, (uint8_t *)&hVIDEO->interface, 1U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = (uint8_t)USBD_FAIL;
          }
          break;

        case USB_REQ_SET_INTERFACE :
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            if (req->wValue <= USBD_MAX_NUM_INTERFACES)
            {
              hVIDEO->interface = LOBYTE(req->wValue);
              if (hVIDEO->interface == 1U)
              {
                /* Start Streaming (First endpoint writing will be done on next SOF) */
                (void)USBD_LL_FlushEP(pdev, VIDEOinEpAdd);
                hVIDEO->uvc_state = UVC_PLAY_STATUS_READY;
              }
              else
              {
                /* Stop Streaming */
                hVIDEO->uvc_state = UVC_PLAY_STATUS_STOP;
                (void)USBD_LL_FlushEP(pdev, VIDEOinEpAdd);
              }
            }
            else
            {
              /* Call the error management function (command will be NAKed) */
              USBD_CtlError(pdev, req);
              ret = (uint8_t)USBD_FAIL;
            }
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = (uint8_t)USBD_FAIL;
          }
          break;

        case USB_REQ_CLEAR_FEATURE:
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = (uint8_t)USBD_FAIL;
          break;
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      ret = (uint8_t)USBD_FAIL;
      break;
  }

  return ret;
}

/**
  * @brief  USBD_VIDEO_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_VIDEO_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_VIDEO_HandleTypeDef *hVIDEO = (USBD_VIDEO_HandleTypeDef *) pdev->pClassDataCmsit[pdev->classId];
  static uint8_t  packet[UVC_PACKET_SIZE + (UVC_HEADER_PACKET_CNT * 2U)] = {0x00U};
  static uint8_t *Pcktdata = packet;
  static uint16_t PcktIdx = 0U;
  static uint16_t PcktSze = UVC_PACKET_SIZE;
  static uint8_t  payload_header[2] = {0x02U, 0x00U};
  uint8_t i = 0U;
  uint32_t RemainData = 0U;
  uint32_t DataOffset = 0U;

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  VIDEOinEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_ISOC, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  /* Check if the Streaming has already been started */
  if (hVIDEO->uvc_state == UVC_PLAY_STATUS_STREAMING)
  {
    /* Get the current packet buffer, index and size from the application layer */
    ((USBD_VIDEO_ItfTypeDef *)pdev->pUserData[pdev->classId])->Data(&Pcktdata, &PcktSze, &PcktIdx);

    /* Check if end of current image has been reached */
    if (PcktSze > 2U)
    {
      /* Check if this is the first packet in current image */
      if (PcktIdx == 0U)
      {
        /* Set the packet start index */
        payload_header[1] ^= 0x01U;
      }

      RemainData = PcktSze;

      /* fill the Transmit buffer */
      while (RemainData > 0U)
      {
        packet[((DataOffset + 0U) * i)] = payload_header[0];
        packet[((DataOffset + 0U) * i) + 1U] = payload_header[1];

        if (RemainData > pdev->ep_in[VIDEOinEpAdd & 0xFU].maxpacket)
        {
          DataOffset = pdev->ep_in[VIDEOinEpAdd & 0xFU].maxpacket;
          (void)USBD_memcpy((packet + ((DataOffset + 0U) * i) + 2U),
                            Pcktdata + ((DataOffset - 2U) * i), (DataOffset - 2U));

          RemainData -= DataOffset;
          i++;
        }
        else
        {
          (void)USBD_memcpy((packet + ((DataOffset + 0U) * i) + 2U),
                            Pcktdata + ((DataOffset - 2U) * i), (RemainData - 2U));

          RemainData = 0U;
        }
      }
    }
    else
    {
      /* Add the packet header */
      packet[0] = payload_header[0];
      packet[1] = payload_header[1];
    }

    hVIDEO->uvc_buffer = (uint8_t *)&packet;
    hVIDEO->uvc_size = (uint32_t)PcktSze;

    /* Transmit the packet on Endpoint */
    (void)USBD_LL_Transmit(pdev, (uint8_t)(epnum | 0x80U),
                           hVIDEO->uvc_buffer, hVIDEO->uvc_size);
  }

  /* Exit with no error code */
  return (uint8_t) USBD_OK;
}

/**
  * @brief  USBD_VIDEO_SOF
  *         handle SOF event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_VIDEO_SOF(USBD_HandleTypeDef *pdev)
{
  USBD_VIDEO_HandleTypeDef *hVIDEO = (USBD_VIDEO_HandleTypeDef *) pdev->pClassDataCmsit[pdev->classId];
  uint8_t payload[2] = {0x02U, 0x00U};

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  VIDEOinEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_ISOC, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  /* Check if the Streaming has already been started by SetInterface AltSetting 1 */
  if (hVIDEO->uvc_state == UVC_PLAY_STATUS_READY)
  {
    hVIDEO->uvc_buffer = (uint8_t *)&payload;
    hVIDEO->uvc_size = 2U;

    /* Transmit the first packet indicating that Streaming is starting */
    (void)USBD_LL_Transmit(pdev, VIDEOinEpAdd, hVIDEO->uvc_buffer, hVIDEO->uvc_size);

    /* Enable Streaming state */
    hVIDEO->uvc_state = UVC_PLAY_STATUS_STREAMING;
  }

  /* Exit with no error code */
  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_VIDEO_IsoINIncomplete
  *         handle data ISO IN Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_VIDEO_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_VIDEO_HandleTypeDef *hVIDEO = (USBD_VIDEO_HandleTypeDef *) pdev->pClassDataCmsit[pdev->classId];

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  VIDEOinEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_ISOC, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  if (epnum == (VIDEOinEpAdd & 0xFU))
  {
    (void)USBD_LL_Transmit(pdev, VIDEOinEpAdd, hVIDEO->uvc_buffer, hVIDEO->uvc_size);
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  VIDEO_Req_GetCurrent
  *         Handles the GET_CUR VIDEO control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void VIDEO_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_VIDEO_HandleTypeDef *hVIDEO;
  hVIDEO = (USBD_VIDEO_HandleTypeDef *)(pdev->pClassDataCmsit[pdev->classId]);
  static __IO uint8_t EntityStatus[8] = {0};

  /* Reset buffer to zeros */
  (void) USBD_memset(hVIDEO->control.data, 0, USB_MAX_EP0_SIZE);

  /* Manage Video Control interface requests */
  if (LOBYTE(req->wIndex) == 0x00U)
  {
    if (HIBYTE(req->wValue) == 0x02U)
    {
      /* Get the status of the current requested Entity */
      EntityStatus[0] = 0x06U;

      /* Send current status */
      (void) USBD_CtlSendData(pdev, (uint8_t *)&EntityStatus, 1U);
    }
    else
    {
      /* Unknown request */
      USBD_CtlError(pdev, req);
    }
  }
  /* Manage Video Streaming interface requests */
  else
  {
    if (LOBYTE(req->wValue) == (uint8_t)VS_PROBE_CONTROL)
    {
      /* Update bPreferedVersion, bMinVersion and bMaxVersion which must be set only by Device */
      video_Probe_Control.bPreferedVersion = 0x00U;
      video_Probe_Control.bMinVersion = 0x00U;
      video_Probe_Control.bMaxVersion = 0x00U;
      video_Probe_Control.dwMaxVideoFrameSize = UVC_MAX_FRAME_SIZE;

      video_Probe_Control.dwClockFrequency = 0x02DC6C00U;

      if (pdev->dev_speed == USBD_SPEED_HIGH)
      {
        video_Probe_Control.dwFrameInterval = (UVC_INTERVAL(UVC_CAM_FPS_HS));
        video_Probe_Control.dwMaxPayloadTransferSize = UVC_ISO_HS_MPS;
      }
      else
      {
        video_Probe_Control.dwFrameInterval = (UVC_INTERVAL(UVC_CAM_FPS_FS));
        video_Probe_Control.dwMaxPayloadTransferSize = UVC_ISO_FS_MPS;
      }

      /* Probe Request */
      (void)USBD_CtlSendData(pdev, (uint8_t *)&video_Probe_Control,
                             MIN(req->wLength, sizeof(USBD_VideoControlTypeDef)));
    }
    else if (LOBYTE(req->wValue) == (uint8_t)VS_COMMIT_CONTROL)
    {
      if (pdev->dev_speed == USBD_SPEED_HIGH)
      {
        video_Commit_Control.dwFrameInterval = (UVC_INTERVAL(UVC_CAM_FPS_HS));
        video_Commit_Control.dwMaxPayloadTransferSize = UVC_ISO_HS_MPS;
      }
      else
      {
        video_Commit_Control.dwFrameInterval = (UVC_INTERVAL(UVC_CAM_FPS_FS));
        video_Commit_Control.dwMaxPayloadTransferSize = UVC_ISO_FS_MPS;
      }

      /* Commit Request */
      (void)USBD_CtlSendData(pdev, (uint8_t *)&video_Commit_Control,
                             MIN(req->wLength, sizeof(USBD_VideoControlTypeDef)));
    }
    else
    {
      /* Send the current state */
      (void) USBD_CtlSendData(pdev, hVIDEO->control.data,
                              MIN(req->wLength, USB_MAX_EP0_SIZE));
    }
  }
}

/**
  * @brief  VIDEO_Req_SetCurrent
  *         Handles the SET_CUR VIDEO control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void VIDEO_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{

  /* Check that the request has control data */
  if (req->wLength > 0U)
  {
    /* Prepare the reception of the buffer over EP0 */
    if (req->wValue == (uint16_t)VS_PROBE_CONTROL)
    {
      /* Probe Request */
      (void) USBD_CtlPrepareRx(pdev, (uint8_t *)&video_Probe_Control,
                               MIN(req->wLength, sizeof(USBD_VideoControlTypeDef)));
    }
    else if (req->wValue == (uint16_t)VS_COMMIT_CONTROL)
    {
      /* Commit Request */
      (void) USBD_CtlPrepareRx(pdev, (uint8_t *)&video_Commit_Control,
                               MIN(req->wLength, sizeof(USBD_VideoControlTypeDef)));
    }
    else
    {
      (void)USBD_LL_StallEP(pdev, 0x80U);
    }
  }
}

/**
  * @brief  USBD_VIDEO_GetVideoHeaderDesc
  *         This function return the Video Header descriptor
  * @param  pdev: device instance
  * @param  pConfDesc:  pointer to Bos descriptor
  * @retval pointer to the Video Header descriptor
  */
static void *USBD_VIDEO_GetVideoHeaderDesc(uint8_t *pConfDesc)
{
  USBD_ConfigDescTypeDef *desc = (USBD_ConfigDescTypeDef *)(void *)pConfDesc;
  USBD_DescHeaderTypeDef *pdesc = (USBD_DescHeaderTypeDef *)(void *)pConfDesc;
  uint8_t *pVideoDesc = NULL;
  uint16_t ptr;

  if (desc->wTotalLength > desc->bLength)
  {
    ptr = desc->bLength;

    while (ptr < desc->wTotalLength)
    {
      pdesc = USBD_GetNextDesc((uint8_t *)pdesc, &ptr);
      if ((pdesc->bDescriptorType == CS_INTERFACE) &&
          (pdesc->bDescriptorSubType == VC_HEADER))
      {
        pVideoDesc = (uint8_t *)pdesc;
        break;
      }
    }
  }
  return pVideoDesc;
}

/**
  * @brief  USBD_VIDEO_RegisterInterface
  * @param  pdev: instance
  * @param  fops: VIDEO interface callback
  * @retval status
  */
uint8_t USBD_VIDEO_RegisterInterface(USBD_HandleTypeDef   *pdev, USBD_VIDEO_ItfTypeDef *fops)
{
  /* Check if the FOPS pointer is valid */
  if (fops == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  /* Assign the FOPS pointer */
  pdev->pUserData[pdev->classId] = fops;

  /* Exit with no error code */
  return (uint8_t)USBD_OK;
}
