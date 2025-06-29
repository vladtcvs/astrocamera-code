add_library(system_config INTERFACE)

target_include_directories(system_config SYSTEM
INTERFACE
    include
)

target_compile_definitions(system_config
  INTERFACE
    projCOVERAGE_TEST=0
)
