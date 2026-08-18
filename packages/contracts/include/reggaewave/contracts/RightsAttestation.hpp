#pragma once

#include <chrono>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace reggaewave::contracts {

enum class RightsBasis {
    Owned,        // "I own the relevant composition and recording rights."
    Licensed,     // "I have permission or a license to create this version."
    PublicDomain  // "The material is in the public domain, and this recording is authorized for use."
};

inline constexpr std::string_view toString(RightsBasis basis) noexcept {
    switch (basis) {
        case RightsBasis::Owned:        return "OWNED";
        case RightsBasis::Licensed:     return "LICENSED";
        case RightsBasis::PublicDomain: return "PUBLIC_DOMAIN";
    }
    return "UNKNOWN";
}

/**
 * @brief Mandatory rights attestation record for any audio transformation.
 * 
 * Invariants from PRD Section 7.2:
 * - Must explicitly select one valid RightsBasis.
 * - Confirmed flag must be true (never bypassable or pre-selected).
 * - Tracks policy version, timestamp, and project/file ID.
 */
class RightsAttestation {
public:
    static constexpr std::string_view CURRENT_POLICY_VERSION = "2026.1";

    RightsAttestation(RightsBasis basis, bool confirmedByOperator, std::string projectId)
        : basis_(basis)
        , confirmedByOperator_(confirmedByOperator)
        , projectId_(std::move(projectId))
        , policyVersion_(CURRENT_POLICY_VERSION)
        , timestampUtc_(std::chrono::system_clock::now())
    {
        if (!confirmedByOperator_) {
            throw std::invalid_argument("Rights attestation must be explicitly confirmed by the operator");
        }
        if (projectId_.empty()) {
            throw std::invalid_argument("Project ID must not be empty in rights attestation");
        }
    }

    [[nodiscard]] RightsBasis getBasis() const noexcept { return basis_; }
    [[nodiscard]] bool isConfirmed() const noexcept { return confirmedByOperator_; }
    [[nodiscard]] const std::string& getProjectId() const noexcept { return projectId_; }
    [[nodiscard]] const std::string& getPolicyVersion() const noexcept { return policyVersion_; }
    [[nodiscard]] std::chrono::system_clock::time_point getTimestampUtc() const noexcept { return timestampUtc_; }

private:
    RightsBasis basis_;
    bool confirmedByOperator_;
    std::string projectId_;
    std::string policyVersion_;
    std::chrono::system_clock::time_point timestampUtc_;
};

} // namespace reggaewave::contracts
