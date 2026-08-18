cmake_minimum_required(VERSION 3.22)

add_library(reggaewave_audio_fixtures INTERFACE)
target_include_directories(reggaewave_audio_fixtures INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(reggaewave_audio_fixtures INTERFACE cxx_std_20)
