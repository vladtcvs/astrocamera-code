#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "usbd_def.h"
#include <usbd_core.h>

#include <camera_descriptor.h>
#include "camera_internal.h"

#define CS_INTERFACE                                    0x24U
#define VS_INPUT_HEADER                                 0x01U
#define VS_COLORFORMAT                                  0x0DU
#define VS_FORMAT_UNCOMPRESSED                          0x04U
#define VS_FRAME_UNCOMPRESSED                           0x05U

#define VC_HEADER                                     0x01U
#define VC_INPUT_TERMINAL                             0x02U
#define VC_OUTPUT_TERMINAL                            0x03U
#define VC_PROCESSING_TERMINAL                        0x05U

#define VC_CAMERA_ABSOLUTE_TIME                         (1U << 3)
#define VC_PROCESSING_GAIN                              (1U << 9)

#define TT_STREAMING                                   0x0101U
#define ITT_CAMERA                                     0x0201U

#define UVC_CC_VIDEO                                    0x0EU
#define UVC_VERSION                                     0x0150U      /* UVC 1.1 */

#define CDC_COMMUNICATION_CLASS                         0x02U
#define CDC_ACM_SUBCLASS                                0x02U
#define CDC_ACM_PROTOCOL                                0x00U
#define CDC_DATA_CLASS                                  0x0AU


#define USBD_EP_SYNCH_NONE      0x00U
#define USBD_EP_SYNCH_SYNC      0x0CU
#define USBD_EP_SYNCH_ADAPTIVE  0x08U
#define USBD_EP_SYNCH_ASYNC     0x04U

#define PC_PROTOCOL_UNDEFINED                           0x00U

static const uint8_t classSpecificInterfaceDescriptorVC[] = {
    0x0DU,                  // bLength
    CS_INTERFACE,           // bDescriptorType
    VC_HEADER,              // bDescriptorSubType
    WBVAL(UVC_VERSION),     // bcdUVC
    WBVAL(0),               // wTotalLength --- UPDATE AFTER!
    DBVAL(24000000UL),      // dwClockFrequency
    0x01U,                  // bInCollection
    CAMERA_VS_INTERFACE_ID, // baInterfaceNr
};

static const uint8_t classSpecificInterfaceDescriptorDFU[] = {
    0x09,                     // bLength
    0x21,                     // bDescriptorType = DFU Functional
    0x03,                     // bmAttributes:
                              //   Bit 3: WillDetach = 0 (we won't reset)
                              //   Bit 2: Manifestation Tolerant = 0
                              //   Bit 1: CanUpload = 1
                              //   Bit 0: CanDnload = 1
    0xFF, 0x00,               // wDetachTimeOut = 255 ms
    WBVAL(DFU_TRANSFER_SIZE), // wTransferSize = 256 bytes
    0x00, 0x01                // DFU v1.0
};

