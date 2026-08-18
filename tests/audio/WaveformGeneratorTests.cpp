#include <catch2/catch_test_macros.hpp>
#include <reggaewave/audio/WaveformGenerator.hpp>

using namespace reggaewave::audio;

TEST_CASE("WaveformGenerator produces normalized peak overview", "[ui][waveform]") {
    const size_t numSamples = 44100;
    std::vector<std::vector<float>> channels = {
        std::vector<float>(numSamples, 0.5f),
        std::vector<float>(numSamples, 0.8f)
    };

    auto peaks = WaveformGenerator::generatePeaks(channels, 120);

    REQUIRE(peaks.size() == 120);
    for (float p : peaks) {
        REQUIRE(p >= 0.05f);
        REQUIRE(p <= 1.0f);
    }
}

TEST_CASE("WaveformGenerator handles empty channels gracefully", "[ui][waveform]") {
    std::vector<std::vector<float>> empty;
    auto peaks = WaveformGenerator::generatePeaks(empty, 80);
    REQUIRE(peaks.size() == 80);
    for (float p : peaks) {
        REQUIRE(p == 0.0f);
    }
}
