cmake_minimum_required(VERSION 3.22)
project(ReggaeWave VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Sub-packages
add_subdirectory(packages/contracts)
add_subdirectory(packages/audio-engine)
add_subdirectory(packages/audio-fixtures)
add_subdirectory(packages/storage)

# Optional test building
option(REGGAEWAVE_BUILD_TESTS "Build ReggaeWave unit test suite" ON)
if(REGGAEWAVE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Optional desktop app target
option(REGGAEWAVE_BUILD_DESKTOP "Build ReggaeWave desktop application" ON)
if(REGGAEWAVE_BUILD_DESKTOP AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/apps/desktop/CMakeLists.txt")
    add_subdirectory(apps/desktop)
endif()

# Packaging Configuration
include("${CMAKE_CURRENT_SOURCE_DIR}/infra/packaging/CPackConfig.cmake")
