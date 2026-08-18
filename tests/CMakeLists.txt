cmake_minimum_required(VERSION 3.22)

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

add_executable(reggaewave_tests
    contracts/TuningParametersTests.cpp
    contracts/RightsAttestationTests.cpp
    contracts/JobStateTests.cpp
    audio/DubEffectsTests.cpp
    audio/DualTransportTests.cpp
    audio/LoudnessMeterTests.cpp
    audio/AudioValidatorTests.cpp
    audio/AudioDecoderTests.cpp
    audio/AudioNormalizerTests.cpp
    audio/StemSeparatorTests.cpp
    audio/MusicAnalyzerTests.cpp
    audio/ReggaeDrumSynthesizerTests.cpp
    audio/ReggaeBassGeneratorTests.cpp
    audio/ReggaeSkankGeneratorTests.cpp
    audio/ReggaeArrangerTests.cpp
    audio/WaveformGeneratorTests.cpp
    audio/ConversionPipelineTests.cpp
    audio/SubtitleManagerTests.cpp
    audio/LyricVisualizerTests.cpp
    audio/AudioMastererTests.cpp
    audio/AudioExporterTests.cpp
    storage/LocalDatabaseTests.cpp
    storage/RetentionManagerTests.cpp
)

target_link_libraries(reggaewave_tests PRIVATE
    Catch2::Catch2WithMain
    reggaewave_contracts
    reggaewave_audio_engine
    reggaewave_audio_fixtures
    reggaewave_storage
)

target_compile_features(reggaewave_tests PRIVATE cxx_std_20)

include(CTest)
include(Catch)
catch_discover_tests(reggaewave_tests)