ssize_t camera_generate_descriptor(uint8_t *pConf,
                                   uint8_t fps,
                                   uint16_t width,
                                   uint16_t height,
                                   const char *FourCC,
                                   size_t maxlen)
{
    ssize_t size = 0;
    uint8_t *wTotalLength_H = NULL;
    uint8_t *wTotalLength_L = NULL;

    /* Configuration */
    {
        const uint8_t configurationDescriptor[] = {
            0x09U,                                      // bLength
            USB_DESC_TYPE_CONFIGURATION,                // bDescriptorType
            WBVAL(0),                                   // wTotalLength, UPDATE LATER!
            CAMERA_TOTAL_INTERFACES,                    // bNumInterfaces
            0x01U,                                      // bConfigurationValue
            0x00U,                                      // iConfiguration
            0x80U,                                      // bmAttributes
            USBD_MAX_POWER,                             // bMaxPower
        };
        if (size + sizeof(configurationDescriptor) > maxlen)
            return -1;

        if (pConf != NULL) {
            memcpy(pConf + size, configurationDescriptor, sizeof(configurationDescriptor));
            wTotalLength_L = pConf + size + 2;
            wTotalLength_H = pConf + size + 3;
        }
        size += sizeof(configurationDescriptor);
    }

    /* UVC IAD interface */
    {
        {
            uint8_t interfaceAssociationDescriptor[] = {
                0x08U,                                  // bLength
                USB_DESC_TYPE_IAD,                      // bDescriptorType
                CAMERA_VC_INTERFACE_ID,                 // bFirstInterface
                0x02U,                                  // bInterfaceCount
                UVC_CC_VIDEO,                           // bFunctionClass
                0x03U,                                  // bFunctionSubClass
                PC_PROTOCOL_UNDEFINED,                  // bFunctionProtocol
                0x02,                                   // iFunction
            };
            if (size + sizeof(interfaceAssociationDescriptor) > maxlen)
                return -1;

            if (pConf != NULL)
                memcpy(pConf + size, interfaceAssociationDescriptor, sizeof(interfaceAssociationDescriptor));
            size += sizeof(interfaceAssociationDescriptor);
        }
    }

    /* UVC VC intefrace */
    {
        uint16_t wTotalLengthVC = 0;
        uint8_t *wTotalLengthVC_H = NULL;
        uint8_t *wTotalLengthVC_L = NULL;

        {
            uint8_t interfaceDescriptorVC[] = {
                0x09U,                                  // bLength
                USB_DESC_TYPE_INTERFACE,                // bDescriptorType
                CAMERA_VC_INTERFACE_ID,                 // bInterfaceNumber
                0x00U,                                  // bAlternateSetting
                0x00U,                                  // bNumEndpoints
                UVC_CC_VIDEO,                           // bInterfaceClass
                0x01U,                                  // bInterfaceSubClass
                PC_PROTOCOL_UNDEFINED,                  // bInterfaceProtocol
                0x00U,                                  // iInterface
            };
            if (size + sizeof(interfaceDescriptorVC) > maxlen)
                return -1;
            if (pConf != NULL)
                memcpy(pConf + size, interfaceDescriptorVC, sizeof(interfaceDescriptorVC));
            size += sizeof(interfaceDescriptorVC);
        }

        {
            if (size + sizeof(classSpecificInterfaceDescriptorVC) > maxlen)
                return -1;
            if (pConf != NULL) {
                memcpy(pConf + size, classSpecificInterfaceDescriptorVC, sizeof(classSpecificInterfaceDescriptorVC));
                wTotalLengthVC_L = pConf + size + 5;
                wTotalLengthVC_H = pConf + size + 6;
            }
            size += sizeof(classSpecificInterfaceDescriptorVC);
            wTotalLengthVC += sizeof(classSpecificInterfaceDescriptorVC);
        }

        {
            const uint8_t inputTerminalDescriptor[] = {
                0x11U,             // bLength
                CS_INTERFACE,      // bDescriptorType
                VC_INPUT_TERMINAL, // bDescriptorSubType
                0x01U,             // bTerminalID
                WBVAL(ITT_CAMERA), // wTerminalType
                0x00U,             // bAssocTerminal
                0x00U,             // iTerminal
                WBVAL(0),          // wObjectiveFocalLengthMin
                WBVAL(0),          // wObjectiveFocalLengthMax
                WBVAL(0),          // wOcularFocalLength
                0x02U,             // bControlSize
                WBVAL(VC_CAMERA_ABSOLUTE_TIME), // bmControls
            };
            if (size + sizeof(inputTerminalDescriptor) > maxlen)
                return -1;
            if (pConf != NULL)
                memcpy(pConf + size, inputTerminalDescriptor, sizeof(inputTerminalDescriptor));
            size += sizeof(inputTerminalDescriptor);
            wTotalLengthVC += sizeof(inputTerminalDescriptor);
        }

        {
            const uint8_t processingTerminalDescriptor[] = {
                0x0CU,                      // bLength
                CS_INTERFACE,               // bDescriptorType
                VC_PROCESSING_TERMINAL,     // bDescriptorSubType
                0x02U,                      // bTerminalID
                0x01U,                      // bSourceID
                WBVAL(0x0000U),             // wMaxMultiplier
                0x02U,                      // bControlSize
                WBVAL(VC_PROCESSING_GAIN),  // bmControls
                0x00U,                      // iProcessing
                0x01U,                      // bmVideoStandards
            };
            if (size + sizeof(processingTerminalDescriptor) > maxlen)
                return -1;
            if (pConf != NULL)
                memcpy(pConf + size, processingTerminalDescriptor, sizeof(processingTerminalDescriptor));
            size += sizeof(processingTerminalDescriptor);
            wTotalLengthVC += sizeof(processingTerminalDescriptor);
        }

        {
            const uint8_t outputTerminalDescriptor[] = {
                0x09U,               // bLength
                CS_INTERFACE,        // bDescriptorType
                VC_OUTPUT_TERMINAL,  // bDescriptorSubType
                0x03U,               // bTerminalID
                WBVAL(TT_STREAMING), // wTerminalType
                0x00U,               // bAssocTerminal
                0x02U,               // bSourceID
                0x00U,               // iTerminal
            };
            if (size + sizeof(outputTerminalDescriptor) > maxlen)
                return -1;
            if (pConf != NULL)
                memcpy(pConf + size, outputTerminalDescriptor, sizeof(outputTerminalDescriptor));
            size += sizeof(outputTerminalDescriptor);
            wTotalLengthVC += sizeof(outputTerminalDescriptor);
        }

        if (pConf != NULL) {
            *wTotalLengthVC_L = LOBYTE(wTotalLengthVC);
            *wTotalLengthVC_H = HIBYTE(wTotalLengthVC);
        }
    }

    /* UVC VS interface alt 0 */
    {
        uint16_t wTotalLengthVS = 0;
        uint8_t *wTotalLengthVS_H = NULL;
        uint8_t *wTotalLengthVS_L = NULL;

        {
            const uint8_t interfaceDescriptorVS[] = {
                0x09U,                                  // bLength
                USB_DESC_TYPE_INTERFACE,                // bDescriptorType
                CAMERA_VS_INTERFACE_ID,                 // bInterfaceNumber
                0x00U,                                  // bAlternateSetting
                0x00U,                                  // bNumEndpoints
                UVC_CC_VIDEO,                           // bInterfaceClass
                0x02U,                                  // bInterfaceSubClass
                PC_PROTOCOL_UNDEFINED,                  // bInterfaceProtocol
                0x00U,                                  // iInterface
            };
            if (size + sizeof(interfaceDescriptorVS) > maxlen)
                return -1;
            if (pConf != NULL)
                memcpy(pConf + size, interfaceDescriptorVS, sizeof(interfaceDescriptorVS));
            size += sizeof(interfaceDescriptorVS);
        }

        {
            const uint8_t classSpecificInterfaceDescriptorVS[] = {
                0x0EU,           // bLength
                CS_INTERFACE,    // bDescriptorType
                VS_INPUT_HEADER, // bDescriptorSubtype
                0x01U,           // bNumFormats
                WBVAL(0),        // wTotalLength --- UPDATE LATER!
                CAMERA_UVC_EPIN, // bEndpointAddress
                0x00U,           // bmInfo
                0x03U,           // bTerminalLink
                0x01U,           // bStillCaptureMethod
                0x01U,           // bTriggerSupport
                0x00U,           // bTriggerUsage
                0x01U,           // bControlSize
                0x00U,           // bmaControls
            };
            if (size + sizeof(classSpecificInterfaceDescriptorVS) > maxlen)
                return -1;
            if (pConf != NULL) {
                memcpy(pConf + size, classSpecificInterfaceDescriptorVS, sizeof(classSpecificInterfaceDescriptorVS));
                wTotalLengthVS_L = pConf + size + 4;
                wTotalLengthVS_H = pConf + size + 5;
            }
            size += sizeof(classSpecificInterfaceDescriptorVS);
            wTotalLengthVS += sizeof(classSpecificInterfaceDescriptorVS);
        }

        {
            const uint8_t formatDescriptor[] = {
                0x1BU,                  // bLength
                CS_INTERFACE,           // bDescriptorType
                VS_FORMAT_UNCOMPRESSED, // bDescriptorSubtype
                0x01U,                  // bFormatIndex
                0x01U,                  // bNumFrameDescriptors

                FourCC[0], FourCC[1], FourCC[2], FourCC[3], // GUID
                0x00U, 0x00U,
                0x10U, 0x00U,
                0x80U, 0x00U,
                0x00U, 0xAAU, 0x00U, 0x38U, 0x9BU, 0x71U,
                UVC_BITS_PER_PIXEL, // bBitsPerPixel

                0x01U, // bDefaultFrameIndex
                0x00U, // bAspectRatioX
                0x00U, // bAspectRatioY
                0x00U, // bmInterlaceFlags
                0x00U, // bCopyProtect
            };
            if (size + sizeof(formatDescriptor) > maxlen)
                return -1;
            if (pConf != NULL)
                memcpy(pConf + size, formatDescriptor, sizeof(formatDescriptor));
            size += sizeof(formatDescriptor);
            wTotalLengthVS += sizeof(formatDescriptor);
        }

        {
            const uint8_t frameDescriptor[] = {
                0x26U,                                          // bLength
                CS_INTERFACE,                                   // bDescriptorType
                VS_FRAME_UNCOMPRESSED,                          // bDescriptorSubtype
                0x01,                                           // bFrameIndex
                0x03,                                           // bmCapabilities
                WBVAL(width),                                   // wWidth
                WBVAL(height),                                  // wHeight
                DBVAL(UVC_MIN_BIT_RATE(width, height, fps)),    // dwMinBitRate
                DBVAL(UVC_MAX_BIT_RATE(width, height, fps)),    // dwMaxBitRate
                DBVAL(width * height * 2U),                     // dwMaxVideoFrameBufSize
                DBVAL(UVC_INTERVAL(UVC_CAM_FPS_HS)),            // dwDefaultFrameInterval
                0x00,                                           // bFrameIntervalType
                DBVAL(UVC_INTERVAL(UVC_CAM_FPS_HS)),            // dwMinFrameInterval
                DBVAL(UVC_INTERVAL(UVC_CAM_FPS_HS)),            // dwMaxFrameInterval
                DBVAL(0),                                       // dwFrameIntervalStep
            };
            if (size + sizeof(frameDescriptor) > maxlen)
                return -1;
            if (pConf != NULL)
                memcpy(pConf + size, frameDescriptor, sizeof(frameDescriptor));
            size += sizeof(frameDescriptor);
            wTotalLengthVS += sizeof(frameDescriptor);
        }

        {
            const uint8_t colorMatchingDescriptor[] = {
                0x06,                    // bLength
                CS_INTERFACE,            // bDescriptorType
                VS_COLORFORMAT,          // bDescriptorSubtype
                CAMERA_UVC_COLOR_PRIMARIE,      // bColorPrimarie
                CAMERA_UVC_TFR_CHARACTERISTICS, // bTransferCharacteristics
                CAMERA_UVC_MATRIX_COEFFICIENTS, // bMatrixCoefficients
            };
            if (size + sizeof(colorMatchingDescriptor) > maxlen)
                return -1;
            if (pConf != NULL)
                memcpy(pConf + size, colorMatchingDescriptor, sizeof(colorMatchingDescriptor));
            size += sizeof(colorMatchingDescriptor);
            wTotalLengthVS += sizeof(colorMatchingDescriptor);
        }

        if (pConf != NULL) {
            *wTotalLengthVS_L = LOBYTE(wTotalLengthVS);
            *wTotalLengthVS_H = HIBYTE(wTotalLengthVS);
        }
    }

    /* UVC VS interface alt 1 */
    {
        {
            const uint8_t interfaceDescriptorVS[] = {
                0x09U,                                  // bLength
                USB_DESC_TYPE_INTERFACE,                // bDescriptorType
                CAMERA_VS_INTERFACE_ID,                 // bInterfaceNumber
                0x01U,                                  // bAlternateSetting
                0x01U,                                  // bNumEndpoints
                UVC_CC_VIDEO,                           // bInterfaceClass
                0x02U,                                  // bInterfaceSubClass
                PC_PROTOCOL_UNDEFINED,                  // bInterfaceProtocol
                0x00U,                                  // iInterface
            };
            if (size + sizeof(interfaceDescriptorVS) > maxlen)
                return -1;
            if (pConf != NULL)
                memcpy(pConf + size, interfaceDescriptorVS, sizeof(interfaceDescriptorVS));
            size += sizeof(interfaceDescriptorVS);
        }

        {
            const uint8_t isochronousVideoDataEndpointDescriptor[] = {
                0x07U,                                      // bLength
                USB_DESC_TYPE_ENDPOINT,                     // bDescriptorType
                CAMERA_UVC_EPIN,                            // bEndpointAddress
                USBD_EP_TYPE_ISOC | USBD_EP_SYNCH_ASYNC,    // bmAttributes
                WBVAL(CAMERA_UVC_EPIN_SIZE),                // wMaxPacketSize
                0x01U,                                      // bInterval
            };
            if (size + sizeof(isochronousVideoDataEndpointDescriptor) > maxlen)
                return -1;
            if (pConf != NULL)
                memcpy(pConf + size, isochronousVideoDataEndpointDescriptor, sizeof(isochronousVideoDataEndpointDescriptor));
            size += sizeof(isochronousVideoDataEndpointDescriptor);
        }
    }

    /* DFU interface */
    {
        {
            const uint8_t interfaceDescriptorDFU_alt0[] = {
                0x09,                    // bLength
                0x04,                    // bDescriptorType = Interface
                CAMERA_DFU_RUNTIME_INTERFACE_ID, // bInterfaceNumber = 3
                0x00,                    // bAlternateSetting = 0
                0x00,                    // bNumEndpoints = 0 (DFU uses only EP0)
                0xFE,                    // bInterfaceClass = Application Specific (DFU)
                0x01,                    // bInterfaceSubClass = Device Firmware Upgrade
                0x01,                    // bInterfaceProtocol = Runtime mode
                0x00,                    // iInterface = 0, no string
            };
            if (size + sizeof(interfaceDescriptorDFU_alt0) > maxlen)
                return -1;
            if (pConf != NULL)
                memcpy(pConf + size, interfaceDescriptorDFU_alt0, sizeof(interfaceDescriptorDFU_alt0));
            size += sizeof(interfaceDescriptorDFU_alt0);
        }

        {
            if (pConf != NULL)
                memcpy(pConf + size, classSpecificInterfaceDescriptorDFU, sizeof(classSpecificInterfaceDescriptorDFU));
            size += sizeof(classSpecificInterfaceDescriptorDFU);
        }
    }

    /* CDC interface */

    /* CDC IAD interface */
    {
        {
            uint8_t interfaceAssociationDescriptor[] = {
                0x08U,                                  // bLength
                USB_DESC_TYPE_IAD,                      // bDescriptorType
                CAMERA_CDC_ACM_INTERFACE_ID,            // bFirstInterface
                0x02U,                                  // bInterfaceCount
                CDC_COMMUNICATION_CLASS,                // bFunctionClass
                CDC_ACM_SUBCLASS,                       // bFunctionSubClass
                CDC_ACM_PROTOCOL,                       // bFunctionProtocol
                0x00,                                   // iFunction
            };
            if (size + sizeof(interfaceAssociationDescriptor) > maxlen)
                return -1;
            if (pConf != NULL)
                memcpy(pConf + size, interfaceAssociationDescriptor, sizeof(interfaceAssociationDescriptor));
            size += sizeof(interfaceAssociationDescriptor);
        }
    }

    /* CDC ACM interface */
    {
        {
            const uint8_t interfaceDescriptorCDC_CTL[] = {
                0x09U,                          // bLength
                USB_DESC_TYPE_INTERFACE,        // bDescriptorType
                CAMERA_CDC_ACM_INTERFACE_ID,    // bInterfaceNumber
                0x00U,                          // bAlternateSetting
                0x01U,                          // bNumEndpoints
                CDC_COMMUNICATION_CLASS,        // bInterfaceClass
                CDC_ACM_SUBCLASS,               // bInterfaceSubClass
                CDC_ACM_PROTOCOL,               // bInterfaceProtocol
                0x00U,                          // iInterface
            };
            if (size + sizeof(interfaceDescriptorCDC_CTL) > maxlen)
                return -1;
            if (pConf != NULL)
                memcpy(pConf + size, interfaceDescriptorCDC_CTL, sizeof(interfaceDescriptorCDC_CTL));
            size += sizeof(interfaceDescriptorCDC_CTL);
        }

        {
            const uint8_t functionalDescriptor[] = {
                0x05U,                          // bLength
                0x24U,                          // bDescriptorType CS_INTERFACE
                0x00U,                          // bDescriptorSubtype : Header
                0x10, 0x01,                     // bcdCDC: 1.10
            };
            if (size + sizeof(functionalDescriptor) > maxlen)
                return -1;
            if (pConf != NULL)
                memcpy(pConf + size, functionalDescriptor, sizeof(functionalDescriptor));
            size += sizeof(functionalDescriptor);
        }

        {
            const uint8_t functionalDescriptor[] = {
                0x05U,                          // bLength
                0x24U,                          // bDescriptorType CS_INTERFACE
                0x01U,                          // bDescriptorSubtype : Call management
                0x00,                           // bmCapabilities: no call mgmt
                CAMERA_CDC_DATA_INTERFACE_ID,   // bDataInterface
            };
            if (size + sizeof(functionalDescriptor) > maxlen)
                return -1;
            if (pConf != NULL)
                memcpy(pConf + size, functionalDescriptor, sizeof(functionalDescriptor));
            size += sizeof(functionalDescriptor);
        }

        {
            const uint8_t functionalDescriptor[] = {
                0x04U,                          // bLength
                0x24U,                          // bDescriptorType CS_INTERFACE
                0x02U,                          // bDescriptorSubtype : Abstract Control Management
                0x02U,                          // bmCapabilities (Set_Line_Coding, Set_Control_Line_State, etc.)
            };
            if (size + sizeof(functionalDescriptor) > maxlen)
                return -1;
            if (pConf != NULL)
                memcpy(pConf + size, functionalDescriptor, sizeof(functionalDescriptor));
            size += sizeof(functionalDescriptor);
        }

        {
            const uint8_t functionalDescriptor[] = {
                0x05,                           // bFunctionLength
                0x24,                           // bDescriptorType: CS_INTERFACE
                0x06,                           // bDescriptorSubtype: Union
                CAMERA_CDC_ACM_INTERFACE_ID,    // bMasterInterface (Comm IF)
                CAMERA_CDC_DATA_INTERFACE_ID,   // bSlaveInterface0 (Data IF)
            };
            if (size + sizeof(functionalDescriptor) > maxlen)
                return -1;
            if (pConf != NULL)
                memcpy(pConf + size, functionalDescriptor, sizeof(functionalDescriptor));
            size += sizeof(functionalDescriptor);
        }

        {
            const uint8_t epInDesc[] = {
                0x07U,                       // bLength
                USB_DESC_TYPE_ENDPOINT,      // bDescriptorType
                CAMERA_CDC_ACM_EPIN,         // bEndpointAddress
                USBD_EP_TYPE_INTR,           // bmAttributes
                WBVAL(CAMERA_CDC_ACM_EPIN_SIZE), // wMaxPacketSize
                0x10U,                       // bInterval
            };
            if (size + sizeof(epInDesc) > maxlen)
                return -1;
            if (pConf != NULL)
                memcpy(pConf + size, epInDesc, sizeof(epInDesc));
            size += sizeof(epInDesc);
        }
    }

    /* CDC DATA interface */
    {
        {
            const uint8_t interfaceDescriptorCDC_DATA[] = {
                0x09U,                          // bLength
                USB_DESC_TYPE_INTERFACE,        // bDescriptorType
                CAMERA_CDC_DATA_INTERFACE_ID,    // bInterfaceNumber
                0x00U,                          // bAlternateSetting
                0x02U,                          // bNumEndpoints
                CDC_DATA_CLASS,                 // bInterfaceClass
                0x00U,                          // bInterfaceSubClass
                PC_PROTOCOL_UNDEFINED,          // bInterfaceProtocol
                0x00U,                          // iInterface
            };
            if (size + sizeof(interfaceDescriptorCDC_DATA) > maxlen)
                return -1;
            if (pConf != NULL)
                memcpy(pConf + size, interfaceDescriptorCDC_DATA, sizeof(interfaceDescriptorCDC_DATA));
            size += sizeof(interfaceDescriptorCDC_DATA);
        }

        {
            const uint8_t epInDesc[] = {
                0x07U,                       // bLength
                USB_DESC_TYPE_ENDPOINT,      // bDescriptorType
                CAMERA_CDC_DATA_EPIN,        // bEndpointAddress
                USBD_EP_TYPE_BULK,           // bmAttributes
                WBVAL(CAMERA_CDC_DATA_EPIN_SIZE), // wMaxPacketSize
                0x10U,                       // bInterval
            };
            if (size + sizeof(epInDesc) > maxlen)
                return -1;
            if (pConf != NULL)
                memcpy(pConf + size, epInDesc, sizeof(epInDesc));
            size += sizeof(epInDesc);
        }

        {
            const uint8_t epOutDesc[] = {
                0x07U,                       // bLength
                USB_DESC_TYPE_ENDPOINT,      // bDescriptorType
                CAMERA_CDC_DATA_EPOUT,       // bEndpointAddress
                USBD_EP_TYPE_BULK,           // bmAttributes
                WBVAL(CAMERA_CDC_DATA_EPOUT_SIZE), // wMaxPacketSize
                0x10U,                       // bInterval
            };
            if (size + sizeof(epOutDesc) > maxlen)
                return -1;
            if (pConf != NULL)
                memcpy(pConf + size, epOutDesc, sizeof(epOutDesc));
            size += sizeof(epOutDesc);
        }
    }

    if (pConf != NULL) {
        *wTotalLength_L = LOBYTE(size);
        *wTotalLength_H = HIBYTE(size);
    }

    return size;
}

