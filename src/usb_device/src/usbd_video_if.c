/**
 ******************************************************************************
 * @file    usbd_video_if_template.c
 * @author  MCD Application Team
 * @brief   Template file for Video Interface application layer functions
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
 */

#include "usbd_video_if.h"

const uint8_t image[] = {0x00};
const uint8_t *tImagesList[] = {image};
uint16_t tImagesSizes[] = {IMAGE_SIZE};

/* Private variables ---------------------------------------------------------*/
uint8_t img_count;

static int8_t VIDEO_Itf_Init(void);
static int8_t VIDEO_Itf_DeInit(void);
static int8_t VIDEO_Itf_Control(uint8_t cmd, uint8_t *pbuf, uint16_t length);
static int8_t VIDEO_Itf_Data(uint8_t **pbuf, uint16_t *psize, uint16_t *pcktidx);

/**
 * @}
 */

USBD_VIDEO_ItfTypeDef USBD_VIDEO_fops_FS =
    {
        VIDEO_Itf_Init,
        VIDEO_Itf_DeInit,
        VIDEO_Itf_Control,
        VIDEO_Itf_Data,
};

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  Initializes the VIDEO media low layer over USB FS IP
 * @param  VIDEOFreq: VIDEO frequency used to play the VIDEO stream.
 * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
 * @param  options: Reserved for future use
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t VIDEO_Itf_Init(void)
{
    /*
       Add your initialization code here
    */

    return (0);
}

/**
 * @brief  TEMPLATE_DeInit
 *         DeInitializes the UVC media low layer
 * @param  None
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t VIDEO_Itf_DeInit(void)
{
    /*
       Add your deinitialization code here
    */
    return (0);
}

/**
 * @brief  TEMPLATE_Control
 *         Manage the UVC class requests
 * @param  Cmd: Command code
 * @param  Buf: Buffer containing command data (request parameters)
 * @param  Len: Number of data to be sent (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t VIDEO_Itf_Control(uint8_t cmd, uint8_t *pbuf, uint16_t length)
{
    UNUSED(cmd);
    UNUSED(pbuf);
    UNUSED(length);

    return (0);
}

/**
 * @brief  TEMPLATE_Data
 *         Manage the UVC data packets
 * @param  pbuf: pointer to the buffer data to be filled
 * @param  psize: pointer tot he current packet size to be filled
 * @param  pcktidx: pointer to the current packet index in the current image
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t VIDEO_Itf_Data(uint8_t **pbuf, uint16_t *psize, uint16_t *pcktidx)
{
    /*
      Implementation of this function is mandatory to provide the video data to the USB video class
      This function shall parse the MJPEG images and provide each time the buffer packet relative to
      current packet index and its size.
      If the packet is the first packet in the current MJPEG image, then packet size shall be zero and
      the pbuf is ignored and pcktidx shall be zero.
      Below is typical implementation of this function based on a binary image template.

      Binary image template shall provide:
       - tImagesList: table containing pointers to all images
       - tImagesSizes: table containing sizes of each image respectively
       - img_count: global image counter variable
       - IMG_NBR: Total image number

       To generate such file, it is possible to use tools converting video to MJPEG then to JPEG images.

    */
    const uint8_t *(*ImagePtr) = tImagesList;
    uint32_t packet_count = (tImagesSizes[img_count]) / ((UVC_PACKET_SIZE - (UVC_HEADER_PACKET_CNT * 2U)));
    uint32_t packet_remainder = (tImagesSizes[img_count]) % ((UVC_PACKET_SIZE - (UVC_HEADER_PACKET_CNT * 2U)));
    static uint8_t packet_index = 0U;

    /* Check if end of current image has been reached */
    if (packet_index < packet_count)
    {
        /* Set the current packet size */
        *psize = (uint16_t)UVC_PACKET_SIZE;

        /* Get the pointer to the next packet to be transmitted */
        *pbuf = (uint8_t *)(*(ImagePtr + img_count) +
                            (packet_index * ((uint16_t)(UVC_PACKET_SIZE - (UVC_HEADER_PACKET_CNT * 2U)))));
    }
    else if ((packet_index == packet_count))
    {
        if (packet_remainder != 0U)
        {
            /* Get the pointer to the next packet to be transmitted */
            *pbuf = (uint8_t *)(*(ImagePtr + img_count) +
                                (packet_index * ((uint16_t)(UVC_PACKET_SIZE - (UVC_HEADER_PACKET_CNT * 2U)))));

            /* Set the current packet size */
            *psize = (uint16_t)(packet_remainder + (UVC_HEADER_PACKET_CNT * 2U));
        }
        else
        {
            packet_index++;

            /* New image to be started, send only the packet header */
            *psize = 2;
        }
    }
    else
    {
        /* New image to be started, send only the packet header */
        *psize = 2;
    }

    /* Update the packet index */
    *pcktidx = packet_index;

    /* Increment the packet count and check if it reached the end of current image buffer */
    if (packet_index++ >= (packet_count + 1U))
    {
        /* Reset the packet count to zero */
        packet_index = 0U;

        /* Move to the next image in the images table */

        img_count++;
        USBD_Delay(USBD_VIDEO_IMAGE_LAPS);
        /* Check if images count has been reached, then reset to zero (go back to first image in circular loop) */
        if (img_count == IMG_NBR)
        {
            img_count = 0U;
        }
    }

    return (0);
}
