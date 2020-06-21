set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/libs)
include_directories(${PROJECT_SOURCE_DIR}/config)

set(SOURCE
  config.cpp
)

add_library(mengsen_config STATIC ${SOURCE})