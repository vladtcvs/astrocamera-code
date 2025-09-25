cmake_minimum_required(VERSION 3.22)

enable_language(C ASM)

add_executable(${CMAKE_PROJECT_NAME} 
                src/main.c
                src/core.c
                src/shell.c
                src/config.c
                src/ctl_spi.c
                src/hw/pll.c
                src/hw/i2c.c
                src/hw/uart.c
                src/hw/quadspi.c
                src/hw/fpga-ctl.c
                src/hw/spi.c
                src/hw/usb.c
                src/system.c
                src/sysmem.c
                ${CMAKE_SOURCE_DIR}/Drivers/CMSIS-STM32F4/Source/Templates/system_stm32f4xx.c
                ${CMAKE_SOURCE_DIR}/Drivers/CMSIS-STM32F4/Source/Templates/gcc/startup_stm32f446xx.s
)
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC include)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC system_config
                                                   usb_device
                                                   stm32cubemx
                                                   STM32_Drivers
                                                   freertos_kernel)

add_custom_target(${CMAKE_PROJECT_NAME}.hex ALL ${CMAKE_OBJCOPY} ${CMAKE_PROJECT_NAME}.elf -O ihex ${CMAKE_PROJECT_NAME}.hex DEPENDS ${CMAKE_PROJECT_NAME})
add_custom_target(${CMAKE_PROJECT_NAME}.bin ALL ${CMAKE_OBJCOPY} ${CMAKE_PROJECT_NAME}.elf -O binary ${CMAKE_PROJECT_NAME}.bin DEPENDS ${CMAKE_PROJECT_NAME})
