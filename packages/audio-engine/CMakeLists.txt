cmake_minimum_required(VERSION 3.22)

add_library(reggaewave_audio_engine INTERFACE)
target_include_directories(reggaewave_audio_engine INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(reggaewave_audio_engine INTERFACE reggaewave_contracts)

if(WIN32)
    target_link_libraries(reggaewave_audio_engine INTERFACE mfplat mfreadwrite mfuuid ole32)
endif()

