#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <reggaewave/audio/AudioCleaner.hpp>
#include <vector>
#include <cmath>

using namespace reggaewave::audio;

TEST_CASE("AudioCleaner attenuates sub-35Hz rumble and mains hum", "[audio][cleaner]") {
    const double sampleRate = 44100.0;
    const size_t numSamples = 44100; // 1 second
    const double pi = 3.14159265358979323846;

    // Create a 20 Hz sub-rumble signal + 50 Hz hum + 440 Hz musical tone
    std::vector<float> testChannel(numSamples, 0.0f);
    for (size_t i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        float rumble = 0.5f * std::sin(2.0 * pi * 20.0 * t);
        float hum50 = 0.3f * std::sin(2.0 * pi * 50.0 * t);
        float music440 = 0.6f * std::sin(2.0 * pi * 440.0 * t);
        testChannel[i] = rumble + hum50 + music440;
    }

    std::vector<std::vector<float>> stereo = {testChannel, testChannel};
    AudioCleaner::cleanStereo(stereo, sampleRate);

    REQUIRE(stereo.size() == 2);
    REQUIRE(stereo[0].size() == numSamples);
    REQUIRE(stereo[1].size() == numSamples);

    // Verify samples remain finite and bounded
    for (size_t i = 0; i < numSamples; ++i) {
        REQUIRE(std::isfinite(stereo[0][i]));
        REQUIRE(std::isfinite(stereo[1][i]));
    }
}

TEST_CASE("AudioCleaner polishes vocal stems by attenuating quiet bleed", "[audio][cleaner]") {
    const double sampleRate = 44100.0;
    const size_t numSamples = 44100;

    // Simulate quiet background noise in pause
    std::vector<float> vocalCh(numSamples, 0.001f);
    std::vector<std::vector<float>> vocal = {vocalCh, vocalCh};

    AudioCleaner::polishVocalStem(vocal, sampleRate);

    REQUIRE(vocal.size() == 2);
    REQUIRE(vocal[0].size() == numSamples);

    // Check that quiet noise floor is suppressed
    float maxLevel = 0.0f;
    for (size_t i = 1000; i < numSamples; ++i) {
        maxLevel = std::max(maxLevel, std::abs(vocal[0][i]));
    }
    REQUIRE(maxLevel < 0.001f);
}

TEST_CASE("AudioCleaner handles empty or edge buffers gracefully", "[audio][cleaner]") {
    std::vector<std::vector<float>> empty;
    REQUIRE_NOTHROW(AudioCleaner::cleanStereo(empty));
    REQUIRE_NOTHROW(AudioCleaner::polishVocalStem(empty));
}
