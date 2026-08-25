#===============================================================================
# ADS9324 Driver - Root CMake
# Header-only, platform-agnostic C++20 driver.
#===============================================================================

cmake_minimum_required(VERSION 3.16)

project(hf_ads9324_driver LANGUAGES CXX)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/hf_ads9324_build_settings.cmake)

if(HF_ADS9324_SOURCE_FILES)
    add_library(${HF_ADS9324_TARGET_NAME} STATIC ${HF_ADS9324_SOURCE_FILES})
    target_include_directories(${HF_ADS9324_TARGET_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>
            $<BUILD_INTERFACE:${HF_ADS9324_VERSION_HEADER_DIR}>
            $<INSTALL_INTERFACE:include>
    )
    target_compile_features(${HF_ADS9324_TARGET_NAME} PUBLIC cxx_std_20)
else()
    add_library(${HF_ADS9324_TARGET_NAME} INTERFACE)
    target_include_directories(${HF_ADS9324_TARGET_NAME}
        INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src>
            $<BUILD_INTERFACE:${HF_ADS9324_VERSION_HEADER_DIR}>
            $<INSTALL_INTERFACE:include>
    )
    target_compile_features(${HF_ADS9324_TARGET_NAME} INTERFACE cxx_std_20)
endif()

add_library(hf::ads9324 ALIAS ${HF_ADS9324_TARGET_NAME})

set_target_properties(${HF_ADS9324_TARGET_NAME} PROPERTIES VERSION ${HF_ADS9324_VERSION} SOVERSION ${HF_ADS9324_VERSION_MAJOR})

option(HF_ADS9324_ENABLE_WARNINGS "Enable extra warnings for ADS9324 driver" OFF)
if(HF_ADS9324_ENABLE_WARNINGS)
    if(TARGET ${HF_ADS9324_TARGET_NAME})
        target_compile_options(${HF_ADS9324_TARGET_NAME} INTERFACE -Wall -Wextra -Wpedantic)
    endif()
endif()

option(HF_ADS9324_BUILD_HOST_TESTS "Build host protocol unit tests" OFF)
if(HF_ADS9324_BUILD_HOST_TESTS)
    enable_testing()
    add_executable(hf_ads9324_host_tests tests/host/test_ads9324_protocol.cpp)
    target_link_libraries(hf_ads9324_host_tests PRIVATE hf::ads9324)
    target_compile_features(hf_ads9324_host_tests PRIVATE cxx_std_20)
    add_test(NAME hf_ads9324_protocol COMMAND hf_ads9324_host_tests)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS ${HF_ADS9324_TARGET_NAME} EXPORT hf_ads9324Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY inc/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h" PATTERN "*.h.in" EXCLUDE)
install(FILES "${HF_ADS9324_VERSION_HEADER}" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT hf_ads9324Targets
    FILE hf_ads9324Targets.cmake
    NAMESPACE hf::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/hf_ads9324
)

configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/cmake/hf_ads9324Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/hf_ads9324Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/hf_ads9324
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/hf_ads9324ConfigVersion.cmake"
    VERSION ${HF_ADS9324_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/hf_ads9324Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/hf_ads9324ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/hf_ads9324
)
