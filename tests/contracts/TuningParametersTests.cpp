#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <reggaewave/contracts/TuningParameters.hpp>

using namespace reggaewave::contracts;

TEST_CASE("TuningParameters default values match PRD Section 8.3", "[contracts][tuning]") {
    TuningParameters params;

    REQUIRE(params.getReggaeIntensity() == 70);
    REQUIRE(params.getDubEffectsAmount() == 20);
    REQUIRE_THAT(params.getVocalLevelDb(), Catch::Matchers::WithinAbs(0.0, 1e-6));
}

TEST_CASE("TuningParameters valid bounds acceptance", "[contracts][tuning]") {
    TuningParameters params;

    SECTION("Minimum bounds") {
        params.setReggaeIntensity(0);
        params.setDubEffectsAmount(0);
        params.setVocalLevelDb(-6.0);

        REQUIRE(params.getReggaeIntensity() == 0);
        REQUIRE(params.getDubEffectsAmount() == 0);
        REQUIRE_THAT(params.getVocalLevelDb(), Catch::Matchers::WithinAbs(-6.0, 1e-6));
    }

    SECTION("Maximum bounds") {
        params.setReggaeIntensity(100);
        params.setDubEffectsAmount(100);
        params.setVocalLevelDb(6.0);

        REQUIRE(params.getReggaeIntensity() == 100);
        REQUIRE(params.getDubEffectsAmount() == 100);
        REQUIRE_THAT(params.getVocalLevelDb(), Catch::Matchers::WithinAbs(6.0, 1e-6));
    }
}

TEST_CASE("TuningParameters out-of-range values throw std::out_of_range", "[contracts][tuning]") {
    TuningParameters params;

    REQUIRE_THROWS_AS(params.setReggaeIntensity(-1), std::out_of_range);
    REQUIRE_THROWS_AS(params.setReggaeIntensity(101), std::out_of_range);

    REQUIRE_THROWS_AS(params.setDubEffectsAmount(-1), std::out_of_range);
    REQUIRE_THROWS_AS(params.setDubEffectsAmount(101), std::out_of_range);

    REQUIRE_THROWS_AS(params.setVocalLevelDb(-6.01), std::out_of_range);
    REQUIRE_THROWS_AS(params.setVocalLevelDb(6.01), std::out_of_range);
}
