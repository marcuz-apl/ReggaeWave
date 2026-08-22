#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <reggaewave/audio/DualTransportSource.hpp>
#include <vector>

using namespace reggaewave::audio;

TEST_CASE("DualTransportSource synchronized playback and 3-way variation switching", "[audio][transport]") {
    DualTransportSource transport;
    const double sampleRate = 44100.0;
    transport.prepare(sampleRate, 2);

    const int numSamples = 1000;
    std::vector<std::vector<float>> orig = {std::vector<float>(numSamples, 0.3f), std::vector<float>(numSamples, 0.3f)};
    std::vector<std::vector<float>> varA = {std::vector<float>(numSamples, 0.4f), std::vector<float>(numSamples, 0.4f)};
    std::vector<std::vector<float>> varB = {std::vector<float>(numSamples, 0.8f), std::vector<float>(numSamples, 0.8f)};
    std::vector<std::vector<float>> vocal = {std::vector<float>(numSamples, 0.2f), std::vector<float>(numSamples, 0.2f)};

    transport.loadOriginal(orig);
    transport.loadVariationA(varA);
    transport.loadVariationB(varB);
    transport.loadLeadVocal(vocal);

    REQUIRE(transport.getTotalLengthSamples() == numSamples);
    REQUIRE(transport.getPlayheadSample() == 0);

    std::vector<float> outLeft(100, 0.0f);
    std::vector<float> outRight(100, 0.0f);
    std::vector<float*> outChannels = {outLeft.data(), outRight.data()};

    // Render Variation A (0.4) + Vocal (0.2) = 0.6
    transport.renderNextBlock(outChannels.data(), 2, 100);
    REQUIRE(transport.getPlayheadSample() == 100);
    REQUIRE_THAT(outLeft[0], Catch::Matchers::WithinAbs(0.6, 1e-5));

    // Switch to Variation B
    transport.setActiveVariation(ActiveVariation::VariationB);
    transport.renderNextBlock(outChannels.data(), 2, 100);
    REQUIRE(transport.getPlayheadSample() == 200);

    // Switch to Pure Original Source
    transport.setActiveVariation(ActiveVariation::Original);
    transport.renderNextBlock(outChannels.data(), 2, 100);
    REQUIRE(transport.getPlayheadSample() == 300);

    // Set playhead position
    transport.setPlayheadSample(500);
    REQUIRE(transport.getPlayheadSample() == 500);
    REQUIRE_THAT(transport.getPlayheadSeconds(), Catch::Matchers::WithinAbs(500.0 / sampleRate, 1e-6));
}

TEST_CASE("DualTransportSource vocal gain scaling", "[audio][transport]") {
    DualTransportSource transport;
    transport.prepare(44100.0, 1);

    std::vector<std::vector<float>> varA = {std::vector<float>(100, 0.0f)};
    std::vector<std::vector<float>> vocal = {std::vector<float>(100, 0.5f)};
    transport.loadVariationA(varA);
    transport.loadLeadVocal(vocal);

    // +6 dB boost => ~2x gain (0.5 * 1.995 = ~0.9976)
    transport.setVocalGainDb(6.0);

    std::vector<float> out(100, 0.0f);
    float* ptr = out.data();
    transport.renderNextBlock(&ptr, 1, 10);

    REQUIRE_THAT(out[0], Catch::Matchers::WithinAbs(0.9976, 0.01));
}
