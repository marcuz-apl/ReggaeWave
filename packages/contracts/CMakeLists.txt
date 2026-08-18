cmake_minimum_required(VERSION 3.22)

add_library(reggaewave_contracts INTERFACE)
target_include_directories(reggaewave_contracts INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(reggaewave_contracts INTERFACE cxx_std_20)
