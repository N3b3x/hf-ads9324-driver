#===============================================================================
# ADS9324 Driver - Build Settings
# Single source of truth for version, includes, and sources.
#===============================================================================

include_guard(GLOBAL)

set(HF_ADS9324_TARGET_NAME "hf_ads9324")

set(HF_ADS9324_VERSION_MAJOR 1)
set(HF_ADS9324_VERSION_MINOR 0)
set(HF_ADS9324_VERSION_PATCH 0)
set(HF_ADS9324_VERSION "${HF_ADS9324_VERSION_MAJOR}.${HF_ADS9324_VERSION_MINOR}.${HF_ADS9324_VERSION_PATCH}")

set(HF_ADS9324_VERSION_TEMPLATE "${CMAKE_CURRENT_LIST_DIR}/../inc/ads9324_version.h.in")
set(HF_ADS9324_VERSION_HEADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/hf_ads9324_generated")
set(HF_ADS9324_VERSION_HEADER     "${HF_ADS9324_VERSION_HEADER_DIR}/ads9324_version.h")

file(MAKE_DIRECTORY "${HF_ADS9324_VERSION_HEADER_DIR}")

if(EXISTS "${HF_ADS9324_VERSION_TEMPLATE}")
    configure_file(
        "${HF_ADS9324_VERSION_TEMPLATE}"
        "${HF_ADS9324_VERSION_HEADER}"
        @ONLY
    )
    message(STATUS "ADS9324 driver v${HF_ADS9324_VERSION} — generated ads9324_version.h in ${HF_ADS9324_VERSION_HEADER_DIR}")
else()
    message(WARNING "ads9324_version.h.in not found at ${HF_ADS9324_VERSION_TEMPLATE}")
endif()

set(HF_ADS9324_PUBLIC_INCLUDE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/../inc"
    "${CMAKE_CURRENT_LIST_DIR}/../src"
    "${HF_ADS9324_VERSION_HEADER_DIR}"
)

# Header-only (implementation in ads9324.ipp)
set(HF_ADS9324_SOURCE_FILES)

set(HF_ADS9324_IDF_REQUIRES driver)
