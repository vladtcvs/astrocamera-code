add_library(system_config INTERFACE)

target_include_directories(system_config SYSTEM
INTERFACE
    include
)

target_compile_definitions(system_config
  INTERFACE
    projCOVERAGE_TEST=0
)

add_library(freertos_config ALIAS system_config)
