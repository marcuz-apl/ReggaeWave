#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <reggaewave/audio/MusicAnalyzer.hpp>
#include <reggaewave/fixtures/AudioSynthesizer.hpp>

using namespace reggaewave::audio;
using namespace reggaewave::fixtures;

TEST_CASE("MusicAnalyzer extracts BPM, key, chords, and sections from audio", "[analysis][harmony]") {
    // 4 seconds of 440 Hz tone at 44.1 kHz stereo
    const size_t numSamples = 44100 * 4;
    std::vector<float> sine(numSamples);
    for (size_t i = 0; i < numSamples; ++i) {
        // Amplitude modulated tone at ~2 Hz to simulate rhythm pulses (~120 BPM)
        double mod = 0.5 + 0.5 * std::cos(2.0 * 3.1415926535 * 2.0 * i / 44100.0);
        sine[i] = static_cast<float>(std::sin(2.0 * 3.1415926535 * 440.0 * i / 44100.0) * mod);
    }
    std::vector<std::vector<float>> input = {sine, sine};

    auto report = MusicAnalyzer::analyze(input);

    // 1. BPM verification
    REQUIRE(report.manifest.bpm >= 60.0);
    REQUIRE(report.manifest.bpm <= 180.0);
    REQUIRE(report.beatGrid.beatPositions.size() > 0);
    REQUIRE(report.beatGrid.downbeatPositions.size() > 0);

    // 2. Key verification
    REQUIRE_FALSE(report.manifest.key.empty());
    REQUIRE(report.keyResult.confidence > 0.5);

    // 3. Chords & Sections
    REQUIRE_FALSE(report.manifest.detectedChords.empty());
    REQUIRE_FALSE(report.sections.empty());
    REQUIRE((report.sections[0].name == "Intro" || report.sections[0].name == "Verse"));

    // 4. Manifest score
    REQUIRE(report.manifest.confidenceScore >= 0.5);
    REQUIRE(report.manifest.confidenceScore <= 1.0);
}

TEST_CASE("MusicAnalyzer throws on empty input buffer", "[analysis][harmony]") {
    std::vector<std::vector<float>> empty;
    REQUIRE_THROWS_AS(MusicAnalyzer::analyze(empty), std::invalid_argument);
}
