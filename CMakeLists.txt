cmake_minimum_required(VERSION 3.6)

project(mag)

# GoogleTest requires at least C++14, but mag needs at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add_library(cuda_compiler_flags INTERFACE)
# target_compile_features(cuda_compiler_flags INTERFACE cxx_std_11)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#set(CMAKE_MODULE_PATH              "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_EXPORT_COMPILE_COMMANDS  ON)

find_package(ROOT REQUIRED)
set(ROOT_ROOTCONFIG_CMD root-config)
execute_process(COMMAND ${ROOT_ROOTCONFIG_CMD} --etcdir OUTPUT_VARIABLE ROOTETC OUTPUT_STRIP_TRAILING_WHITESPACE)
message("ROOT etc dir found in " ${ROOTETC})

include_directories(${ROOT_INCLUDE_DIRS})

find_package(Boost 1.30 COMPONENTS program_options REQUIRED )

##find_package(TBB REQUIRED)

include(cmake/BuildFlags.cmake)
include_directories(src)

option(USE_CUDA "Build with CUDA support" OFF)
option(USE_CUDA1 "Build with dummy CUDA support" OFF)

add_subdirectory(src)

option(BUILD_TESTS "Build tests" ON)
IF(BUILD_TESTS)
    include(CTest)
    add_subdirectory(test)
ENDIF(BUILD_TESTS)
