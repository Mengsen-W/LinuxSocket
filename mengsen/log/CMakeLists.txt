set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/libs)
include_directories(${PROJECT_SOURCE_DIR}/log)

set(SOURCE
  log.cpp
)

add_library(mengsen_log STATIC ${SOURCE})
target_link_libraries(mengsen_log pthread)