size_t camera_generate_descriptor_dfu(uint8_t *pConf,
                                      size_t maxlen)
{
    size_t size = 0;
    uint8_t *wTotalLength_H = NULL;
    uint8_t *wTotalLength_L = NULL;

    /* Configuration */
    {
        const uint8_t configurationDescriptor[] = {
            0x09U,                                      // bLength
            USB_DESC_TYPE_CONFIGURATION,                // bDescriptorType
            WBVAL(0),                                   // wTotalLength, UPDATE LATER!
            0x01U,                                      // bNumInterfaces
            0x01U,                                      // bConfigurationValue
            0x00U,                                      // iConfiguration
            0x80U,                                      // bmAttributes
            USBD_MAX_POWER,                             // bMaxPower
        };
        if (pConf != NULL) {
            memcpy(pConf + size, configurationDescriptor, sizeof(configurationDescriptor));
            wTotalLength_L = pConf + size + 2;
            wTotalLength_H = pConf + size + 3;
        }
        size += sizeof(configurationDescriptor);
    }

    /* DFU interface */
    {
        {
            const uint8_t interfaceDescriptorDFU_alt0[] = {
                0x09,                        // bLength
                0x04,                        // bDescriptorType = Interface
                CAMERA_DFU_DFU_INTERFACE_ID, // bInterfaceNumber = 0
                0x00,                        // bAlternateSetting = 0
                0x00,                        // bNumEndpoints = 0 (DFU uses only EP0)
                0xFE,                        // bInterfaceClass = Application Specific (DFU)
                0x01,                        // bInterfaceSubClass = Device Firmware Upgrade
                0x02,                        // bInterfaceProtocol = DFU mode
                0x30,                        // iInterface = 0x30 (Internal Flash)
            };
            if (pConf != NULL)
                memcpy(pConf + size, interfaceDescriptorDFU_alt0, sizeof(interfaceDescriptorDFU_alt0));
            size += sizeof(interfaceDescriptorDFU_alt0);
        }

        {
            if (pConf != NULL)
                memcpy(pConf + size, classSpecificInterfaceDescriptorDFU, sizeof(classSpecificInterfaceDescriptorDFU));
            size += sizeof(classSpecificInterfaceDescriptorDFU);
        }

        {
            const uint8_t interfaceDescriptorDFU_alt1[] = {
                0x09,                        // bLength
                0x04,                        // bDescriptorType = Interface
                CAMERA_DFU_DFU_INTERFACE_ID, // bInterfaceNumber = 3
                0x01,                        // bAlternateSetting = 1
                0x00,                        // bNumEndpoints = 0 (DFU uses only EP0)
                0xFE,                        // bInterfaceClass = Application Specific (DFU)
                0x01,                        // bInterfaceSubClass = Device Firmware Upgrade
                0x02,                        // bInterfaceProtocol = DFU mode
                0x31,                        // iInterface = 0x31 (Parameters)
            };
            if (pConf != NULL)
                memcpy(pConf + size, interfaceDescriptorDFU_alt1, sizeof(interfaceDescriptorDFU_alt1));
            size += sizeof(interfaceDescriptorDFU_alt1);
        }

        {
            const uint8_t interfaceDescriptorDFU_alt2[] = {
                0x09,                        // bLength
                0x04,                        // bDescriptorType = Interface
                CAMERA_DFU_DFU_INTERFACE_ID, // bInterfaceNumber = 3
                0x02,                        // bAlternateSetting = 2
                0x00,                        // bNumEndpoints = 0 (DFU uses only EP0)
                0xFE,                        // bInterfaceClass = Application Specific (DFU)
                0x01,                        // bInterfaceSubClass = Device Firmware Upgrade
                0x02,                        // bInterfaceProtocol = DFU mode
                0x32,                        // iInterface = 0x32 (FPGA)
            };
            if (pConf != NULL)
                memcpy(pConf + size, interfaceDescriptorDFU_alt2, sizeof(interfaceDescriptorDFU_alt2));
            size += sizeof(interfaceDescriptorDFU_alt2);
        }
    }

    if (pConf != NULL) {
        *wTotalLength_L = LOBYTE(size);
        *wTotalLength_H = HIBYTE(size);
    }

    return size;
}

