#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <reggaewave/audio/SubtitleManager.hpp>

using namespace reggaewave::audio;

TEST_CASE("SubtitleManager stores machine and user revised transcripts", "[subtitles]") {
    SubtitleManager manager;
    REQUIRE_FALSE(manager.hasTranscript());

    std::vector<LyricSegment> machine = {
        {1.0, 3.5, "Feel the reggae vibration"},
        {4.0, 7.2, "Roots rock reggae music"}
    };

    manager.setMachineTranscript(machine);
    REQUIRE(manager.hasTranscript());
    REQUIRE(manager.getMachineTranscript().size() == 2);
    REQUIRE(manager.getUserRevisions().size() == 2);

    // Update user revisions
    std::vector<LyricSegment> revised = {
        {1.0, 3.5, "Feel the Reggae vibration"}, // Corrected capitalization
        {4.0, 7.2, "Roots rock Reggae music"}
    };
    manager.setUserRevisions(revised);
    REQUIRE(manager.getUserRevisions()[0].text == "Feel the Reggae vibration");
}

TEST_CASE("SubtitleManager generates valid SRT formatted output", "[subtitles]") {
    SubtitleManager manager;
    std::vector<LyricSegment> segments = {
        {1.5, 4.25, "Positive vibrations in the air"}
    };
    manager.setUserRevisions(segments);

    std::string srt = manager.formatSrt();

    REQUIRE_THAT(srt, Catch::Matchers::ContainsSubstring("1\n"));
    REQUIRE_THAT(srt, Catch::Matchers::ContainsSubstring("00:00:01,500 --> 00:00:04,250\n"));
    REQUIRE_THAT(srt, Catch::Matchers::ContainsSubstring("Positive vibrations in the air\n"));
}

TEST_CASE("SubtitleManager generates valid WebVTT formatted output", "[subtitles]") {
    SubtitleManager manager;
    std::vector<LyricSegment> segments = {
        {65.2, 70.0, "One good thing about music"}
    };
    manager.setUserRevisions(segments);

    std::string vtt = manager.formatVtt();

    REQUIRE_THAT(vtt, Catch::Matchers::StartsWith("WEBVTT\n"));
    REQUIRE_THAT(vtt, Catch::Matchers::ContainsSubstring("00:01:05.200 --> 00:01:10.000\n"));
    REQUIRE_THAT(vtt, Catch::Matchers::ContainsSubstring("One good thing about music\n"));
}

TEST_CASE("SubtitleManager matches active lyric text at timestamp", "[subtitles]") {
    SubtitleManager manager;
    std::vector<LyricSegment> segments = {
        {2.0, 5.0, "Line One"},
        {6.0, 9.0, "Line Two"}
    };
    manager.setUserRevisions(segments);

    REQUIRE(manager.getActiveLyricAtTime(1.0).empty());
    REQUIRE(manager.getActiveLyricAtTime(3.5) == "Line One");
    REQUIRE(manager.getActiveLyricAtTime(5.5).empty());
    REQUIRE(manager.getActiveLyricAtTime(7.0) == "Line Two");
}
