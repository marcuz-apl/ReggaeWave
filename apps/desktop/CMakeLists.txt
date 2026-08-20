cmake_minimum_required(VERSION 3.22)

# Check if JUCE is already available or fetch it
if(NOT TARGET juce::juce_gui_basics)
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.4
    )
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_gui_app(ReggaeWave
    PRODUCT_NAME "ReggaeWave"
    VERSION "${REGGAEWAVE_PROJECT_VERSION}"
    COMPANY_NAME "ReggaeWave"
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/Resources/reggaewave_512.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/Resources/reggaewave_32.png"
)

target_sources(ReggaeWave PRIVATE
    Source/Main.cpp
    Source/MainWindow.h
    Source/MainWindow.cpp
    Source/MainComponent.h
    Source/MainComponent.cpp
    Source/UI/ReggaeWaveTheme.h
    Source/UI/ReggaeWaveTheme.cpp
    Source/UI/RightsAttestationModal.h
    Source/UI/RightsAttestationModal.cpp
    Source/UI/TuningPanel.h
    Source/UI/TuningPanel.cpp
    Source/UI/WaveformABView.h
    Source/UI/WaveformABView.cpp
    Source/UI/ReggaeWaveIcon.h
    Source/UI/ReggaeWaveIcon.cpp
    Source/UI/ImportCard.h
    Source/UI/ImportCard.cpp
    Source/UI/StudioPlaybackCard.h
    Source/UI/StudioPlaybackCard.cpp
    Source/UI/ExportDeckCard.h
    Source/UI/ExportDeckCard.cpp
    Source/UI/InfoDialogModal.h
    Source/UI/InfoDialogModal.cpp
    Source/UI/ImportFileModal.h
    Source/UI/ImportFileModal.cpp
    Source/UI/InlineExportProgressBar.h
    Source/UI/InlineExportProgressBar.cpp
    Source/UI/ExportDialogModal.h
    Source/UI/ExportDialogModal.cpp
    Source/UI/LyricEditorView.h
    Source/UI/LyricEditorView.cpp
)

target_include_directories(ReggaeWave PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_compile_features(ReggaeWave PRIVATE cxx_std_20)

target_compile_definitions(ReggaeWave PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

target_link_libraries(ReggaeWave PRIVATE
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

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GUI_DEPS REQUIRED gtk+-3.0 alsa)
    target_include_directories(ReggaeWave PRIVATE ${GUI_DEPS_INCLUDE_DIRS})
    target_link_libraries(ReggaeWave PRIVATE ${GUI_DEPS_LIBRARIES} asound dl pthread)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../tools/generate_icons.cpp")
    add_executable(generate_icons
        ../../tools/generate_icons.cpp
        Source/UI/ReggaeWaveIcon.cpp
        Source/UI/ReggaeWaveTheme.cpp
    )
    target_include_directories(generate_icons PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)
    target_compile_definitions(generate_icons PRIVATE JUCE_USE_CURL=0 JUCE_WEB_BROWSER=0)
    target_link_libraries(generate_icons PRIVATE
        juce::juce_gui_basics
        juce::juce_graphics
        juce::juce_events
        juce::juce_core
        juce::juce_data_structures
    )
    if(UNIX AND NOT APPLE)
        target_include_directories(generate_icons PRIVATE ${GUI_DEPS_INCLUDE_DIRS})
        target_link_libraries(generate_icons PRIVATE ${GUI_DEPS_LIBRARIES} asound dl pthread)
    endif()
endif()

# Installation rules for package generation (DEB, RPM, NSIS)
install(TARGETS ReggaeWave RUNTIME DESTINATION bin)
if(UNIX AND NOT APPLE)
    install(FILES "${CMAKE_SOURCE_DIR}/infra/packaging/linux/reggaewave.desktop"
            DESTINATION share/applications)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/Resources/reggaewave_512.png"
            DESTINATION share/icons/hicolor/512x512/apps
            RENAME reggaewave.png)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/Resources/reggaewave_256.png"
            DESTINATION share/icons/hicolor/256x256/apps
            RENAME reggaewave.png)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/Resources/reggaewave_128.png"
            DESTINATION share/icons/hicolor/128x128/apps
            RENAME reggaewave.png)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/Resources/reggaewave_64.png"
            DESTINATION share/icons/hicolor/64x64/apps
            RENAME reggaewave.png)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/Resources/reggaewave_32.png"
            DESTINATION share/icons/hicolor/32x32/apps
            RENAME reggaewave.png)
endif()


