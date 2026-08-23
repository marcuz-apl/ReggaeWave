#include <catch2/catch_test_macros.hpp>

// Compile the header through its Android branch so platform-only regressions
// are caught by the host test build as well as Android CI.
#define __ANDROID__ 1
#include <reggaewave/audio/AudioExporter.hpp>

TEST_CASE("AudioExporter compiles with the Android platform branch", "[export][android]") {
    REQUIRE(reggaewave::audio::AudioExportFormat::Mp3_320Kbps
            == reggaewave::audio::AudioExportFormat::Mp3_320Kbps);
}
