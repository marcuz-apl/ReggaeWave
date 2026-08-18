#include <catch2/catch_test_macros.hpp>
#include <reggaewave/storage/RetentionManager.hpp>

using namespace reggaewave::storage;
using namespace reggaewave::contracts;

TEST_CASE("RetentionManager manages stem lifecycles and purge window", "[storage][retention]") {
    LocalDatabase db;
    RetentionManager retention(db);

    const std::string projId = "proj-retention-01";
    retention.registerIntermediateStem(projId, "/tmp/vocal_stem.wav");
    retention.registerIntermediateStem(projId, "/tmp/acc_stem.wav");
    retention.registerFinalExport(projId, "/exports/reggae_master.wav");

    REQUIRE(retention.getActiveArtifactCount(projId) == 3);

    // After 25 hours, intermediate stems should purge, final export remains
    auto now = std::chrono::system_clock::now();
    auto after25Hours = now + std::chrono::hours(25);
    size_t purged = retention.purgeExpiredArtifacts(after25Hours);

    REQUIRE(purged == 2);
    REQUIRE(retention.getActiveArtifactCount(projId) == 1);
}

TEST_CASE("RetentionManager immediate project deletion cleans all artifacts", "[storage][retention]") {
    LocalDatabase db;
    RetentionManager retention(db);

    const std::string projId = "proj-delete-immediate";
    retention.registerIntermediateStem(projId, "/tmp/stem1.wav");
    retention.registerFinalExport(projId, "/exports/master.wav");

    REQUIRE(retention.getActiveArtifactCount(projId) == 2);

    retention.deleteProjectImmediately(projId);

    REQUIRE(retention.getActiveArtifactCount(projId) == 0);
    REQUIRE(db.getConversionState(projId) == ConversionJobState::Deleted);
}
