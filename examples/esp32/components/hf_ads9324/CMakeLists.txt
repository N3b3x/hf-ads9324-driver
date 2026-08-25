###############################################################################
# HF ADS9324 Driver - ESP-IDF Component Wrapper
###############################################################################

set(HF_ADS9324_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../.." CACHE PATH
    "Path to root of hf-ads9324-driver")

get_filename_component(HF_ADS9324_ROOT "${HF_ADS9324_ROOT}" ABSOLUTE)
if(NOT EXISTS "${HF_ADS9324_ROOT}/cmake/hf_ads9324_build_settings.cmake")
    message(FATAL_ERROR
        "Cannot find ADS9324 driver root.\n"
        "  Resolved path: ${HF_ADS9324_ROOT}\n"
        "  Expected file: ${HF_ADS9324_ROOT}/cmake/hf_ads9324_build_settings.cmake")
endif()

include("${HF_ADS9324_ROOT}/cmake/hf_ads9324_build_settings.cmake")

idf_component_register(
    SRCS ${HF_ADS9324_SOURCE_FILES}
    INCLUDE_DIRS ${HF_ADS9324_PUBLIC_INCLUDE_DIRS}
    REQUIRES ${HF_ADS9324_IDF_REQUIRES}
)

if(HF_ADS9324_SOURCE_FILES)
    target_compile_features(${COMPONENT_LIB} PRIVATE cxx_std_20)
else()
    target_compile_features(${COMPONENT_LIB} INTERFACE cxx_std_20)
endif()
