#include <catch2/catch_test_macros.hpp>
#include <reggaewave/contracts/JobState.hpp>

using namespace reggaewave::contracts;

TEST_CASE("ConversionJobState happy path without subtitles", "[contracts][job_state]") {
    REQUIRE(isValidConversionTransition(ConversionJobState::Created, ConversionJobState::Importing));
    REQUIRE(isValidConversionTransition(ConversionJobState::Importing, ConversionJobState::Validating));
    REQUIRE(isValidConversionTransition(ConversionJobState::Validating, ConversionJobState::Queued));
    REQUIRE(isValidConversionTransition(ConversionJobState::Queued, ConversionJobState::Normalizing));
    REQUIRE(isValidConversionTransition(ConversionJobState::Normalizing, ConversionJobState::Separating));
    REQUIRE(isValidConversionTransition(ConversionJobState::Separating, ConversionJobState::Analyzing));
    REQUIRE(isValidConversionTransition(ConversionJobState::Analyzing, ConversionJobState::Arranging));
    REQUIRE(isValidConversionTransition(ConversionJobState::Arranging, ConversionJobState::Mixing));
    // Subtitles disabled skips Transcribing
    REQUIRE(isValidConversionTransition(ConversionJobState::Mixing, ConversionJobState::PreviewReady, false));
    REQUIRE(isValidConversionTransition(ConversionJobState::PreviewReady, ConversionJobState::Completed));
}

TEST_CASE("ConversionJobState happy path with subtitles enabled", "[contracts][job_state]") {
    REQUIRE(isValidConversionTransition(ConversionJobState::Mixing, ConversionJobState::Transcribing, true));
    REQUIRE(isValidConversionTransition(ConversionJobState::Transcribing, ConversionJobState::PreviewReady, true));
}

TEST_CASE("ConversionJobState cancellation and failure transitions", "[contracts][job_state]") {
    REQUIRE(isValidConversionTransition(ConversionJobState::Arranging, ConversionJobState::Failed));
    REQUIRE(isValidConversionTransition(ConversionJobState::Separating, ConversionJobState::Cancelled));
    REQUIRE(isValidConversionTransition(ConversionJobState::PreviewReady, ConversionJobState::Deleted));
}

TEST_CASE("ConversionJobState invalid skipping transitions", "[contracts][job_state]") {
    REQUIRE_FALSE(isValidConversionTransition(ConversionJobState::Created, ConversionJobState::Separating));
    REQUIRE_FALSE(isValidConversionTransition(ConversionJobState::Importing, ConversionJobState::Completed));
    REQUIRE_FALSE(isValidConversionTransition(ConversionJobState::Completed, ConversionJobState::Arranging));
}

TEST_CASE("ExportJobState transition validation", "[contracts][job_state]") {
    REQUIRE(isValidExportTransition(ExportJobState::Created, ExportJobState::Queued));
    REQUIRE(isValidExportTransition(ExportJobState::Queued, ExportJobState::Rendering));
    REQUIRE(isValidExportTransition(ExportJobState::Rendering, ExportJobState::Validating));
    REQUIRE(isValidExportTransition(ExportJobState::Validating, ExportJobState::Completed));
    REQUIRE(isValidExportTransition(ExportJobState::Rendering, ExportJobState::Failed));
    REQUIRE(isValidExportTransition(ExportJobState::Queued, ExportJobState::Cancelled));
}
