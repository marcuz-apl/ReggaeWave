#include <catch2/catch_test_macros.hpp>
#include <reggaewave/contracts/RightsAttestation.hpp>

using namespace reggaewave::contracts;

TEST_CASE("RightsAttestation valid instantiation", "[contracts][rights]") {
    SECTION("Owned rights basis") {
        RightsAttestation attestation(RightsBasis::Owned, true, "proj-001");
        REQUIRE(attestation.getBasis() == RightsBasis::Owned);
        REQUIRE(attestation.isConfirmed());
        REQUIRE(attestation.getProjectId() == "proj-001");
        REQUIRE(attestation.getPolicyVersion() == "2026.1");
    }

    SECTION("Licensed basis") {
        RightsAttestation attestation(RightsBasis::Licensed, true, "proj-002");
        REQUIRE(attestation.getBasis() == RightsBasis::Licensed);
        REQUIRE(attestation.isConfirmed());
    }

    SECTION("Public domain basis") {
        RightsAttestation attestation(RightsBasis::PublicDomain, true, "proj-003");
        REQUIRE(attestation.getBasis() == RightsBasis::PublicDomain);
        REQUIRE(attestation.isConfirmed());
    }
}

TEST_CASE("RightsAttestation rejects unconfirmed declaration", "[contracts][rights]") {
    REQUIRE_THROWS_AS(RightsAttestation(RightsBasis::Owned, false, "proj-001"), std::invalid_argument);
    REQUIRE_THROWS_AS(RightsAttestation(RightsBasis::Licensed, false, "proj-002"), std::invalid_argument);
    REQUIRE_THROWS_AS(RightsAttestation(RightsBasis::PublicDomain, false, "proj-003"), std::invalid_argument);
}

TEST_CASE("RightsAttestation rejects empty project identifier", "[contracts][rights]") {
    REQUIRE_THROWS_AS(RightsAttestation(RightsBasis::Owned, true, ""), std::invalid_argument);
}
