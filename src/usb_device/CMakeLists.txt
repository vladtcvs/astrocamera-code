cmake_minimum_required(VERSION 3.22)

enable_language(C)

add_library(usb_device STATIC
                src/usb_device.c
                src/usbd_conf.c
                src/usbd_customhid_if.c
                src/usbd_desc.c)
            
target_include_directories(usb_device PUBLIC include)
target_link_libraries(usb_device PUBLIC system_config USB_Device_Library STM32_Drivers stm32cubemx)
