/**
  ******************************************************************************
  * @file    usbd_composite_builder.c
  * @author  MCD Application Team
  * @brief   This file provides all the composite builder functions.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
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
  *                                Composite Builder  Description
  *          ===================================================================
  *
  *           The composite builder builds the configuration descriptors based on
  *           the selection of classes by user.
  *           It includes all USB Device classes in order to instantiate their
  *           descriptors, but for better management, it is possible to optimize
  *           footprint by removing unused classes. It is possible to do so by
  *           commenting the relative define in usbd_conf.h.
  *
  *  @endverbatim
  *
  ******************************************************************************
  */

/* BSPDependencies
- None
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbd_composite_builder.h"

uint8_t  *USBD_CMPSIT_GetFSCfgDesc(uint16_t *length);
#ifdef USE_USB_HS
uint8_t  *USBD_CMPSIT_GetHSCfgDesc(uint16_t *length);
#endif /* USE_USB_HS */

uint8_t  *USBD_CMPSIT_GetOtherSpeedCfgDesc(uint16_t *length);

uint8_t  *USBD_CMPSIT_GetDeviceQualifierDescriptor(uint16_t *length);

static uint8_t USBD_CMPSIT_FindFreeIFNbr(USBD_HandleTypeDef *pdev);

static void  USBD_CMPSIT_AddConfDesc(uint32_t Conf, __IO uint32_t *pSze);

static void  USBD_CMPSIT_AssignEp(USBD_HandleTypeDef *pdev, uint8_t Add, uint8_t Type, uint32_t Sze);


#if USBD_CMPSIT_ACTIVATE_DFU == 1U
static void  USBD_CMPSIT_DFUDesc(USBD_HandleTypeDef *pdev, uint32_t pConf, __IO uint32_t *Sze, uint8_t speed);
#endif /* USBD_CMPSIT_ACTIVATE_DFU == 1U */

#if USBD_CMPSIT_ACTIVATE_CUSTOMHID == 1
static void  USBD_CMPSIT_CUSTOMHIDDesc(USBD_HandleTypeDef *pdev, uint32_t pConf, __IO uint32_t *Sze, uint8_t speed);
#endif /* USBD_CMPSIT_ACTIVATE_CUSTOMHID == 1U */

#if USBD_CMPSIT_ACTIVATE_VIDEO == 1U
static void  USBD_CMPSIT_VIDEODesc(USBD_HandleTypeDef *pdev, uint32_t pConf, __IO uint32_t *Sze, uint8_t speed);
#endif /* USBD_CMPSIT_ACTIVATE_VIDEO == 1U */

/* This structure is used only for the Configuration descriptors and Device Qualifier */
USBD_ClassTypeDef  USBD_CMPSIT =
{
  NULL, /* Init, */
  NULL, /* DeInit, */
  NULL, /* Setup, */
  NULL, /* EP0_TxSent, */
  NULL, /* EP0_RxReady, */
  NULL, /* DataIn, */
  NULL, /* DataOut, */
  NULL, /* SOF,  */
  NULL,
  NULL,
#ifdef USE_USB_HS
  USBD_CMPSIT_GetHSCfgDesc,
#else
  NULL,
#endif /* USE_USB_HS */
  USBD_CMPSIT_GetFSCfgDesc,
  USBD_CMPSIT_GetOtherSpeedCfgDesc,
  USBD_CMPSIT_GetDeviceQualifierDescriptor,
#if (USBD_SUPPORT_USER_STRING_DESC == 1U)
  NULL,
#endif /* USBD_SUPPORT_USER_STRING_DESC */
};

/* The generic configuration descriptor buffer that will be filled by builder
   Size of the buffer is the maximum possible configuration descriptor size. */
__ALIGN_BEGIN static uint8_t USBD_CMPSIT_FSCfgDesc[USBD_CMPST_MAX_CONFDESC_SZ]  __ALIGN_END = {0};
static uint8_t *pCmpstFSConfDesc = USBD_CMPSIT_FSCfgDesc;
/* Variable that dynamically holds the current size of the configuration descriptor */
static __IO uint32_t CurrFSConfDescSz = 0U;

#ifdef USE_USB_HS
__ALIGN_BEGIN static uint8_t USBD_CMPSIT_HSCfgDesc[USBD_CMPST_MAX_CONFDESC_SZ]  __ALIGN_END = {0};
static uint8_t *pCmpstHSConfDesc = USBD_CMPSIT_HSCfgDesc;
/* Variable that dynamically holds the current size of the configuration descriptor */
static __IO uint32_t CurrHSConfDescSz = 0U;
#endif /* USE_USB_HS */

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CMPSIT_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC]  __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,      /* bLength */
  USB_DESC_TYPE_DEVICE_QUALIFIER,  /* bDescriptorType */
  0x00,                            /* bcdDevice low */
  0x02,                            /* bcdDevice high */
  0xEF,                            /* Class */
  0x02,                            /* SubClass */
  0x01,                            /* Protocol */
  0x40,                            /* bMaxPacketSize0 */
  0x01,                            /* bNumConfigurations */
  0x00,                            /* bReserved */
};

