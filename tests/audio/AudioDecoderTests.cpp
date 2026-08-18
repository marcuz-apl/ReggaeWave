#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <reggaewave/audio/AudioDecoder.hpp>
#include <reggaewave/fixtures/AudioSynthesizer.hpp>

using namespace reggaewave::audio;
using namespace reggaewave::fixtures;

TEST_CASE("AudioDecoder successfully parses synthetic 16-bit stereo WAV buffer", "[intake][decoder]") {
    // 0.5 seconds of 440 Hz tone at 44.1 kHz stereo
    auto wavBytes = AudioSynthesizer::generateWav(440.0, 0.5, 44100, 2, 16);

    auto decoded = AudioDecoder::decodeWavBytes(wavBytes.data(), wavBytes.size());

    REQUIRE(decoded.numChannels == 2);
    REQUIRE(decoded.sampleRate == 44100.0);
    REQUIRE(decoded.numSamples == 22050);
    REQUIRE_THAT(decoded.durationSeconds, Catch::Matchers::WithinAbs(0.5, 0.001));
    REQUIRE(decoded.channels.size() == 2);
    REQUIRE(decoded.channels[0].size() == 22050);

    // Verify non-zero sample amplitudes
    float maxSample = 0.0f;
    for (float s : decoded.channels[0]) {
        maxSample = std::max(maxSample, std::abs(s));
    }
    REQUIRE(maxSample > 0.5f);
}

TEST_CASE("AudioDecoder successfully parses synthetic 24-bit stereo WAV buffer", "[intake][decoder]") {
    // 0.25 seconds of 1000 Hz tone at 48 kHz stereo 24-bit
    auto wavBytes = AudioSynthesizer::generateWav(1000.0, 0.25, 48000, 2, 24);

    auto decoded = AudioDecoder::decodeWavBytes(wavBytes.data(), wavBytes.size());

    REQUIRE(decoded.numChannels == 2);
    REQUIRE(decoded.sampleRate == 48000.0);
    REQUIRE(decoded.numSamples == 12000);
    REQUIRE_THAT(decoded.durationSeconds, Catch::Matchers::WithinAbs(0.25, 0.001));
}

TEST_CASE("AudioDecoder throws on corrupted or truncated byte buffer", "[intake][decoder]") {
    std::vector<uint8_t> badBytes = {0x00, 0x01, 0x02, 0x03};
    REQUIRE_THROWS_AS(AudioDecoder::decodeWavBytes(badBytes.data(), badBytes.size()), std::runtime_error);
}
