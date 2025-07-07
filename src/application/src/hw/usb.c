#include "hw/usb.h"
#include "system_config.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal_def.h"

void HAL_PCD_MspInit(PCD_HandleTypeDef *pcdHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (pcdHandle->Instance == USB_OTG_HS)
    {
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        /** Configure USB RST pin Output Level
         *  USB Reset = PC1 (active high)
         */
        HAL_GPIO_WritePin(USB_RST_GPIO_Port, USB_RST_Pin, GPIO_PIN_RESET);

        /* Configure GPIO pin : USB_RST_Pin */
        GPIO_InitStruct.Pin = USB_RST_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(USB_RST_GPIO_Port, &GPIO_InitStruct);

        /** USB_OTG_HS GPIO Configuration
         *  ULPI_STP  = PC0
         *  ULPI_DIR  = PC2
         *  ULPI_NXT  = PC3
         *  ULPI_D0   = PA3
         *  ULPI_CK   = PA5
         *  ULPI_D1   = PB0
         *  ULPI_D2   = PB1
         *  ULPI_D4   = PB2
         *  ULPI_D3   = PB10
         *  ULPI_D5   = PB12
         *  ULPI_D6   = PB13
         *  ULPI_D7   = PB5
         */
        GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_5;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_10 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_5;
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
    // Never called
    UNUSED(pcdHandle);
}
