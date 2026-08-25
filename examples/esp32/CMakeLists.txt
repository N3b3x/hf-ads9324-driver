# =============================================================================
# ADS9324 Driver Examples - Project Root (ESP32-C6)
# =============================================================================

cmake_minimum_required(VERSION 3.16)

set(DEFAULT_APP_TYPE "driver_integration_test" CACHE STRING "Default app type for requirements phase")
set(DEFAULT_BUILD_TYPE "Release" CACHE STRING "Default build type for requirements phase")
set(DEFAULT_SOURCE_FILE "driver_integration_test.cpp" CACHE STRING "Default source file for requirements phase")

if(NOT DEFINED APP_TYPE)
    message(FATAL_ERROR
        "APP_TYPE not defined. Please use build_app.sh.\n"
        "Example: ./scripts/build_app.sh ${DEFAULT_APP_TYPE} ${DEFAULT_BUILD_TYPE}")
endif()

if(NOT DEFINED BUILD_TYPE)
    message(FATAL_ERROR
        "BUILD_TYPE not defined. Please use build_app.sh.")
endif()

message(STATUS "Building app type: ${APP_TYPE}")
message(STATUS "Build type: ${BUILD_TYPE}")

set(APP_TYPE "${APP_TYPE}" CACHE STRING "App type to build" FORCE)
set(BUILD_TYPE "${BUILD_TYPE}" CACHE STRING "Build type (Debug/Release)" FORCE)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32_ads9324_${APP_TYPE}_app)
