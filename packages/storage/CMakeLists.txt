cmake_minimum_required(VERSION 3.22)

add_library(reggaewave_storage INTERFACE)
target_include_directories(reggaewave_storage INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(reggaewave_storage INTERFACE cxx_std_20)
target_link_libraries(reggaewave_storage INTERFACE reggaewave_contracts)
