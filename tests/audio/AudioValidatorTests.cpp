#include <catch2/catch_test_macros.hpp>
#include <reggaewave/audio/AudioValidator.hpp>

using namespace reggaewave::audio;

TEST_CASE("AudioValidator accepts valid audio metadata with rights attestation", "[intake][validator]") {
    // 3-minute stereo 44.1kHz file of size 30 MB with rights confirmed
    uint64_t fileSize = 30ULL * 1024ULL * 1024ULL;
    double duration = 180.0;
    int sampleRate = 44100;
    int channels = 2;
    bool hasRights = true;

    auto res = AudioValidator::validateAudioMetadata(fileSize, duration, sampleRate, channels, hasRights);
    REQUIRE(res.isValid);
    REQUIRE(res.errorCode == ValidationErrorCode::None);
    REQUIRE(res.sanitizedMessage == "Success");
}

TEST_CASE("AudioValidator rejects audio when rights attestation is missing", "[intake][validator]") {
    auto res = AudioValidator::validateAudioMetadata(100000, 60.0, 44100, 2, false);
    REQUIRE_FALSE(res.isValid);
    REQUIRE(res.errorCode == ValidationErrorCode::RightsNotAttested);
}

TEST_CASE("AudioValidator rejects files exceeding 200 MB limit", "[intake][validator]") {
    uint64_t over200Mb = 201ULL * 1024ULL * 1024ULL;
    auto res = AudioValidator::validateAudioMetadata(over200Mb, 60.0, 44100, 2, true);
    REQUIRE_FALSE(res.isValid);
    REQUIRE(res.errorCode == ValidationErrorCode::FileTooLarge);
}

TEST_CASE("AudioValidator rejects files exceeding 10 minute duration limit", "[intake][validator]") {
    double duration601s = 600.1; // > 10 minutes
    auto res = AudioValidator::validateAudioMetadata(5000000, duration601s, 44100, 2, true);
    REQUIRE_FALSE(res.isValid);
    REQUIRE(res.errorCode == ValidationErrorCode::DurationTooLong);
}

TEST_CASE("AudioValidator rejects zero or negative duration", "[intake][validator]") {
    auto res = AudioValidator::validateAudioMetadata(5000000, 0.0, 44100, 2, true);
    REQUIRE_FALSE(res.isValid);
    REQUIRE(res.errorCode == ValidationErrorCode::ZeroDuration);
}

TEST_CASE("AudioValidator rejects invalid channel count", "[intake][validator]") {
    auto res = AudioValidator::validateAudioMetadata(5000000, 60.0, 44100, 0, true);
    REQUIRE_FALSE(res.isValid);
    REQUIRE(res.errorCode == ValidationErrorCode::UnsupportedChannels);
}
