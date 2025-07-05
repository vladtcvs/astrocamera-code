/**
  ******************************************************************************
  * @file    usbd_customhid.c
  * @author  MCD Application Team
  * @brief   This file provides the CUSTOM_HID core functions.
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2015 STMicroelectronics.
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
  *                                CUSTOM_HID Class  Description
  *          ===================================================================
  *           This module manages the CUSTOM_HID class V1.11 following the "Device Class Definition
  *           for Human Interface Devices (CUSTOM_HID) Version 1.11 Jun 27, 2001".
  *           This driver implements the following aspects of the specification:
  *             - The Boot Interface Subclass
  *             - Usage Page : Generic Desktop
  *             - Usage : Vendor
  *             - Collection : Application
  *
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *
  *
  *  @endverbatim
  *
  ******************************************************************************
  */

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbd_customhid.h"
#include "usbd_ctlreq.h"


static uint8_t USBD_CUSTOM_HID_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_CUSTOM_HID_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_CUSTOM_HID_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

static uint8_t USBD_CUSTOM_HID_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_CUSTOM_HID_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_CUSTOM_HID_EP0_RxReady(USBD_HandleTypeDef  *pdev);

USBD_ClassTypeDef  USBD_CUSTOM_HID =
{
  USBD_CUSTOM_HID_Init,
  USBD_CUSTOM_HID_DeInit,
  USBD_CUSTOM_HID_Setup,
  NULL, /*EP0_TxSent*/
  USBD_CUSTOM_HID_EP0_RxReady, /*EP0_RxReady*/ /* STATUS STAGE IN */
  USBD_CUSTOM_HID_DataIn, /*DataIn*/
  USBD_CUSTOM_HID_DataOut,
  NULL, /*SOF */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
};

/* USB CUSTOM_HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CUSTOM_HID_Desc[USB_CUSTOM_HID_DESC_SIZ] __ALIGN_END =
{
  /* 18 */
  0x09,                                               /* bLength: CUSTOM_HID Descriptor size */
  CUSTOM_HID_DESCRIPTOR_TYPE,                         /* bDescriptorType: CUSTOM_HID */
  0x11,                                               /* bCUSTOM_HIDUSTOM_HID: CUSTOM_HID Class Spec release number */
  0x01,
  0x00,                                               /* bCountryCode: Hardware target country */
  0x01,                                               /* bNumDescriptors: Number of CUSTOM_HID class descriptors
                                                         to follow */
  0x22,                                               /* bDescriptorType */
  LOBYTE(USBD_CUSTOM_HID_REPORT_DESC_SIZE),                   /* wItemLength: Total length of Report descriptor */
  HIBYTE(USBD_CUSTOM_HID_REPORT_DESC_SIZE),
};

static uint8_t CUSTOMHIDInEpAdd = CUSTOM_HID_EPIN_ADDR;
static uint8_t CUSTOMHIDOutEpAdd = CUSTOM_HID_EPOUT_ADDR;

