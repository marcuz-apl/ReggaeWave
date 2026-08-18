#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <reggaewave/audio/LyricVisualizer.hpp>

using namespace reggaewave::audio;

TEST_CASE("LyricVisualizer generates accurate video frame metadata", "[visualizer]") {
    SubtitleManager subManager;
    subManager.setUserRevisions({
        {1.0, 3.0, "Singing sweet songs of melodies pure"}
    });

    // 4 seconds of stereo audio at 44.1 kHz
    std::vector<std::vector<float>> audio(2, std::vector<float>(44100 * 4, 0.4f));

    // Frame 60 at 30 fps = 2.0 seconds (inside lyric window [1.0, 3.0])
    auto frame = LyricVisualizer::renderFrame(60, 30.0, subManager, audio, "Soul Rebel");

    REQUIRE(frame.frameIndex == 60);
    REQUIRE_THAT(frame.timestampSeconds, Catch::Matchers::WithinAbs(2.0, 1e-4));
    REQUIRE(frame.width == 1920);
    REQUIRE(frame.height == 1080);
    REQUIRE(frame.activeLyricText == "Singing sweet songs of melodies pure");
    REQUIRE(frame.trackTitle == "Soul Rebel");
    REQUIRE(frame.audioRmsLevel > 0.35f);
}
