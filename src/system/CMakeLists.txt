add_library(system STATIC
    src/syscalls.c
    src/sysmem.c
    src/system.c)

target_include_directories(system PUBLIC include)
target_link_libraries(system PUBLIC system_config STM32_Drivers)
