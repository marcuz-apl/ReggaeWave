#include <catch2/catch_test_macros.hpp>
#include <reggaewave/audio/ReggaeSkankGenerator.hpp>
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

TEST_CASE("ReggaeSkankGenerator generates staccato offbeat chops", "[arrangement][skank]") {
    BeatGrid grid;
    grid.bpm = 120.0;
    grid.beatIntervalSamples = 22050;
    for (size_t i = 0; i < 4; ++i) {
        grid.beatPositions.push_back(i * grid.beatIntervalSamples);
    }
    const size_t totalSamples = 4 * grid.beatIntervalSamples;

    std::vector<ChordEvent> chords = {
        {0, totalSamples, "Am"}
    };

    auto skank = ReggaeSkankGenerator::synthesize(totalSamples, grid, chords, true, 70);

    REQUIRE(skank.size() == 2);
    REQUIRE(skank[0].size() == totalSamples);

    // Verify offbeat "&" has clear chop energy
    size_t offbeatPos = grid.beatIntervalSamples / 2;
    float offbeatPeak = getWindowPeak(skank[0], offbeatPos, 2000);
    REQUIRE(offbeatPeak > 0.1f);
}
