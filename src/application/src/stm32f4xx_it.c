#include "system_config.h"
#include "stm32f4xx_it.h"
#include "stm32f4xx.h"

#include <FreeRTOS.h>
#include <task.h>

extern PCD_HandleTypeDef hpcd_USB_OTG_HS;

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void)
{

    while (1)
    {
    }
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void)
{

    while (1)
    {
    }
}

/**
 * @brief This function handles Memory management fault.
 */
void MemManage_Handler(void)
{

    while (1)
    {
    }
}

/**
 * @brief This function handles Pre-fetch fault, memory access fault.
 */
void BusFault_Handler(void)
{

    while (1)
    {
    }
}

/**
 * @brief This function handles Undefined instruction or illegal state.
 */
void UsageFault_Handler(void)
{

    while (1)
    {
    }
}

/**
 * @brief This function handles Debug monitor.
 */
void DebugMon_Handler(void)
{
}

#if 0
/**
 * @brief This function handles System service call via SWI instruction.
 */
void vPortSVCHandler( void );
void SVC_Handler(void)
{
    vPortSVCHandler();
}

/**
 * @brief This function handles Pendable request for system service.
 */
void xPortPendSVHandler( void );
void PendSV_Handler(void)
{
    xPortPendSVHandler();
}
#endif

/**
 * @brief This function handles System tick timer.
 */
void xPortSysTickHandler( void );
void SysTick_Handler(void)
{
    HAL_IncTick();
    xPortSysTickHandler();
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
 * @brief This function handles USB On The Go HS global interrupt.
 */
void OTG_HS_IRQHandler(void)
{
    HAL_PCD_IRQHandler(&hpcd_USB_OTG_HS);
}


