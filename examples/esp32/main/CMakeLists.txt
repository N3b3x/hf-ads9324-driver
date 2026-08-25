# =============================================================================
# ADS9324 Driver Examples - Main Component
# =============================================================================

if(NOT DEFINED APP_TYPE)
    if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(APP_TYPE "${DEFAULT_APP_TYPE}")
        message(STATUS "APP_TYPE not defined during requirements phase, using default: ${APP_TYPE}")
    else()
        message(FATAL_ERROR
            "APP_TYPE not defined. Use ./scripts/build_app.sh ${DEFAULT_APP_TYPE} Release")
    endif()
endif()

if(NOT DEFINED APP_SOURCE_FILE)
    if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(APP_SOURCE_FILE "${DEFAULT_SOURCE_FILE}")
        message(STATUS "APP_SOURCE_FILE not defined during requirements phase, using default: ${APP_SOURCE_FILE}")
    else()
        message(FATAL_ERROR "APP_SOURCE_FILE not defined. Use build_app.sh.")
    endif()
endif()

message(STATUS "Building app type: ${APP_TYPE}")
message(STATUS "Using source file: ${APP_SOURCE_FILE}")

set(INC_BASE_PATH "../../../inc")
set(SRC_BASE_PATH "../../../src")
set(MAIN_INCLUDE_DIRS
    "."
    "${INC_BASE_PATH}"
    "${SRC_BASE_PATH}"
)

set(MAIN_REQUIRES
    driver
    esp_timer
    freertos
    spi_flash
    nvs_flash
    hf_ads9324
)

idf_component_register(
    SRCS "${APP_SOURCE_FILE}"
    INCLUDE_DIRS ${MAIN_INCLUDE_DIRS}
    REQUIRES ${MAIN_REQUIRES}
)

target_compile_features(${COMPONENT_LIB} PRIVATE cxx_std_20)

if(BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Wpedantic -O0 -g3 -DDEBUG -DADS9324_DEBUG)
else()
    target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Wpedantic -O2 -g -DNDEBUG -DADS9324_RELEASE)
endif()