/**
  * @brief  USBD_CMPSIT_AddClass
  *         Register a class in the class builder
  * @param  pdev: device instance
  * @param  pclass: pointer to the class structure to be added
  * @param  class: type of the class to be added (from USBD_CompositeClassTypeDef)
  * @param  cfgidx: configuration index
  * @retval status
  */
uint8_t  USBD_CMPSIT_AddClass(USBD_HandleTypeDef *pdev,
                              USBD_ClassTypeDef *pclass,
                              USBD_CompositeClassTypeDef class,
                              uint8_t cfgidx)
{
  if ((pdev->classId < USBD_MAX_SUPPORTED_CLASS) && (pdev->tclasslist[pdev->classId].Active == 0U))
  {
    /* Store the class parameters in the global tab */
    pdev->pClass[pdev->classId] = pclass;
    pdev->tclasslist[pdev->classId].ClassId = pdev->classId;
    pdev->tclasslist[pdev->classId].Active = 1U;
    pdev->tclasslist[pdev->classId].ClassType = class;

    /* Call configuration descriptor builder and endpoint configuration builder */
    if (USBD_CMPSIT_AddToConfDesc(pdev) != (uint8_t)USBD_OK)
    {
      return (uint8_t)USBD_FAIL;
    }
  }

  UNUSED(cfgidx);

  return (uint8_t)USBD_OK;
}


/**
  * @brief  USBD_CMPSIT_AddToConfDesc
  *         Add a new class to the configuration descriptor
  * @param  pdev: device instance
  * @retval status
  */