/**
  * @brief  USBD_CUSTOM_HID_Init
  *         Initialize the CUSTOM_HID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_CUSTOM_HID_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);
  USBD_CUSTOM_HID_HandleTypeDef *hhid;

  hhid = (USBD_CUSTOM_HID_HandleTypeDef *)USBD_malloc(sizeof(USBD_CUSTOM_HID_HandleTypeDef));

  if (hhid == NULL)
  {
    pdev->pClassDataCmsit[pdev->classId] = NULL;
    return (uint8_t)USBD_EMEM;
  }

  pdev->pClassDataCmsit[pdev->classId] = (void *)hhid;
  pdev->pClassData = pdev->pClassDataCmsit[pdev->classId];

  /* Get the Endpoints addresses allocated for this class instance */
  CUSTOMHIDInEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_INTR, (uint8_t)pdev->classId);
  CUSTOMHIDOutEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_INTR, (uint8_t)pdev->classId);

  if (pdev->dev_speed == USBD_SPEED_HIGH)
  {
    pdev->ep_in[CUSTOMHIDInEpAdd & 0xFU].bInterval = CUSTOM_HID_HS_BINTERVAL;
    pdev->ep_out[CUSTOMHIDOutEpAdd & 0xFU].bInterval = CUSTOM_HID_HS_BINTERVAL;
  }
  else   /* LOW and FULL-speed endpoints */
  {
    pdev->ep_in[CUSTOMHIDInEpAdd & 0xFU].bInterval = CUSTOM_HID_FS_BINTERVAL;
    pdev->ep_out[CUSTOMHIDOutEpAdd & 0xFU].bInterval = CUSTOM_HID_FS_BINTERVAL;
  }

  /* Open EP IN */
  (void)USBD_LL_OpenEP(pdev, CUSTOMHIDInEpAdd, USBD_EP_TYPE_INTR,
                       CUSTOM_HID_EPIN_SIZE);

  pdev->ep_in[CUSTOMHIDInEpAdd & 0xFU].is_used = 1U;

  if (USBD_CUSTOMHID_OUTREPORT_BUF_SIZE < CUSTOM_HID_EPOUT_SIZE)
  {
    return (uint8_t)USBD_FAIL;
  }

  /* Open EP OUT */
  (void)USBD_LL_OpenEP(pdev, CUSTOMHIDOutEpAdd, USBD_EP_TYPE_INTR,
                       CUSTOM_HID_EPOUT_SIZE);

  pdev->ep_out[CUSTOMHIDOutEpAdd & 0xFU].is_used = 1U;

  hhid->state = CUSTOM_HID_IDLE;

  ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->Init();

