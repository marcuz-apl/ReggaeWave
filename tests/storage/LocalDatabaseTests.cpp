#include <catch2/catch_test_macros.hpp>
#include <reggaewave/storage/LocalDatabase.hpp>

using namespace reggaewave::storage;
using namespace reggaewave::contracts;

TEST_CASE("LocalDatabase project saving and retrieval", "[storage]") {
    LocalDatabase db;

    ProjectManifest proj;
    proj.projectId = "reggae-proj-42";
    proj.projectName = "Roots Anthem";
    proj.reggaeIntensity = 80;
    proj.dubEffectsAmount = 35;
    proj.vocalLevelDb = 1.5;

    db.saveProject(proj);

    auto retrieved = db.getProject("reggae-proj-42");
    REQUIRE(retrieved.has_value());
    REQUIRE(retrieved->projectName == "Roots Anthem");
    REQUIRE(retrieved->reggaeIntensity == 80);
    REQUIRE(retrieved->dubEffectsAmount == 35);
}

TEST_CASE("LocalDatabase state machine transitions", "[storage]") {
    LocalDatabase db;
    const std::string projId = "proj-state-test";

    REQUIRE(db.getConversionState(projId) == ConversionJobState::Created);

    db.updateConversionState(projId, ConversionJobState::Importing);
    REQUIRE(db.getConversionState(projId) == ConversionJobState::Importing);

    db.updateConversionState(projId, ConversionJobState::Validating);
    REQUIRE(db.getConversionState(projId) == ConversionJobState::Validating);

    // Invalid transition throws
    REQUIRE_THROWS_AS(db.updateConversionState(projId, ConversionJobState::Completed), std::runtime_error);
}
