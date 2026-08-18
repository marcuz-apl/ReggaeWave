#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <reggaewave/audio/StemSeparator.hpp>
#include <vector>

using namespace reggaewave::audio;

TEST_CASE("StemSeparator isolates vocal and accompaniment stems with identical length", "[analysis][separation]") {
    StemSeparator separator(ExecutionProvider::CPU);
    REQUIRE(separator.getProvider() == ExecutionProvider::CPU);

    separator.setProvider(ExecutionProvider::DirectML);
    REQUIRE(separator.getProvider() == ExecutionProvider::DirectML);

    const size_t numSamples = 44100; // 1 second
    std::vector<std::vector<float>> input = {
        std::vector<float>(numSamples, 0.5f),
        std::vector<float>(numSamples, 0.5f)
    };

    auto result = separator.separate(input);

    REQUIRE(result.leadVocal.size() == 2);
    REQUIRE(result.accompaniment.size() == 2);
    REQUIRE(result.leadVocal[0].size() == numSamples);
    REQUIRE(result.accompaniment[0].size() == numSamples);
    REQUIRE(result.separationConfidence >= 0.5);
    REQUIRE(result.modelVersion == "demucs_v4_onnx");
}

TEST_CASE("StemSeparator throws on empty or single channel input", "[analysis][separation]") {
    StemSeparator separator;
    std::vector<std::vector<float>> emptyBuffer;
    REQUIRE_THROWS_AS(separator.separate(emptyBuffer), std::invalid_argument);

    std::vector<std::vector<float>> singleChannel = {std::vector<float>(100, 0.0f)};
    REQUIRE_THROWS_AS(separator.separate(singleChannel), std::invalid_argument);
}