uint8_t  USBD_CMPSIT_AddToConfDesc(USBD_HandleTypeDef *pdev)
{
  uint8_t idxIf = 0U;
  uint8_t iEp = 0U;

  /* For the first class instance, start building the config descriptor common part */
  if (pdev->classId == 0U)
  {
    /* Add configuration and IAD descriptors */
    USBD_CMPSIT_AddConfDesc((uint32_t)pCmpstFSConfDesc, &CurrFSConfDescSz);
#ifdef USE_USB_HS
    USBD_CMPSIT_AddConfDesc((uint32_t)pCmpstHSConfDesc, &CurrHSConfDescSz);
#endif /* USE_USB_HS */
  }

  switch (pdev->tclasslist[pdev->classId].ClassType)
  {
#if USBD_CMPSIT_ACTIVATE_DFU == 1
    case CLASS_TYPE_DFU:
      /* Setup Max packet sizes (for DFU, no dependency on USB Speed, both HS/FS have same packet size) */
      pdev->tclasslist[pdev->classId].CurrPcktSze = 64U;

      /* Find the first available interface slot and Assign number of interfaces */
      idxIf = USBD_CMPSIT_FindFreeIFNbr(pdev);
      pdev->tclasslist[pdev->classId].NumIf = 1U;
      pdev->tclasslist[pdev->classId].Ifs[0] = idxIf;

      /* Assign endpoint numbers */
      pdev->tclasslist[pdev->classId].NumEps = 0U; /* only EP0 is used */

      /* Configure and Append the Descriptor */
      USBD_CMPSIT_DFUDesc(pdev, (uint32_t)pCmpstFSConfDesc, &CurrFSConfDescSz, (uint8_t)USBD_SPEED_FULL);

#ifdef USE_USB_HS
      USBD_CMPSIT_DFUDesc(pdev, (uint32_t)pCmpstHSConfDesc, &CurrHSConfDescSz, (uint8_t)USBD_SPEED_HIGH);
#endif /* USE_USB_HS */

      break;
#endif /* USBD_CMPSIT_ACTIVATE_DFU */

#if USBD_CMPSIT_ACTIVATE_CUSTOMHID == 1
    case CLASS_TYPE_CHID:
      /* Setup Max packet sizes */
      pdev->tclasslist[pdev->classId].CurrPcktSze = CUSTOM_HID_EPOUT_SIZE;

      /* Find the first available interface slot and Assign number of interfaces */
      idxIf = USBD_CMPSIT_FindFreeIFNbr(pdev);
      pdev->tclasslist[pdev->classId].NumIf = 1U;
      pdev->tclasslist[pdev->classId].Ifs[0] = idxIf;

      /* Assign endpoint numbers */
      pdev->tclasslist[pdev->classId].NumEps = 2U; /* EP1_IN, EP1_OUT */

      /* Set IN endpoint slot */
      iEp = pdev->tclasslist[pdev->classId].EpAdd[0];
      USBD_CMPSIT_AssignEp(pdev, iEp, USBD_EP_TYPE_INTR,CUSTOM_HID_EPIN_SIZE);

      /* Set OUT endpoint slot */
      iEp = pdev->tclasslist[pdev->classId].EpAdd[1];
      USBD_CMPSIT_AssignEp(pdev, iEp, USBD_EP_TYPE_INTR, CUSTOM_HID_EPOUT_SIZE);

      /* Configure and Append the Descriptor */
      USBD_CMPSIT_CUSTOMHIDDesc(pdev, (uint32_t)pCmpstFSConfDesc, &CurrFSConfDescSz, (uint8_t)USBD_SPEED_FULL);

#ifdef USE_USB_HS
      USBD_CMPSIT_CUSTOMHIDDesc(pdev, (uint32_t)pCmpstHSConfDesc, &CurrHSConfDescSz, (uint8_t)USBD_SPEED_HIGH);
#endif /* USE_USB_HS */

      break;
#endif /* USBD_CMPSIT_ACTIVATE_CUSTOMHID */

#if USBD_CMPSIT_ACTIVATE_VIDEO == 1
    case CLASS_TYPE_VIDEO:
      /* Setup default Max packet size */
      pdev->tclasslist[pdev->classId].CurrPcktSze = UVC_ISO_FS_MPS;

      /* Find the first available interface slot and Assign number of interfaces */
      idxIf = USBD_CMPSIT_FindFreeIFNbr(pdev);
      pdev->tclasslist[pdev->classId].NumIf = 2U;
      pdev->tclasslist[pdev->classId].Ifs[0] = idxIf;
      pdev->tclasslist[pdev->classId].Ifs[1] = (uint8_t)(idxIf + 1U);

      /* Assign endpoint numbers */
      pdev->tclasslist[pdev->classId].NumEps = 1U; /* EP1_IN */

      /* Set IN endpoint slot */
      iEp = pdev->tclasslist[pdev->classId].EpAdd[0];

      /* Assign IN Endpoint */
      USBD_CMPSIT_AssignEp(pdev, iEp, USBD_EP_TYPE_ISOC, pdev->tclasslist[pdev->classId].CurrPcktSze);

      /* Configure and Append the Descriptor */
      USBD_CMPSIT_VIDEODesc(pdev, (uint32_t)pCmpstFSConfDesc, &CurrFSConfDescSz, (uint8_t)USBD_SPEED_FULL);

#ifdef USE_USB_HS
      USBD_CMPSIT_VIDEODesc(pdev, (uint32_t)pCmpstHSConfDesc, &CurrHSConfDescSz, (uint8_t)USBD_SPEED_HIGH);
#endif /* USE_USB_HS */

      break;
#endif /* USBD_CMPSIT_ACTIVATE_VIDEO */

    default:
      UNUSED(idxIf);
      UNUSED(iEp);
      UNUSED(USBD_CMPSIT_FindFreeIFNbr);
      UNUSED(USBD_CMPSIT_AssignEp);
      break;
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CMPSIT_GetFSCfgDesc
  *         return configuration descriptor for both FS and HS modes
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
uint8_t  *USBD_CMPSIT_GetFSCfgDesc(uint16_t *length)
{
  *length = (uint16_t)CurrFSConfDescSz;

  return USBD_CMPSIT_FSCfgDesc;
}

#ifdef USE_USB_HS
/**
  * @brief  USBD_CMPSIT_GetHSCfgDesc
  *         return configuration descriptor for both FS and HS modes
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
uint8_t  *USBD_CMPSIT_GetHSCfgDesc(uint16_t *length)
{
  *length = (uint16_t)CurrHSConfDescSz;

  return USBD_CMPSIT_HSCfgDesc;
}
#endif /* USE_USB_HS */

/**
  * @brief  USBD_CMPSIT_GetOtherSpeedCfgDesc
  *         return other speed configuration descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
uint8_t  *USBD_CMPSIT_GetOtherSpeedCfgDesc(uint16_t *length)
{
  *length = (uint16_t)CurrFSConfDescSz;

  return USBD_CMPSIT_FSCfgDesc;
}

/**
  * @brief  DeviceQualifierDescriptor
  *         return Device Qualifier descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
uint8_t  *USBD_CMPSIT_GetDeviceQualifierDescriptor(uint16_t *length)
{
  *length = (uint16_t)(sizeof(USBD_CMPSIT_DeviceQualifierDesc));
  return USBD_CMPSIT_DeviceQualifierDesc;
}

/**
  * @brief  USBD_CMPSIT_FindFreeIFNbr
  *         Find the first interface available slot
  * @param  pdev: device instance
  * @retval The interface number to be used
  */
static uint8_t USBD_CMPSIT_FindFreeIFNbr(USBD_HandleTypeDef *pdev)
{
  uint32_t idx = 0U;

  /* Unroll all already activated classes */
  for (uint32_t i = 0U; i < pdev->NumClasses; i++)
  {
    /* Unroll each class interfaces */
    for (uint32_t j = 0U; j < pdev->tclasslist[i].NumIf; j++)
    {
      /* Increment the interface counter index */
      idx++;
    }
  }

  /* Return the first available interface slot */
  return (uint8_t)idx;
}

/**
  * @brief  USBD_CMPSIT_AddToConfDesc
  *         Add a new class to the configuration descriptor
  * @param  pdev: device instance
  * @retval none
  */
static void  USBD_CMPSIT_AddConfDesc(uint32_t Conf, __IO uint32_t *pSze)
{
  /* Intermediate variable to comply with MISRA-C Rule 11.3 */
  USBD_ConfigDescTypeDef *ptr = (USBD_ConfigDescTypeDef *)Conf;

  ptr->bLength = (uint8_t)sizeof(USBD_ConfigDescTypeDef);
  ptr->bDescriptorType = USB_DESC_TYPE_CONFIGURATION;
  ptr->wTotalLength = 0U;
  ptr->bNumInterfaces = 0U;
  ptr->bConfigurationValue = 1U;
  ptr->iConfiguration = USBD_CONFIG_STR_DESC_IDX;

#if (USBD_SELF_POWERED == 1U)
  ptr->bmAttributes = 0xC0U;   /* bmAttributes: Self Powered according to user configuration */
#else
  ptr->bmAttributes = 0x80U;   /* bmAttributes: Bus Powered according to user configuration */
#endif /* USBD_SELF_POWERED */

  ptr->bMaxPower = USBD_MAX_POWER;

  *pSze += sizeof(USBD_ConfigDescTypeDef);
}

/**
  * @brief  USBD_CMPSIT_AssignEp
  *         Assign and endpoint
  * @param  pdev: device instance
  * @param  Add: Endpoint address
  * @param  Type: Endpoint type
  * @param  Sze: Endpoint max packet size
  * @retval none
  */
static void  USBD_CMPSIT_AssignEp(USBD_HandleTypeDef *pdev, uint8_t Add, uint8_t Type, uint32_t Sze)
{
  uint32_t idx = 0U;

  /* Find the first available endpoint slot */
  while (((idx < (pdev->tclasslist[pdev->classId]).NumEps) && \
          ((pdev->tclasslist[pdev->classId].Eps[idx].is_used) != 0U)))
  {
    /* Increment the index */
    idx++;
  }

  /* Configure the endpoint */
  pdev->tclasslist[pdev->classId].Eps[idx].add = Add;
  pdev->tclasslist[pdev->classId].Eps[idx].type = Type;
  pdev->tclasslist[pdev->classId].Eps[idx].size = (uint8_t)Sze;
  pdev->tclasslist[pdev->classId].Eps[idx].is_used = 1U;
}

#if USBD_CMPSIT_ACTIVATE_DFU == 1
/**
  * @brief  USBD_CMPSIT_DFUDesc
  *         Configure and Append the DFU Descriptor
  * @param  pdev: device instance
  * @param  pConf: Configuration descriptor pointer
  * @param  Sze: pointer to the current configuration descriptor size
  * @retval None
  */
static void  USBD_CMPSIT_DFUDesc(USBD_HandleTypeDef *pdev, uint32_t pConf, __IO uint32_t *Sze, uint8_t speed)
{
  static USBD_IfDescTypeDef *pIfDesc;
  static USBD_DFUFuncDescTypeDef *pDFUFuncDesc;
  uint32_t idx;
  UNUSED(speed);

  for (idx = 0U; idx < USBD_DFU_MAX_ITF_NUM; idx++)
  {
    /* Append DFU Interface descriptor to Configuration descriptor */
    __USBD_CMPSIT_SET_IF(pdev->tclasslist[pdev->classId].Ifs[0], (uint8_t)idx, 0U, 0xFEU, 0x01U, 0x02U, \
                         (uint8_t)USBD_IDX_INTERFACE_STR + 1U + (uint8_t)idx);
  }

  /* Append DFU Functional descriptor to Configuration descriptor */
  pDFUFuncDesc = ((USBD_DFUFuncDescTypeDef *)(pConf + *Sze));
  pDFUFuncDesc->bLength              = (uint8_t)sizeof(USBD_DFUFuncDescTypeDef);
  pDFUFuncDesc->bDescriptorType      = DFU_DESCRIPTOR_TYPE;
  pDFUFuncDesc->bmAttributes         = USBD_DFU_BM_ATTRIBUTES;
  pDFUFuncDesc->wDetachTimeout       = USBD_DFU_DETACH_TIMEOUT;
  pDFUFuncDesc->wTransferSze         = USBD_DFU_XFER_SIZE;
  pDFUFuncDesc->bcdDFUVersion        = 0x011AU;
  *Sze                              += (uint32_t)sizeof(USBD_DFUFuncDescTypeDef);

  /* Update Config Descriptor and IAD descriptor */
  ((USBD_ConfigDescTypeDef *)pConf)->bNumInterfaces += 1U;
  ((USBD_ConfigDescTypeDef *)pConf)->wTotalLength = (uint16_t)(*Sze);

  UNUSED(idx);
}
#endif /* USBD_CMPSIT_ACTIVATE_DFU == 1 */

#if USBD_CMPSIT_ACTIVATE_CUSTOMHID == 1
/**
  * @brief  USBD_CMPSIT_CUSTOMHIDDesc
  *         Configure and Append the MSC Descriptor
  * @param  pdev: device instance
  * @param  pConf: Configuration descriptor pointer
  * @param  Sze: pointer to the current configuration descriptor size
  * @retval None
  */
static void  USBD_CMPSIT_CUSTOMHIDDesc(USBD_HandleTypeDef *pdev, uint32_t pConf, __IO uint32_t *Sze, uint8_t speed)
{
  static USBD_IfDescTypeDef *pIfDesc;
  static USBD_EpDescTypeDef *pEpDesc;
  static USBD_DescTypeDef *pDesc;

  /* Control Interface Descriptor */
  __USBD_CMPSIT_SET_IF(pdev->tclasslist[pdev->classId].Ifs[0],  0U, 2U, 3U, 0U, 0U, 0U);

  /* Descriptor of CUSTOM_HID */
  pDesc = ((USBD_DescTypeDef *)((uint32_t)pConf + *Sze));
  pDesc->bLength = 0x09U;
  pDesc->bDescriptorTypeCHID = CUSTOM_HID_DESCRIPTOR_TYPE;
  pDesc->bcdCUSTOM_HID = 0x0111U;
  pDesc->bCountryCode = 0x00U;
  pDesc->bNumDescriptors = 0x01U;
  pDesc->bDescriptorType = 0x22U;
#ifdef USBD_CUSTOMHID_REPORT_DESC_SIZE_ENABLED
  pDesc->wItemLength = ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->wReportDescLen;
#else
  pDesc->wItemLength = USBD_CUSTOM_HID_REPORT_DESC_SIZE;
#endif /* USBD_CUSTOMHID_REPORT_DESC_SIZE_ENABLED */

  *Sze += (uint32_t)sizeof(USBD_DescTypeDef);

  /* Descriptor of Custom HID endpoints */
  /* Append Endpoint descriptor to Configuration descriptor */
  __USBD_CMPSIT_SET_EP(pdev->tclasslist[pdev->classId].Eps[0].add, \
                       USBD_EP_TYPE_INTR, CUSTOM_HID_EPIN_SIZE, CUSTOM_HID_HS_BINTERVAL, CUSTOM_HID_FS_BINTERVAL);

  /* Append Endpoint descriptor to Configuration descriptor */
  __USBD_CMPSIT_SET_EP(pdev->tclasslist[pdev->classId].Eps[1].add, \
                       USBD_EP_TYPE_INTR, CUSTOM_HID_EPOUT_SIZE, CUSTOM_HID_HS_BINTERVAL, CUSTOM_HID_FS_BINTERVAL);

  /* Update Config Descriptor and IAD descriptor */
  ((USBD_ConfigDescTypeDef *)pConf)->bNumInterfaces += 1U;
  ((USBD_ConfigDescTypeDef *)pConf)->wTotalLength = (uint16_t)(*Sze);
}
#endif /* USBD_CMPSIT_ACTIVATE_CUSTOMHID == 1U */

#if USBD_CMPSIT_ACTIVATE_VIDEO == 1
/**
  * @brief  USBD_CMPSIT_VIDEODesc
  *         Configure and Append the VIDEO Descriptor
  * @param  pdev: device instance
  * @param  pConf: Configuration descriptor pointer
  * @param  Sze: pointer to the current configuration descriptor size
  * @retval None
  */
static void  USBD_CMPSIT_VIDEODesc(USBD_HandleTypeDef *pdev, uint32_t pConf, __IO uint32_t *Sze, uint8_t speed)
{

  __ALIGN_BEGIN static uint8_t usbd_uvc_guid[16] __ALIGN_END = {DBVAL(UVC_UNCOMPRESSED_GUID), 0x00, 0x00, 0x10,
                                                                0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71
                                                               };
  static USBD_IfDescTypeDef *pIfDesc;

  USBD_specificVCInDescTypeDef            *pSVCInDesc;
  USBD_InputTerminalDescTypeDef           *pInTerDesc;
  USBD_OutputTerminalDescTypeDef          *pOuTerDesc;
  USBD_ClassSpecificVsHeaderDescTypeDef   *pSpHeaDesc;
  USBD_PayloadFormatDescTypeDef           *pPayForDesc;
  USBD_ColorMatchingDescTypeDef           *pColMaDesc;
  USBD_StandardVCDataEPDescTypeDef        *pSVCDEP;
  USBD_VIDEO_VSFrameDescTypeDef           *pClassSpecVS;

  /* Append VIDEO Interface descriptor to Configuration descriptor */
  __USBD_CMPSIT_SET_IF(pdev->tclasslist[pdev->classId].Ifs[0], 0U, 0U, UVC_CC_VIDEO, 1U, PC_PROTOCOL_UNDEFINED, 0U);

  /* Append Class-specific VC Interface Descriptor to Configuration descriptor*/
  pSVCInDesc = ((USBD_specificVCInDescTypeDef *)(pConf + *Sze));
  pSVCInDesc->bLength = (uint8_t)sizeof(USBD_specificVCInDescTypeDef);
  pSVCInDesc->bDescriptorType = CS_INTERFACE;
  pSVCInDesc->bDescriptorSubtype = VC_HEADER;
  pSVCInDesc->bcdUVC = UVC_VERSION;
  pSVCInDesc->wTotalLength = 0x001EU;
  pSVCInDesc->dwClockFrequency = 0x02DC6C00U;
  pSVCInDesc->baInterfaceNr = 0x01U;
  pSVCInDesc->iTerminal = 0x01U;
  *Sze += (uint32_t)sizeof(USBD_specificVCInDescTypeDef);

  /*Append Input Terminal Descriptor to Configuration descriptor */
  pInTerDesc = ((USBD_InputTerminalDescTypeDef *)(pConf + *Sze));
  pInTerDesc->bLength = (uint8_t)sizeof(USBD_InputTerminalDescTypeDef);
  pInTerDesc->bDescriptorType = CS_INTERFACE;
  pInTerDesc->bDescriptorSubtype = VC_INPUT_TERMINAL;
  pInTerDesc->bTerminalID = 0x01U;
  pInTerDesc->wTerminalType = ITT_VENDOR_SPECIFIC;
  pInTerDesc->bAssocTerminal = 0x00U;
  pInTerDesc->iTerminal =  0x00U;
  *Sze += (uint32_t)sizeof(USBD_InputTerminalDescTypeDef);

  /* Append Output Terminal Descriptor to Configuration descriptor */
  pOuTerDesc = ((USBD_OutputTerminalDescTypeDef *)(pConf + *Sze));
  pOuTerDesc->bLength = (uint8_t)sizeof(USBD_OutputTerminalDescTypeDef);
  pOuTerDesc->bDescriptorType = CS_INTERFACE;
  pOuTerDesc->bDescriptorSubtype = VC_OUTPUT_TERMINAL;
  pOuTerDesc->bTerminalID = 0x02U;
  pOuTerDesc->wTerminalType = TT_STREAMING;
  pOuTerDesc->bAssocTerminal = 0x00U;
  pOuTerDesc->bSourceID = 0x01U;
  pOuTerDesc->iTerminal = 0x00U;
  *Sze += (uint32_t)sizeof(USBD_OutputTerminalDescTypeDef);

  /* Standard VS (Video Streaming) Interface Descriptor */
  /* Interface 1, Alternate Setting 0 = Zero Bandwidth*/
  __USBD_CMPSIT_SET_IF(pdev->tclasslist[pdev->classId].Ifs[1], 0U, 0U, UVC_CC_VIDEO, \
                       SC_VIDEOSTREAMING, PC_PROTOCOL_UNDEFINED, 0U);

  /* Append Class-specific VS Header Descriptor (Input) to Configuration descriptor */
  pSpHeaDesc = ((USBD_ClassSpecificVsHeaderDescTypeDef *)(pConf + *Sze));
  pSpHeaDesc->bLength = (uint8_t)sizeof(USBD_ClassSpecificVsHeaderDescTypeDef);
  pSpHeaDesc->bDescriptorType = CS_INTERFACE;
  pSpHeaDesc->bDescriptorSubtype = VS_INPUT_HEADER;
  pSpHeaDesc->bNumFormats = 0x4D01U;
  pSpHeaDesc->bVideoControlSize = 0x00U;
  pSpHeaDesc->bEndPointAddress = UVC_IN_EP;
  pSpHeaDesc->bmInfo = 0x00U;
  pSpHeaDesc->bTerminalLink = 0x02U;
  pSpHeaDesc->bStillCaptureMethod = 0x00U;
  pSpHeaDesc->bTriggerSupport = 0x00U;
  pSpHeaDesc->bTriggerUsage = 0x00U;
  pSpHeaDesc->bControlSize = 0x01U;
  pSpHeaDesc->bmaControls = 0x00U;
  *Sze += (uint32_t)sizeof(USBD_ClassSpecificVsHeaderDescTypeDef);

  /* Append Payload Format Descriptor to Configuration descriptor */
  pPayForDesc = ((USBD_PayloadFormatDescTypeDef *)(pConf + *Sze));
  pPayForDesc->bLength = (uint8_t)sizeof(USBD_PayloadFormatDescTypeDef);
  pPayForDesc->bDescriptorType = CS_INTERFACE;
  pPayForDesc->bDescriptorSubType = VS_FORMAT_SUBTYPE;
  pPayForDesc->bFormatIndex = 0x01U;
  pPayForDesc->bNumFrameDescriptor = 0x01U;
#ifdef USBD_UVC_FORMAT_UNCOMPRESSED
  (void)USBD_memcpy(pPayForDesc->pGiudFormat, usbd_uvc_guid, 16);
  pPayForDesc->bBitsPerPixel = UVC_BITS_PER_PIXEL;
#else
  pPayForDesc->bmFlags = 0x01U;
#endif /* USBD_UVC_FORMAT_UNCOMPRESSED */
  pPayForDesc->bDefaultFrameIndex = 0x01U;
  pPayForDesc->bAspectRatioX = 0x00U;
  pPayForDesc->bAspectRatioY = 0x00U;
  pPayForDesc->bInterlaceFlags = 0x00U;
  pPayForDesc->bCopyProtect = 0x00U;
  *Sze += (uint32_t)sizeof(USBD_PayloadFormatDescTypeDef);

  /* Append Class-specific VS (Video Streaming) Frame Descriptor to Configuration descriptor */
  pClassSpecVS = ((USBD_VIDEO_VSFrameDescTypeDef *)(pConf + *Sze));
  pClassSpecVS->bLength = (uint8_t)sizeof(USBD_VIDEO_VSFrameDescTypeDef);
  pClassSpecVS->bDescriptorType = CS_INTERFACE;
  pClassSpecVS->bDescriptorSubType = VS_FRAME_SUBTYPE;
  pClassSpecVS->bFrameIndex = 0x01U;

#ifdef USBD_UVC_FORMAT_UNCOMPRESSED
  pClassSpecVS->bmCapabilities = 0x00U;
#else
  pClassSpecVS->bmCapabilities = 0x02U;
#endif /* USBD_UVC_FORMAT_UNCOMPRESSED */

  pClassSpecVS->wWidth = UVC_WIDTH;
  pClassSpecVS->wHeight = UVC_HEIGHT;

  if (speed == (uint8_t)USBD_SPEED_HIGH)
  {
    pClassSpecVS->dwMinBitRate = UVC_MIN_BIT_RATE(UVC_CAM_FPS_HS);
    pClassSpecVS->dwMaxBitRate = UVC_MAX_BIT_RATE(UVC_CAM_FPS_HS);
    pClassSpecVS->dwDefaultFrameInterval = UVC_INTERVAL(UVC_CAM_FPS_HS);
    pClassSpecVS->dwMinFrameInterval = UVC_INTERVAL(UVC_CAM_FPS_HS);
  }
  else
  {
    pClassSpecVS->dwMinBitRate = UVC_MIN_BIT_RATE(UVC_CAM_FPS_FS);
    pClassSpecVS->dwMaxBitRate = UVC_MAX_BIT_RATE(UVC_CAM_FPS_FS);
    pClassSpecVS->dwDefaultFrameInterval = UVC_INTERVAL(UVC_CAM_FPS_FS);
    pClassSpecVS->dwMinFrameInterval = UVC_INTERVAL(UVC_CAM_FPS_FS);
  }

  pClassSpecVS->dwMaxVideoFrameBufSize = UVC_MAX_FRAME_SIZE;
  pClassSpecVS->bFrameIntervalType = 0x01U;

  *Sze += (uint32_t)sizeof(USBD_VIDEO_VSFrameDescTypeDef);

#ifdef USBD_UVC_FORMAT_UNCOMPRESSED
  /* Append Color Matching Descriptor to Configuration descriptor */
  pColMaDesc = ((USBD_ColorMatchingDescTypeDef *)(pConf + *Sze));
  pColMaDesc->bLength = (uint8_t)sizeof(USBD_ColorMatchingDescTypeDef);
  pColMaDesc->bDescriptorType = CS_INTERFACE;
  pColMaDesc->bDescriptorSubType = VS_COLORFORMAT;
  pColMaDesc->bColorPrimarie = UVC_COLOR_PRIMARIE;
  pColMaDesc->bTransferCharacteristics = UVC_TFR_CHARACTERISTICS;
  pColMaDesc->bMatrixCoefficients = UVC_MATRIX_COEFFICIENTS;
  *Sze += (uint32_t)sizeof(USBD_ColorMatchingDescTypeDef);
#endif /* USBD_UVC_FORMAT_UNCOMPRESSED */

  /* USB Standard VS Interface  Descriptor - data transfer mode */
  /* Interface 1, Alternate Setting 1*/
  __USBD_CMPSIT_SET_IF(1U, 1U, 1U, UVC_CC_VIDEO, SC_VIDEOSTREAMING, PC_PROTOCOL_UNDEFINED, 0U);

  /* Standard VS (Video Streaming) data Endpoint */
  pSVCDEP = ((USBD_StandardVCDataEPDescTypeDef *)(pConf + *Sze));
  pSVCDEP->bLength = (uint8_t)sizeof(USBD_StandardVCDataEPDescTypeDef);
  pSVCDEP->bDescriptorType = USB_DESC_TYPE_ENDPOINT;
  pSVCDEP->bEndpointAddress = UVC_IN_EP;
  pSVCDEP->bmAttributes = 0x05U;
  pSVCDEP->bInterval = 0x01U;

  if (speed == (uint8_t)USBD_SPEED_HIGH)
  {
    pSVCDEP->wMaxPacketSize = UVC_ISO_HS_MPS;
  }
  else
  {
    pSVCDEP->wMaxPacketSize = UVC_ISO_FS_MPS;
  }

  *Sze += (uint32_t)sizeof(USBD_StandardVCDataEPDescTypeDef);

  /* Update Config Descriptor and IAD descriptor */
  ((USBD_ConfigDescTypeDef *)pConf)->bNumInterfaces += 2U;
  ((USBD_ConfigDescTypeDef *)pConf)->wTotalLength = (uint16_t)(*Sze);
}
#endif /* USBD_CMPSIT_ACTIVATE_VIDEO == 1 */


/**
  * @brief  USBD_CMPSIT_SetClassID
  *         Find and set the class ID relative to selected class type and instance
  * @param  pdev: device instance
  * @param  Class: Class type, can be CLASS_TYPE_NONE if requested to find class from setup request
  * @param  Instance: Instance number of the class (0 if first/unique instance, >0 otherwise)
  * @retval The Class ID, The pdev->classId is set with the value of the selected class ID.
  */
uint32_t  USBD_CMPSIT_SetClassID(USBD_HandleTypeDef *pdev, USBD_CompositeClassTypeDef Class, uint32_t Instance)
{
  uint32_t idx;
  uint32_t inst = 0U;

  /* Unroll all already activated classes */
  for (idx = 0U; idx < pdev->NumClasses; idx++)
  {
    /* Check if the class correspond to the requested type and if it is active */
    if (((USBD_CompositeClassTypeDef)(pdev->tclasslist[idx].ClassType) == Class) &&
        ((pdev->tclasslist[idx].Active) == 1U))
    {
      if (inst == Instance)
      {
        /* Set the new class ID */
        pdev->classId = idx;

        /* Return the class ID value */
        return (idx);
      }
      else
      {
        /* Increment instance index and look for next instance */
        inst++;
      }
    }
  }

  /* No class found, return 0xFF */
  return 0xFFU;
}

/**
  * @brief  USBD_CMPSIT_GetClassID
  *         Returns the class ID relative to selected class type and instance
  * @param  pdev: device instance
  * @param  Class: Class type, can be CLASS_TYPE_NONE if requested to find class from setup request
  * @param  Instance: Instance number of the class (0 if first/unique instance, >0 otherwise)
  * @retval The Class ID (this function does not set the pdev->classId field.
  */
uint32_t  USBD_CMPSIT_GetClassID(USBD_HandleTypeDef *pdev, USBD_CompositeClassTypeDef Class, uint32_t Instance)
{
  uint32_t idx;
  uint32_t inst = 0U;

  /* Unroll all already activated classes */
  for (idx = 0U; idx < pdev->NumClasses; idx++)
  {
    /* Check if the class correspond to the requested type and if it is active */
    if (((USBD_CompositeClassTypeDef)(pdev->tclasslist[idx].ClassType) == Class) &&
        ((pdev->tclasslist[idx].Active) == 1U))
    {
      if (inst == Instance)
      {
        /* Return the class ID value */
        return (idx);
      }
      else
      {
        /* Increment instance index and look for next instance */
        inst++;
      }
    }
  }

  /* No class found, return 0xFF */
  return 0xFFU;
}

/**
  * @brief  USBD_CMPST_ClearConfDesc
  *         Reset the configuration descriptor
  * @param  pdev: device instance (reserved for future use)
  * @retval Status.
  */
uint8_t USBD_CMPST_ClearConfDesc(USBD_HandleTypeDef *pdev)
{
  UNUSED(pdev);

  /* Reset the configuration descriptor pointer to default value and its size to zero */
  pCmpstFSConfDesc = USBD_CMPSIT_FSCfgDesc;
  CurrFSConfDescSz = 0U;

#ifdef USE_USB_HS
  pCmpstHSConfDesc = USBD_CMPSIT_HSCfgDesc;
  CurrHSConfDescSz = 0U;
#endif /* USE_USB_HS */

  /* All done, can't fail */
  return (uint8_t)USBD_OK;
}
