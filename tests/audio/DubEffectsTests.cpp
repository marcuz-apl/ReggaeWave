#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <reggaewave/audio/DubEffectsProcessor.hpp>
#include <vector>

using namespace reggaewave::audio;

TEST_CASE("DubEffectsProcessor bypass when dub amount is zero", "[audio][dub]") {
    DubEffectsProcessor dubFx;
    dubFx.prepare(44100.0, 512, 2);
    dubFx.setDubAmount(0);

    std::vector<float> left = {1.0f, 0.5f, -0.5f, -1.0f};
    std::vector<float> right = {0.8f, 0.4f, -0.4f, -0.8f};
    std::vector<float*> channels = {left.data(), right.data()};

    dubFx.process(channels.data(), 2, 4);

    REQUIRE_THAT(left[0], Catch::Matchers::WithinAbs(1.0, 1e-6));
    REQUIRE_THAT(left[1], Catch::Matchers::WithinAbs(0.5, 1e-6));
    REQUIRE_THAT(right[0], Catch::Matchers::WithinAbs(0.8, 1e-6));
}

TEST_CASE("DubEffectsProcessor generates rhythmic echoes when dub amount is active", "[audio][dub]") {
    DubEffectsProcessor dubFx;
    const double sampleRate = 44100.0;
    dubFx.prepare(sampleRate, 512, 1);
    dubFx.setTempoBpm(120.0);
    dubFx.setDubAmount(50);

    // Impulse input at sample 0
    const int totalSamples = static_cast<int>(sampleRate * 1.0); // 1 second
    std::vector<float> buffer(totalSamples, 0.0f);
    buffer[0] = 1.0f; // Single impulse

    float* channelPtr = buffer.data();
    dubFx.process(&channelPtr, 1, totalSamples);

    // Dotted 8th delay at 120 BPM: (60 / 120) * 0.75 = 0.375s => sample 16537
    const size_t expectedEchoSample = static_cast<size_t>(0.375 * sampleRate);
    
    // There should be a distinct non-zero wet echo around the expected delay time
    float maxEcho = 0.0f;
    for (size_t i = expectedEchoSample - 5; i <= expectedEchoSample + 5; ++i) {
        if (i < buffer.size()) {
            maxEcho = std::max(maxEcho, std::abs(buffer[i]));
        }
    }

    REQUIRE(maxEcho > 0.05f);
}

TEST_CASE("DubEffectsProcessor reset clears delay buffer", "[audio][dub]") {
    DubEffectsProcessor dubFx;
    dubFx.prepare(44100.0, 512, 1);
    dubFx.setDubAmount(50);

    std::vector<float> buffer(1024, 0.5f);
    float* ptr = buffer.data();
    dubFx.process(&ptr, 1, 1024);

    dubFx.reset();

    // Process silent buffer after reset; should remain silent
    std::vector<float> silence(1024, 0.0f);
    float* silencePtr = silence.data();
    dubFx.process(&silencePtr, 1, 1024);

    float maxVal = 0.0f;
    for (float s : silence) {
        maxVal = std::max(maxVal, std::abs(s));
    }
    REQUIRE_THAT(maxVal, Catch::Matchers::WithinAbs(0.0, 1e-6));
}
