cmake_minimum_required( VERSION 3.10 )

project(mengsen)

enable_testing()

set(CMAKE_BUILD_TYPE Debug)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CXX_FLAGS
# for gdb
  -g
# for waring infomation
  -Wall
  -Wextra
  -Werror
  -Wconversion
  -Wno-unused-parameter
  -Wold-style-cast
  -Woverloaded-virtual
  -Wpointer-arith
  -Wshadow
  -Wwrite-strings
  -march=native
  -rdynamic
)

if(CMAKE_BUILD_BITS EQUAL 32)
  list(APPEND CXX_FLAGS "-m32")
endif()
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNODEBUG")
# source dir mean CMakeLists.txt dir
# binary dir mean executed cmake dir
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

find_package(Boost REQUIRED)
find_package(Threads REQUIRED)
find_library(YAML NAMES yaml-cpp)
find_library(JSON NAMES jsoncpp )

if(YAML)
  message(STATUS "found yaml-cpp")
endif()

if(JSON)
  message(STATUS "found jsoncpp")
endif()

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${Boost_INCLUDES_DIR})

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})

add_subdirectory(./coroutine)
add_subdirectory(./log)

add_executable(main.out
  main.cpp
)

target_link_libraries(main.out
  PRIVATE
    mengsen_co
    mengsen_log
    jsoncpp
    yaml-cpp
)
