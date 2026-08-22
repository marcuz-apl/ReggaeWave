cmake_minimum_required(VERSION 3.22)

# Check if JUCE is already available or fetch it
if(NOT TARGET juce::juce_gui_basics)
    set(JUCE_INSTALL OFF CACHE BOOL "Disable JUCE install rules" FORCE)
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.4
    )
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_gui_app(ReggaeWaveMobile
    PRODUCT_NAME "ReggaeWave"
    VERSION "${REGGAEWAVE_VERSION_MAJOR}.${REGGAEWAVE_VERSION_MINOR}.${REGGAEWAVE_VERSION_PATCH}"
    COMPANY_NAME "Alfazen-Inc"
    BUNDLE_IDENTIFIER "com.alfazen.reggaewave"
    DOCUMENT_BROWSER_ENABLED ON
    MICROPHONE_PERMISSION_ENABLED ON
    IPHONE_SCREEN_ORIENTATIONS UIInterfaceOrientationPortrait UIInterfaceOrientationPortraitUpsideDown
    IPAD_SCREEN_ORIENTATIONS UIInterfaceOrientationPortrait UIInterfaceOrientationPortraitUpsideDown UIInterfaceOrientationLandscapeLeft UIInterfaceOrientationLandscapeRight
    ANDROID_ACTIVITY_NAME "com.alfazen.reggaewave.ReggaeWaveActivity"
    ANDROID_MIN_SDK_VERSION 26
    ANDROID_TARGET_SDK_VERSION 34
    ANDROID_PERMISSIONS 
        android.permission.READ_MEDIA_AUDIO 
        android.permission.READ_EXTERNAL_STORAGE 
        android.permission.WRITE_EXTERNAL_STORAGE 
        android.permission.MODIFY_AUDIO_SETTINGS
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/Resources/reggaewave_512.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/Resources/reggaewave_32.png"
)

target_sources(ReggaeWaveMobile PRIVATE
    Source/MobileMain.cpp
    Source/MobileMainWindow.h
    Source/MobileMainWindow.cpp
    Source/MobileMainComponent.h
    Source/MobileMainComponent.cpp
    Source/Platform/NativeMobileSharing.h
    Source/Platform/NativeMobileSharing.cpp
    ${CMAKE_SOURCE_DIR}/apps/desktop/Source/UI/ReggaeWaveTheme.h
    ${CMAKE_SOURCE_DIR}/apps/desktop/Source/UI/ReggaeWaveTheme.cpp
    ${CMAKE_SOURCE_DIR}/apps/desktop/Source/UI/ReggaeWaveIcon.h
    ${CMAKE_SOURCE_DIR}/apps/desktop/Source/UI/ReggaeWaveIcon.cpp
    ${CMAKE_SOURCE_DIR}/apps/desktop/Source/UI/RightsAttestationModal.h
    ${CMAKE_SOURCE_DIR}/apps/desktop/Source/UI/RightsAttestationModal.cpp
    ${CMAKE_SOURCE_DIR}/apps/desktop/Source/UI/TuningPanel.h
    ${CMAKE_SOURCE_DIR}/apps/desktop/Source/UI/TuningPanel.cpp
    ${CMAKE_SOURCE_DIR}/apps/desktop/Source/UI/WaveformABView.h
    ${CMAKE_SOURCE_DIR}/apps/desktop/Source/UI/WaveformABView.cpp
    ${CMAKE_SOURCE_DIR}/apps/desktop/Source/UI/AboutDialogModal.h
    ${CMAKE_SOURCE_DIR}/apps/desktop/Source/UI/AboutDialogModal.cpp
    ${CMAKE_SOURCE_DIR}/apps/desktop/Source/UI/HelpDialogModal.h
    ${CMAKE_SOURCE_DIR}/apps/desktop/Source/UI/HelpDialogModal.cpp
    ${CMAKE_SOURCE_DIR}/apps/desktop/Source/UI/InlineExportProgressBar.h
    ${CMAKE_SOURCE_DIR}/apps/desktop/Source/UI/InlineExportProgressBar.cpp
    ${CMAKE_SOURCE_DIR}/apps/desktop/Source/UI/ExportDialogModal.h
    ${CMAKE_SOURCE_DIR}/apps/desktop/Source/UI/ExportDialogModal.cpp
    ${CMAKE_SOURCE_DIR}/apps/desktop/Source/UI/ImportFileModal.h
    ${CMAKE_SOURCE_DIR}/apps/desktop/Source/UI/ImportFileModal.cpp
)

target_include_directories(ReggaeWaveMobile PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
    ${CMAKE_SOURCE_DIR}/apps/desktop/Source
)

target_compile_features(ReggaeWaveMobile PRIVATE cxx_std_20)

target_compile_definitions(ReggaeWaveMobile PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_ALSA_MIDI=0
    JUCE_USE_ALSA_MIDI=0
    REGGAEWAVE_MOBILE=1
)

target_link_libraries(ReggaeWaveMobile PRIVATE
    reggaewave_contracts
    reggaewave_audio_engine
    reggaewave_storage
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
)

if(APPLE)
    set_source_files_properties(Source/Platform/NativeMobileSharing.cpp PROPERTIES LANGUAGE OBJCXX)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    target_link_libraries(ReggaeWaveMobile PRIVATE android log OpenSLES)
endif()

if(UNIX AND NOT APPLE AND NOT (CMAKE_SYSTEM_NAME MATCHES "Android|iOS"))
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GUI_DEPS REQUIRED gtk+-3.0 alsa)
    target_include_directories(ReggaeWaveMobile PRIVATE ${GUI_DEPS_INCLUDE_DIRS})
    target_link_libraries(ReggaeWaveMobile PRIVATE ${GUI_DEPS_LIBRARIES} asound dl pthread)
endif()