uint8_t *camera_get_video_descriptor(size_t *len)
{
    *len = sizeof(classSpecificInterfaceDescriptorVC);
    return classSpecificInterfaceDescriptorVC;
}

uint8_t *camera_get_dfu_descriptor(size_t *len)
{
    *len = sizeof(classSpecificInterfaceDescriptorDFU);
    return classSpecificInterfaceDescriptorDFU;
}

void camera_fill_probe_control(uint8_t *probe, uint16_t width, uint16_t height)
{
    const uint8_t probe_control[] = {
        WBVAL(0x0001U),                     // bmHint
        0x01U,                              // bFormatIndex
        0x01U,                              // bFrameIndex
        DBVAL(UVC_INTERVAL(UVC_CAM_FPS_HS)),// dwFrameInterval
        WBVAL(0x0000U),                     // wKeyFrameRate
        WBVAL(0x0000U),                     // wPFrameRate
        WBVAL(0x0000U),                     // wCompQuality
        WBVAL(0x0000U),                     // wCompWindowSize
        WBVAL(0x0000U),                     // wDelay
        DBVAL(width * height * 2),          // dwMaxVideoFrameSize
        DBVAL(CAMERA_UVC_EPIN_SIZE),        // dwMaxPayloadTransferSize
        DBVAL(24000000UL),                  // dwClockFrequency
        0x00U,                              // bmFramingInfo
        0x00U,                              // bPreferedVersion
        0x00U,                              // bMinVersion
        0x00U,                              // bMaxVersion
        WBVAL(0x0000U),                     // bExtensionUnitCode
        WBVAL(0x0000U),                     // bExtensionControl
        0x00U, 0x00U, 0x00U, 0x00U, 
        0x00U, 0x00U, 0x00U, 0x00U,         // Reserved
        WBVAL(0x0000U),                     // wExtensionSelector
    };
    memcpy(probe, probe_control, sizeof(probe_control));
}
