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
    VERSION 0.1.0
    COMPANY_NAME "ReggaeWave"
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
    pkg_check_modules(GUI_DEPS REQUIRED gtk+-x11-3.0 webkit2gtk-4.1 libcurl alsa)
    target_include_directories(ReggaeWave PRIVATE ${GUI_DEPS_INCLUDE_DIRS})
    target_link_libraries(ReggaeWave PRIVATE ${GUI_DEPS_LIBRARIES} curl asound dl pthread)
endif()
