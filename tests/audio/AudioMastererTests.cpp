#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <reggaewave/audio/AudioMasterer.hpp>

using namespace reggaewave::audio;

TEST_CASE("AudioMasterer achieves -14 LUFS target and enforces -1 dBTP ceiling", "[mastering]") {
    const size_t numSamples = 44100 * 2;
    std::vector<float> sine(numSamples);
    for (size_t i = 0; i < numSamples; ++i) {
        sine[i] = std::sin(2.0 * 3.1415926535 * 440.0 * i / 44100.0) * 0.2f; // Low level mix (-14 dBFS)
    }
    std::vector<std::vector<float>> mix = {sine, sine};

    auto result = AudioMasterer::master(mix, 44100.0);

    REQUIRE(result.masteredAudio.size() == 2);
    REQUIRE(result.masteredAudio[0].size() == numSamples);
    REQUIRE_THAT(result.integratedLufs, Catch::Matchers::WithinAbs(-14.0, 1.2));
    REQUIRE(result.truePeakDb <= -0.9);
    REQUIRE(result.isCompliant);
}

TEST_CASE("AudioMasterer limits hot signal without inter-sample clipping", "[mastering]") {
    const size_t numSamples = 44100;
    std::vector<float> hotSignal(numSamples, 1.5f); // Overscaled mix > 0 dBFS
    std::vector<std::vector<float>> mix = {hotSignal, hotSignal};

    auto result = AudioMasterer::master(mix, 44100.0);

    REQUIRE(result.truePeakDb <= -0.9);
    for (float s : result.masteredAudio[0]) {
        REQUIRE(std::abs(s) <= 0.892f); // -1.0 dBTP
    }
}
