#include "usb.h"
#include "system_config.h"
#include "stm32f4xx.h"


void HAL_PCD_MspInit(PCD_HandleTypeDef *pcdHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (pcdHandle->Instance == USB_OTG_HS)
    {
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        /**Configure USB RST pin Output Level
        PC1    ------> USB Reset, high level active
        */
        HAL_GPIO_WritePin(USB_RST_GPIO_Port, USB_RST_Pin, GPIO_PIN_RESET);

        /*Configure GPIO pin : USB_RST_Pin */    
        GPIO_InitStruct.Pin = USB_RST_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(USB_RST_GPIO_Port, &GPIO_InitStruct);

        /**USB_OTG_HS GPIO Configuration
        PC0     ------> USB_OTG_HS_ULPI_STP
        PC2     ------> USB_OTG_HS_ULPI_DIR
        PC3     ------> USB_OTG_HS_ULPI_NXT
        PA3     ------> USB_OTG_HS_ULPI_D0
        PA5     ------> USB_OTG_HS_ULPI_CK
        PB0     ------> USB_OTG_HS_ULPI_D1
        PB1     ------> USB_OTG_HS_ULPI_D2
        PB2     ------> USB_OTG_HS_ULPI_D4
        PB10     ------> USB_OTG_HS_ULPI_D3
        PB12     ------> USB_OTG_HS_ULPI_D5
        PB13     ------> USB_OTG_HS_ULPI_D6
        PB5     ------> USB_OTG_HS_ULPI_D7
        */
        GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_5;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_10 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_5;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* Peripheral clock enable */
        __HAL_RCC_USB_OTG_HS_CLK_ENABLE();
        __HAL_RCC_USB_OTG_HS_ULPI_CLK_ENABLE();

        /* Peripheral interrupt init */
        HAL_NVIC_SetPriority(OTG_HS_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(OTG_HS_IRQn);
    }
}

void HAL_PCD_MspDeInit(PCD_HandleTypeDef *pcdHandle)
{
    if (pcdHandle->Instance == USB_OTG_HS)
    {

        /* Disable Peripheral clock */
        __HAL_RCC_USB_OTG_HS_CLK_DISABLE();
        __HAL_RCC_USB_OTG_HS_ULPI_CLK_DISABLE();

        /**USB_OTG_HS GPIO Configuration
        PC0     ------> USB_OTG_HS_ULPI_STP
        PC2     ------> USB_OTG_HS_ULPI_DIR
        PC3     ------> USB_OTG_HS_ULPI_NXT
        PA3     ------> USB_OTG_HS_ULPI_D0
        PA5     ------> USB_OTG_HS_ULPI_CK
        PB0     ------> USB_OTG_HS_ULPI_D1
        PB1     ------> USB_OTG_HS_ULPI_D2
        PB2     ------> USB_OTG_HS_ULPI_D4
        PB10     ------> USB_OTG_HS_ULPI_D3
        PB12     ------> USB_OTG_HS_ULPI_D5
        PB13     ------> USB_OTG_HS_ULPI_D6
        PB5     ------> USB_OTG_HS_ULPI_D7
        */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3);

        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_3 | GPIO_PIN_5);

        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_10 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_5);

        /* Peripheral interrupt Deinit*/
        HAL_NVIC_DisableIRQ(OTG_HS_IRQn);
    }
}
