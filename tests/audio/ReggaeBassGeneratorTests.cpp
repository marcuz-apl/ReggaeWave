#include <catch2/catch_test_macros.hpp>
#include <reggaewave/audio/ReggaeBassGenerator.hpp>
#include <algorithm>
#include <cmath>

using namespace reggaewave::audio;

static float getWindowPeak(const std::vector<float>& channel, size_t startPos, size_t windowLen = 2000) {
    float peak = 0.0f;
    for (size_t i = 0; i < windowLen && startPos + i < channel.size(); ++i) {
        peak = std::max(peak, std::abs(channel[startPos + i]));
    }
    return peak;
}

TEST_CASE("ReggaeBassGenerator generates melodic sub-bass aligned to chords", "[arrangement][bass]") {
    BeatGrid grid;
    grid.bpm = 120.0;
    grid.beatIntervalSamples = 22050;
    for (size_t i = 0; i < 8; ++i) {
        grid.beatPositions.push_back(i * grid.beatIntervalSamples);
    }
    const size_t totalSamples = 8 * grid.beatIntervalSamples;

    std::vector<ChordEvent> chords = {
        {0, 4 * grid.beatIntervalSamples, "C"},
        {4 * grid.beatIntervalSamples, 8 * grid.beatIntervalSamples, "G"}
    };

    auto bass = ReggaeBassGenerator::synthesize(totalSamples, grid, chords, 70);

    REQUIRE(bass.size() == 2);
    REQUIRE(bass[0].size() == totalSamples);

    // Verify bass notes have strong energy on beat 2 and 3 of bar 1
    float beat2Peak = getWindowPeak(bass[0], grid.beatIntervalSamples, 2000);
    float beat3Peak = getWindowPeak(bass[0], grid.beatIntervalSamples * 2, 2000);

    REQUIRE(beat2Peak > 0.1f);
    REQUIRE(beat3Peak > 0.1f);
}
