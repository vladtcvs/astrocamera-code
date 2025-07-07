cmake_minimum_required(VERSION 3.22)

enable_language(C)

set(USB_Device_Library_Src
    ${CMAKE_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c
    ${CMAKE_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c
    ${CMAKE_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c
)

add_library(usb_device STATIC
                ${USB_Device_Library_Src}
                src/camera_descriptor.c
                src/camera.c
                src/camera_setup_req_standard.c
                src/camera_setup_req_class_uvc.c
                src/usb_device.c
                src/usbd_conf.c
                src/device_descriptor.c
                )

target_include_directories(usb_device PUBLIC include)
target_include_directories(usb_device PUBLIC
    ${CMAKE_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Core/Inc
)

target_link_libraries(usb_device PUBLIC system_config STM32_Drivers stm32cubemx)
