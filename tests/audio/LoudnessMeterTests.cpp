#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <reggaewave/audio/LoudnessMeter.hpp>
#include <vector>
#include <cmath>

using namespace reggaewave::audio;

TEST_CASE("LoudnessMeter measurement of silent audio", "[audio][loudness]") {
    std::vector<std::vector<float>> silence = {
        std::vector<float>(44100, 0.0f),
        std::vector<float>(44100, 0.0f)
    };

    auto measurement = LoudnessMeter::measure(silence, 44100.0);
    REQUIRE(measurement.integratedLufs <= -69.0);
    REQUIRE(measurement.truePeakDb <= -99.0);
}

TEST_CASE("LoudnessMeter measurement of full-scale sine wave", "[audio][loudness]") {
    const int sampleRate = 44100;
    std::vector<float> sine(sampleRate);
    for (int i = 0; i < sampleRate; ++i) {
        sine[i] = std::sin(2.0 * 3.1415926535 * 1000.0 * i / sampleRate);
    }
    std::vector<std::vector<float>> stereoSine = {sine, sine};

    auto measurement = LoudnessMeter::measure(stereoSine, sampleRate);
    // Sine wave peak is 1.0 (0 dBFS) and RMS is 0.707 (-3.01 dBFS) => integrated LUFS ~ -3.7
    REQUIRE_THAT(measurement.truePeakDb, Catch::Matchers::WithinAbs(0.0, 0.05));
    REQUIRE_THAT(measurement.integratedLufs, Catch::Matchers::WithinAbs(-3.7, 0.5));
}
