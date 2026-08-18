#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <reggaewave/audio/AudioExporter.hpp>

using namespace reggaewave::audio;

TEST_CASE("AudioExporter generates valid 24-bit stereo PCM WAV bytes", "[export]") {
    const size_t numSamples = 22050; // 0.5s at 44.1kHz
    std::vector<std::vector<float>> audio = {
        std::vector<float>(numSamples, 0.5f),
        std::vector<float>(numSamples, -0.5f)
    };

    auto wavBytes = AudioExporter::encodeWav24Bit(audio, 44100.0);

    // 44-byte header + (22050 samples * 2 channels * 3 bytes/sample) = 44 + 132300 = 132344
    REQUIRE(wavBytes.size() == 132344);

    // Verify RIFF and WAVE magic
    REQUIRE(std::memcmp(&wavBytes[0], "RIFF", 4) == 0);
    REQUIRE(std::memcmp(&wavBytes[8], "WAVE", 4) == 0);
}

TEST_CASE("AudioExporter generates 320 kbps MP3 format container", "[export]") {
    const size_t numSamples = 1000;
    std::vector<std::vector<float>> audio = {
        std::vector<float>(numSamples, 0.3f),
        std::vector<float>(numSamples, 0.3f)
    };

    auto mp3Bytes = AudioExporter::encodeMp3(audio, 44100.0);
    REQUIRE(mp3Bytes.size() > 1000);
}
