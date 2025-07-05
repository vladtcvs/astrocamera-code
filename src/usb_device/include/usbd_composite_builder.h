/**
  ******************************************************************************
  * @file    usbd_composite_builder.h
  * @author  MCD Application Team
  * @brief   Header for the usbd_composite_builder.c file
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
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_COMPOSITE_BUILDER_H__
#define __USBD_COMPOSITE_BUILDER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"

#ifndef USBD_CMPSIT_ACTIVATE_DFU
#define USBD_CMPSIT_ACTIVATE_DFU                           0U
#endif /* USBD_CMPSIT_ACTIVATE_DFU */

#ifndef USBD_CMPSIT_ACTIVATE_CUSTOMHID
#define USBD_CMPSIT_ACTIVATE_CUSTOMHID                     0U
#endif /* USBD_CMPSIT_ACTIVATE_CUSTOMHID */

#ifndef USBD_CMPSIT_ACTIVATE_VIDEO
#define USBD_CMPSIT_ACTIVATE_VIDEO                         0U
#endif /* USBD_CMPSIT_ACTIVATE_VIDEO */

#if USBD_CMPSIT_ACTIVATE_DFU == 1U
#include "usbd_dfu.h"
#endif /* USBD_CMPSIT_ACTIVATE_DFU */

#if USBD_CMPSIT_ACTIVATE_CUSTOMHID == 1
#include "usbd_customhid.h"
#endif /* USBD_CMPSIT_ACTIVATE_CUSTOMHID */

#if USBD_CMPSIT_ACTIVATE_VIDEO == 1
#include "usbd_video.h"
#endif /* USBD_CMPSIT_ACTIVATE_VIDEO */

#ifndef USBD_CMPST_MAX_CONFDESC_SZ
#define USBD_CMPST_MAX_CONFDESC_SZ                         300U
#endif /* USBD_CMPST_MAX_CONFDESC_SZ */

#ifndef USBD_CONFIG_STR_DESC_IDX
#define USBD_CONFIG_STR_DESC_IDX                           4U
#endif /* USBD_CONFIG_STR_DESC_IDX */

/* Exported types ------------------------------------------------------------*/
/* USB Iad descriptors structure */
typedef struct
{
  uint8_t           bLength;
  uint8_t           bDescriptorType;
  uint8_t           bFirstInterface;
  uint8_t           bInterfaceCount;
  uint8_t           bFunctionClass;
  uint8_t           bFunctionSubClass;
  uint8_t           bFunctionProtocol;
  uint8_t           iFunction;
} USBD_IadDescTypeDef;

/* USB interface descriptors structure */
typedef struct
{
  uint8_t           bLength;
  uint8_t           bDescriptorType;
  uint8_t           bInterfaceNumber;
  uint8_t           bAlternateSetting;
  uint8_t           bNumEndpoints;
  uint8_t           bInterfaceClass;
  uint8_t           bInterfaceSubClass;
  uint8_t           bInterfaceProtocol;
  uint8_t           iInterface;
} USBD_IfDescTypeDef;

extern USBD_ClassTypeDef  USBD_CMPSIT;

/* Exported functions prototypes ---------------------------------------------*/
uint8_t  USBD_CMPSIT_AddToConfDesc(USBD_HandleTypeDef *pdev);

#ifdef USE_USBD_COMPOSITE
uint8_t  USBD_CMPSIT_AddClass(USBD_HandleTypeDef *pdev,
                              USBD_ClassTypeDef *pclass,
                              USBD_CompositeClassTypeDef class,
                              uint8_t cfgidx);

uint32_t  USBD_CMPSIT_SetClassID(USBD_HandleTypeDef *pdev,
                                 USBD_CompositeClassTypeDef Class,
                                 uint32_t Instance);

uint32_t  USBD_CMPSIT_GetClassID(USBD_HandleTypeDef *pdev,
                                 USBD_CompositeClassTypeDef Class,
                                 uint32_t Instance);
#endif /* USE_USBD_COMPOSITE */

uint8_t USBD_CMPST_ClearConfDesc(USBD_HandleTypeDef *pdev);

/* Private macro -----------------------------------------------------------*/
#define __USBD_CMPSIT_SET_EP(epadd, eptype, epsize, HSinterval, FSinterval) \
  do { \
    /* Append Endpoint descriptor to Configuration descriptor */ \
    pEpDesc = ((USBD_EpDescTypeDef*)((uint32_t)pConf + *Sze)); \
    pEpDesc->bLength            = (uint8_t)sizeof(USBD_EpDescTypeDef); \
    pEpDesc->bDescriptorType    = USB_DESC_TYPE_ENDPOINT; \
    pEpDesc->bEndpointAddress   = (epadd); \
    pEpDesc->bmAttributes       = (eptype); \
    pEpDesc->wMaxPacketSize     = (uint16_t)(epsize); \
    if(speed == (uint8_t)USBD_SPEED_HIGH) \
    { \
      pEpDesc->bInterval        = HSinterval; \
    } \
    else \
    { \
      pEpDesc->bInterval        = FSinterval;   \
    } \
    *Sze += (uint32_t)sizeof(USBD_EpDescTypeDef); \
  } while(0)

#define __USBD_CMPSIT_SET_IF(ifnum, alt, eps, class, subclass, protocol, istring) \
  do { \
    /* Interface Descriptor */ \
    pIfDesc = ((USBD_IfDescTypeDef*)((uint32_t)pConf + *Sze)); \
    pIfDesc->bLength = (uint8_t)sizeof(USBD_IfDescTypeDef); \
    pIfDesc->bDescriptorType = USB_DESC_TYPE_INTERFACE; \
    pIfDesc->bInterfaceNumber = ifnum; \
    pIfDesc->bAlternateSetting = alt; \
    pIfDesc->bNumEndpoints = eps; \
    pIfDesc->bInterfaceClass = class; \
    pIfDesc->bInterfaceSubClass = subclass; \
    pIfDesc->bInterfaceProtocol = protocol; \
    pIfDesc->iInterface = istring; \
    *Sze += (uint32_t)sizeof(USBD_IfDescTypeDef); \
  } while(0)

#ifdef __cplusplus
}
#endif

#endif  /* __USBD_COMPOSITE_BUILDER_H__ */

/**
  * @}
  */

