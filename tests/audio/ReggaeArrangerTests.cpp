#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <reggaewave/audio/ReggaeArranger.hpp>

using namespace reggaewave::audio;

TEST_CASE("ReggaeArranger generates duration-aligned Variation A and Variation B", "[arrangement][orchestrator]") {
    AnalysisReport report;
    report.beatGrid.bpm = 120.0;
    report.beatGrid.beatIntervalSamples = 22050;
    for (size_t i = 0; i < 8; ++i) {
        report.beatGrid.beatPositions.push_back(i * report.beatGrid.beatIntervalSamples);
    }
    report.chordTimeline = {
        {0, 4 * report.beatGrid.beatIntervalSamples, "C"},
        {4 * report.beatGrid.beatIntervalSamples, 8 * report.beatGrid.beatIntervalSamples, "G"}
    };

    const size_t totalSamples = 8 * report.beatGrid.beatIntervalSamples;
    auto arrangement = ReggaeArranger::arrange(totalSamples, report, 75);

    // 1. Both variations exist and have exact sample count match
    REQUIRE(arrangement.variationA.size() == 2);
    REQUIRE(arrangement.variationB.size() == 2);
    REQUIRE(arrangement.variationA[0].size() == totalSamples);
    REQUIRE(arrangement.variationB[0].size() == totalSamples);

    // 2. Manifest metadata
    REQUIRE(arrangement.manifestA.variationId == "variation_a");
    REQUIRE(arrangement.manifestA.rhythmStyle == "ONE_DROP");
    REQUIRE(arrangement.manifestB.variationId == "variation_b");
    REQUIRE(arrangement.manifestB.rhythmStyle == "STEPPERS");

    // 3. Audio signals have non-zero content
    float maxA = 0.0f;
    float maxB = 0.0f;
    for (size_t i = 0; i < totalSamples; ++i) {
        maxA = std::max(maxA, std::abs(arrangement.variationA[0][i]));
        maxB = std::max(maxB, std::abs(arrangement.variationB[0][i]));
    }

    REQUIRE(maxA > 0.2f);
    REQUIRE(maxB > 0.2f);
}