#ifndef USBD_CUSTOMHID_OUT_PREPARE_RECEIVE_DISABLED
  /* Prepare Out endpoint to receive 1st packet */
  (void)USBD_LL_PrepareReceive(pdev, CUSTOMHIDOutEpAdd, hhid->Report_buf,
                               USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
#endif /* USBD_CUSTOMHID_OUT_PREPARE_RECEIVE_DISABLED */

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_HID_Init
  *         DeInitialize the CUSTOM_HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_CUSTOM_HID_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  CUSTOMHIDInEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_INTR, (uint8_t)pdev->classId);
  CUSTOMHIDOutEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_INTR, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  /* Close CUSTOM_HID EP IN */
  (void)USBD_LL_CloseEP(pdev, CUSTOMHIDInEpAdd);
  pdev->ep_in[CUSTOMHIDInEpAdd & 0xFU].is_used = 0U;
  pdev->ep_in[CUSTOMHIDInEpAdd & 0xFU].bInterval = 0U;

  /* Close CUSTOM_HID EP OUT */
  (void)USBD_LL_CloseEP(pdev, CUSTOMHIDOutEpAdd);
  pdev->ep_out[CUSTOMHIDOutEpAdd & 0xFU].is_used = 0U;
  pdev->ep_out[CUSTOMHIDOutEpAdd & 0xFU].bInterval = 0U;

  /* Free allocated memory */
  if (pdev->pClassDataCmsit[pdev->classId] != NULL)
  {
    ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->DeInit();
    USBD_free(pdev->pClassDataCmsit[pdev->classId]);
    pdev->pClassDataCmsit[pdev->classId] = NULL;
    pdev->pClassData = NULL;
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_HID_Setup
  *         Handle the CUSTOM_HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t USBD_CUSTOM_HID_Setup(USBD_HandleTypeDef *pdev,
                                     USBD_SetupReqTypedef *req)
{
  USBD_CUSTOM_HID_HandleTypeDef *hhid = (USBD_CUSTOM_HID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint16_t len = 0U;
#ifdef USBD_CUSTOMHID_CTRL_REQ_GET_REPORT_ENABLED
  uint16_t ReportLength = 0U;
#endif /* USBD_CUSTOMHID_CTRL_REQ_GET_REPORT_ENABLED */
  uint8_t  *pbuf = NULL;
  uint16_t status_info = 0U;
  USBD_StatusTypeDef ret = USBD_OK;

  if (hhid == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_CLASS:
      switch (req->bRequest)
      {
        case CUSTOM_HID_REQ_SET_PROTOCOL:
          hhid->Protocol = (uint8_t)(req->wValue);
          break;

        case CUSTOM_HID_REQ_GET_PROTOCOL:
          (void)USBD_CtlSendData(pdev, (uint8_t *)&hhid->Protocol, 1U);
          break;

        case CUSTOM_HID_REQ_SET_IDLE:
          hhid->IdleState = (uint8_t)(req->wValue >> 8);
          break;

        case CUSTOM_HID_REQ_GET_IDLE:
          (void)USBD_CtlSendData(pdev, (uint8_t *)&hhid->IdleState, 1U);
          break;

        case CUSTOM_HID_REQ_SET_REPORT:
#ifdef USBD_CUSTOMHID_CTRL_REQ_COMPLETE_CALLBACK_ENABLED
          if (((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->CtrlReqComplete != NULL)
          {
            /* Let the application decide when to enable EP0 to receive the next report */
            ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->CtrlReqComplete(req->bRequest,
                                                                                            req->wLength);
          }
#endif /* USBD_CUSTOMHID_CTRL_REQ_COMPLETE_CALLBACK_ENABLED */
#ifndef USBD_CUSTOMHID_EP0_OUT_PREPARE_RECEIVE_DISABLED

          if (req->wLength > USBD_CUSTOMHID_OUTREPORT_BUF_SIZE)
          {
            /* Stall EP0 */
            USBD_CtlError(pdev, req);
            return USBD_FAIL;
          }

          hhid->IsReportAvailable = 1U;

          (void)USBD_CtlPrepareRx(pdev, hhid->Report_buf, req->wLength);
#endif /* USBD_CUSTOMHID_EP0_OUT_PREPARE_RECEIVE_DISABLED */
          break;
#ifdef USBD_CUSTOMHID_CTRL_REQ_GET_REPORT_ENABLED
        case CUSTOM_HID_REQ_GET_REPORT:
          if (((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->GetReport != NULL)
          {
            ReportLength = req->wLength;

            /* Get report data buffer */
            pbuf = ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->GetReport(&ReportLength);
          }

          if ((pbuf != NULL) && (ReportLength != 0U))
          {
            len = MIN(ReportLength, req->wLength);

            /* Send the report data over EP0 */
            (void)USBD_CtlSendData(pdev, pbuf, len);
          }
          else
          {
#ifdef USBD_CUSTOMHID_CTRL_REQ_COMPLETE_CALLBACK_ENABLED
            if (((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->CtrlReqComplete != NULL)
            {
              /* Let the application decide what to do, keep EP0 data phase in NAK state and
                 use USBD_CtlSendData() when data become available or stall the EP0 data phase */
              ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->CtrlReqComplete(req->bRequest,
                                                                                              req->wLength);
            }
            else
            {
              /* Stall EP0 if no data available */
              USBD_CtlError(pdev, req);
            }
#else
            /* Stall EP0 if no data available */
            USBD_CtlError(pdev, req);
#endif /* USBD_CUSTOMHID_CTRL_REQ_COMPLETE_CALLBACK_ENABLED */
          }
          break;
#endif /* USBD_CUSTOMHID_CTRL_REQ_GET_REPORT_ENABLED */

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        case USB_REQ_GET_STATUS:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            (void)USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_GET_DESCRIPTOR:
          if ((req->wValue >> 8) == CUSTOM_HID_REPORT_DESC)
          {
            len = MIN(USBD_CUSTOM_HID_REPORT_DESC_SIZE, req->wLength);
            pbuf = ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->pReport;
          }
          else
          {
            if ((req->wValue >> 8) == CUSTOM_HID_DESCRIPTOR_TYPE)
            {
              pbuf = USBD_CUSTOM_HID_Desc;
              len = MIN(USB_CUSTOM_HID_DESC_SIZ, req->wLength);
            }
          }

          if (pbuf != NULL)
          {
            (void)USBD_CtlSendData(pdev, pbuf, len);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_GET_INTERFACE:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            (void)USBD_CtlSendData(pdev, (uint8_t *)&hhid->AltSetting, 1U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_SET_INTERFACE:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            hhid->AltSetting = (uint8_t)(req->wValue);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_CLEAR_FEATURE:
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
  }
  return (uint8_t)ret;
}

/**
  * @brief  USBD_CUSTOM_HID_SendReport
  *         Send CUSTOM_HID Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @param  ClassId: The Class ID
  * @retval status
  */
uint8_t USBD_CUSTOM_HID_SendReport(USBD_HandleTypeDef *pdev,
                                   uint8_t *report, uint16_t len, uint8_t ClassId)
{
  USBD_CUSTOM_HID_HandleTypeDef *hhid = (USBD_CUSTOM_HID_HandleTypeDef *)pdev->pClassDataCmsit[ClassId];

  if (hhid == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  /* Get Endpoint IN address allocated for this class instance */
  CUSTOMHIDInEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_INTR, ClassId);

  if (pdev->dev_state == USBD_STATE_CONFIGURED)
  {
    if (hhid->state == CUSTOM_HID_IDLE)
    {
      hhid->state = CUSTOM_HID_BUSY;
      (void)USBD_LL_Transmit(pdev, CUSTOMHIDInEpAdd, report, len);
    }
    else
    {
      return (uint8_t)USBD_BUSY;
    }
  }
  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_HID_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_CUSTOM_HID_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  UNUSED(epnum);

  /* Ensure that the FIFO is empty before a new transfer, this condition could
  be caused by  a new transfer before the end of the previous transfer */
  ((USBD_CUSTOM_HID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId])->state = CUSTOM_HID_IDLE;

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_HID_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_CUSTOM_HID_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  UNUSED(epnum);
  USBD_CUSTOM_HID_HandleTypeDef *hhid;

  if (pdev->pClassDataCmsit[pdev->classId] == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  hhid = (USBD_CUSTOM_HID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  /* USB data will be immediately processed, this allow next USB traffic being
  NAKed till the end of the application processing */

#ifdef USBD_CUSTOMHID_REPORT_BUFFER_EVENT_ENABLED
  ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->OutEvent(hhid->Report_buf);
#else
  ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->OutEvent(hhid->Report_buf[0],
                                                                           hhid->Report_buf[1]);
#endif /* USBD_CUSTOMHID_REPORT_BUFFER_EVENT_ENABLED */

  return (uint8_t)USBD_OK;
}


/**
  * @brief  USBD_CUSTOM_HID_ReceivePacket
  *         prepare OUT Endpoint for reception
  * @param  pdev: device instance
  * @retval status
  */
uint8_t USBD_CUSTOM_HID_ReceivePacket(USBD_HandleTypeDef *pdev)
{
  USBD_CUSTOM_HID_HandleTypeDef *hhid;

  if (pdev->pClassDataCmsit[pdev->classId] == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  /* Get OUT Endpoint address allocated for this class instance */
  CUSTOMHIDOutEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_INTR, (uint8_t)pdev->classId);

  hhid = (USBD_CUSTOM_HID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  /* Resume USB Out process */
  (void)USBD_LL_PrepareReceive(pdev, CUSTOMHIDOutEpAdd, hhid->Report_buf,
                               USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);

  return (uint8_t)USBD_OK;
}


/**
  * @brief  USBD_CUSTOM_HID_EP0_RxReady
  *         Handles control request data.
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_CUSTOM_HID_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
  USBD_CUSTOM_HID_HandleTypeDef *hhid = (USBD_CUSTOM_HID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  if (hhid == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  if (hhid->IsReportAvailable == 1U)
  {
#ifdef USBD_CUSTOMHID_REPORT_BUFFER_EVENT_ENABLED
    ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->OutEvent(hhid->Report_buf);
#else
    ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->OutEvent(hhid->Report_buf[0],
                                                                             hhid->Report_buf[1]);
#endif /* USBD_CUSTOMHID_REPORT_BUFFER_EVENT_ENABLED */
    hhid->IsReportAvailable = 0U;
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_HID_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: CUSTOMHID Interface callback
  * @retval status
  */
uint8_t USBD_CUSTOM_HID_RegisterInterface(USBD_HandleTypeDef *pdev,
                                          USBD_CUSTOM_HID_ItfTypeDef *fops)
{
  if (fops == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  pdev->pUserData[pdev->classId] = fops;

  return (uint8_t)USBD_OK;
}
