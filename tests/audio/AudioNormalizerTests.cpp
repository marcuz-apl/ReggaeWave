#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <reggaewave/audio/AudioNormalizer.hpp>

using namespace reggaewave::audio;

TEST_CASE("AudioNormalizer converts mono input into canonical stereo", "[intake][normalizer]") {
    // 1 second of mono audio at 44.1 kHz
    std::vector<float> monoChannel(44100, 0.42f);
    std::vector<std::vector<float>> input = {monoChannel};

    auto normalized = AudioNormalizer::normalize(input, 44100.0);

    REQUIRE(normalized.sampleRate == 44100.0);
    REQUIRE(normalized.channels.size() == 2);
    REQUIRE(normalized.numSamples == 44100);
    REQUIRE_THAT(normalized.durationSeconds, Catch::Matchers::WithinAbs(1.0, 0.001));

    // Both channels have identical duplicated content
    REQUIRE_THAT(normalized.channels[0][100], Catch::Matchers::WithinAbs(0.42, 1e-6));
    REQUIRE_THAT(normalized.channels[1][100], Catch::Matchers::WithinAbs(0.42, 1e-6));
}

TEST_CASE("AudioNormalizer resamples 48 kHz stereo to canonical 44.1 kHz", "[intake][normalizer]") {
    // 1 second of stereo audio at 48000 Hz
    std::vector<std::vector<float>> input48k = {
        std::vector<float>(48000, 0.5f),
        std::vector<float>(48000, -0.5f)
    };

    auto normalized = AudioNormalizer::normalize(input48k, 48000.0);

    REQUIRE(normalized.sampleRate == 44100.0);
    REQUIRE(normalized.channels.size() == 2);
    REQUIRE(normalized.numSamples == 44100);
    REQUIRE_THAT(normalized.durationSeconds, Catch::Matchers::WithinAbs(1.0, 0.01));

    REQUIRE_THAT(normalized.channels[0][1000], Catch::Matchers::WithinAbs(0.5, 0.01));
    REQUIRE_THAT(normalized.channels[1][1000], Catch::Matchers::WithinAbs(-0.5, 0.01));
}

TEST_CASE("AudioNormalizer downmixes 5.1 multichannel audio to stereo", "[intake][normalizer]") {
    // 5.1 channel input: L, R, C, LFE, Ls, Rs
    const size_t numSamples = 1000;
    std::vector<std::vector<float>> input51(6, std::vector<float>(numSamples, 0.1f));

    auto normalized = AudioNormalizer::normalize(input51, 44100.0);

    REQUIRE(normalized.channels.size() == 2);
    REQUIRE(normalized.numSamples == numSamples);

    // Verify downmixed gain is within valid range [-1.0, 1.0]
    REQUIRE(normalized.channels[0][0] > 0.1f);
    REQUIRE(normalized.channels[0][0] <= 1.0f);
}
