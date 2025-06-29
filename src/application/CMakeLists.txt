cmake_minimum_required(VERSION 3.22)

enable_language(C ASM)

add_executable(${CMAKE_PROJECT_NAME} 
                src/main.c
                src/stm32f4xx_hal_msp.c
                src/stm32f4xx_it.c
                src/sysmem.c
                src/syscalls.c
                src/system_stm32f4xx.c
                startup_stm32f446xx.s
)            
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC include)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC system_config usb_device stm32cubemx STM32_Drivers)


add_custom_target(${CMAKE_PROJECT_NAME}.hex ALL ${CMAKE_OBJCOPY} ${CMAKE_PROJECT_NAME}.elf -O ihex ${CMAKE_PROJECT_NAME}.hex DEPENDS ${CMAKE_PROJECT_NAME})
add_custom_target(${CMAKE_PROJECT_NAME}.bin ALL ${CMAKE_OBJCOPY} ${CMAKE_PROJECT_NAME}.elf -O binary ${CMAKE_PROJECT_NAME}.bin DEPENDS ${CMAKE_PROJECT_NAME})
