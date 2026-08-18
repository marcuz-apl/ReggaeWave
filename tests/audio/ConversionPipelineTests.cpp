#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <reggaewave/audio/ConversionPipeline.hpp>
#include <reggaewave/fixtures/AudioSynthesizer.hpp>

using namespace reggaewave::audio;
using namespace reggaewave::fixtures;
using namespace reggaewave::contracts;

TEST_CASE("ConversionPipeline executes complete end-to-end transformation", "[pipeline][integration]") {
    // 1. Generate 2 seconds of 440 Hz test song (44.1 kHz, 16-bit stereo)
    auto inputBytes = AudioSynthesizer::generateWav(440.0, 2.0, 44100, 2, 16);

    // 2. Confirmed Rights Attestation
    RightsAttestation rights(RightsBasis::Owned, true, "proj-end-to-end-01");
    TuningParameters tuning(70, 25, 1.0);

    ConversionPipeline pipeline;
    REQUIRE_FALSE(pipeline.isReady());

    // 3. Execute Pipeline
    auto output = pipeline.execute(inputBytes, rights, tuning, "proj-end-to-end-01", "Roots Elevation");

    REQUIRE(pipeline.isReady());
    REQUIRE(output.totalSamples == 88200); // 2.0s * 44100
    REQUIRE_THAT(output.durationSeconds, Catch::Matchers::WithinAbs(2.0, 0.01));
    REQUIRE(output.projectManifest.projectId == "proj-end-to-end-01");
    REQUIRE(output.projectManifest.reggaeIntensity == 70);
    REQUIRE(output.manifestVariationA.rhythmStyle == "ONE_DROP");
    REQUIRE(output.manifestVariationB.rhythmStyle == "STEPPERS");
    REQUIRE(output.waveformOverviewPeaks.size() == 160);

    // 4. Test live real-time block processing
    std::vector<float> leftOut(512, 0.0f);
    std::vector<float> rightOut(512, 0.0f);
    std::vector<float*> outChannels = {leftOut.data(), rightOut.data()};

    pipeline.processBlock(outChannels.data(), 2, 512);

    float maxSample = 0.0f;
    for (float s : leftOut) {
        maxSample = std::max(maxSample, std::abs(s));
    }
    REQUIRE(maxSample > 0.05f);

    // 5. Test Live Dynamic Tuning Update
    TuningParameters newTuning(85, 45, -2.0);
    pipeline.updateTuning(newTuning);
    REQUIRE(pipeline.getDubProcessor().getDubAmount() == 45);

    // 6. Test Variation Switching
    pipeline.setActiveVariation(ActiveVariation::VariationB);
    REQUIRE(pipeline.getTransport().getActiveVariation() == ActiveVariation::VariationB);
}

TEST_CASE("ConversionPipeline throws on unconfirmed rights attestation", "[pipeline][integration]") {
    auto inputBytes = AudioSynthesizer::generateWav(440.0, 1.0, 44100, 2, 16);
    TuningParameters tuning;
    ConversionPipeline pipeline;

    // RightsAttestation constructor throws if confirmed is false
    REQUIRE_THROWS_AS(RightsAttestation(RightsBasis::Owned, false, "proj-bad"), std::invalid_argument);
}
