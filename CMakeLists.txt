cmake_minimum_required(VERSION 3.22)

# Read canonical VERSION file
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" REGGAEWAVE_PROJECT_VERSION LIMIT_COUNT 1)
string(STRIP "${REGGAEWAVE_PROJECT_VERSION}" REGGAEWAVE_PROJECT_VERSION)
if(NOT REGGAEWAVE_PROJECT_VERSION)
    set(REGGAEWAVE_PROJECT_VERSION "1.2.5")
endif()

# Extract semantic major.minor.patch
string(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.([0-9]+)" REGGAEWAVE_SEMVER "${REGGAEWAVE_PROJECT_VERSION}")
if(REGGAEWAVE_SEMVER)
    set(REGGAEWAVE_VERSION_MAJOR ${CMAKE_MATCH_1})
    set(REGGAEWAVE_VERSION_MINOR ${CMAKE_MATCH_2})
    set(REGGAEWAVE_VERSION_PATCH ${CMAKE_MATCH_3})
else()
    set(REGGAEWAVE_VERSION_MAJOR 1)
    set(REGGAEWAVE_VERSION_MINOR 2)
    set(REGGAEWAVE_VERSION_PATCH 5)
endif()

project(ReggaeWave VERSION ${REGGAEWAVE_VERSION_MAJOR}.${REGGAEWAVE_VERSION_MINOR}.${REGGAEWAVE_VERSION_PATCH} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_definitions(
    REGGAEWAVE_APP_VERSION_STRING="${REGGAEWAVE_PROJECT_VERSION}"
    REGGAEWAVE_APP_SEMVER="${REGGAEWAVE_SEMVER}"
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

# Sub-packages
add_subdirectory(packages/contracts)
add_subdirectory(packages/audio-engine)
add_subdirectory(packages/audio-fixtures)
add_subdirectory(packages/storage)

# Desktop application GUI target
option(REGGAEWAVE_BUILD_DESKTOP "Build ReggaeWave desktop application" ON)
if(REGGAEWAVE_BUILD_DESKTOP AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/apps/desktop/CMakeLists.txt")
    add_subdirectory(apps/desktop)
endif()

# Unit test suite
option(REGGAEWAVE_BUILD_TESTS "Build ReggaeWave unit test suite" ON)
if(REGGAEWAVE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Packaging Configuration
include("${CMAKE_CURRENT_SOURCE_DIR}/infra/packaging/CPackConfig.cmake")

