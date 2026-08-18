#include <catch2/catch_test_macros.hpp>
#include <reggaewave/audio/ReggaeDrumSynthesizer.hpp>
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

TEST_CASE("ReggaeDrumSynthesizer synthesizes One-Drop and Steppers drum patterns", "[arrangement][drums]") {
    BeatGrid grid;
    grid.bpm = 120.0;
    grid.beatIntervalSamples = 22050; // 0.5s
    for (size_t i = 0; i < 16; ++i) { // 4 bars = 16 beats
        grid.beatPositions.push_back(i * grid.beatIntervalSamples);
    }
    const size_t totalSamples = 16 * grid.beatIntervalSamples;

    SECTION("One-Drop pattern has energy focused on beat 3") {
        auto drums = ReggaeDrumSynthesizer::synthesize(totalSamples, grid, ReggaeDrumStyle::OneDrop, 70);
        REQUIRE(drums.size() == 2);
        REQUIRE(drums[0].size() == totalSamples);

        // Beat 1 has only closed hi-hat, Beat 3 has full Kick + Snare burst
        float beat1Peak = getWindowPeak(drums[0], 0, 2000);
        float beat3Peak = getWindowPeak(drums[0], grid.beatIntervalSamples * 2, 2000);

        REQUIRE(beat3Peak > beat1Peak * 1.5f);
        REQUIRE(beat3Peak > 0.4f);
    }

    SECTION("Steppers pattern has kicks on all 4 beats") {
        auto drums = ReggaeDrumSynthesizer::synthesize(totalSamples, grid, ReggaeDrumStyle::Steppers, 80);
        REQUIRE(drums.size() == 2);
        REQUIRE(drums[0].size() == totalSamples);

        // Check non-zero kick peak on beats 1, 2, 3, 4
        for (int b = 0; b < 4; ++b) {
            float kickPeak = getWindowPeak(drums[0], grid.beatIntervalSamples * b, 2000);
            REQUIRE(kickPeak > 0.4f);
        }
    }
}
