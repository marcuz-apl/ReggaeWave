#pragma once

#include <reggaewave/contracts/JobState.hpp>
#include <reggaewave/contracts/Manifests.hpp>
#include <reggaewave/contracts/RightsAttestation.hpp>
#include <reggaewave/contracts/TuningParameters.hpp>

#include <string>
#include <vector>
#include <optional>
#include <mutex>
#include <unordered_map>
#include <stdexcept>

namespace reggaewave::storage {

/**
 * @brief Local transaction-safe storage for ReggaeWave projects and jobs.
 */
class LocalDatabase {
public:
    LocalDatabase() = default;

    void saveProject(const contracts::ProjectManifest& project) {
        std::lock_guard<std::mutex> lock(mutex_);
        projects_[project.projectId] = project;
    }

    [[nodiscard]] std::optional<contracts::ProjectManifest> getProject(const std::string& projectId) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = projects_.find(projectId);
        if (it != projects_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    void updateConversionState(const std::string& projectId, contracts::ConversionJobState newState, bool subtitlesEnabled = false) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto current = conversionStates_.find(projectId);
        contracts::ConversionJobState currentState = (current != conversionStates_.end()) 
            ? current->second 
            : contracts::ConversionJobState::Created;

        if (!contracts::isValidConversionTransition(currentState, newState, subtitlesEnabled)) {
            throw std::runtime_error(std::string("Invalid conversion state transition from ") + 
                                     std::string(contracts::toString(currentState)) + " to " + 
                                     std::string(contracts::toString(newState)));
        }

        conversionStates_[projectId] = newState;
    }

    [[nodiscard]] contracts::ConversionJobState getConversionState(const std::string& projectId) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = conversionStates_.find(projectId);
        return (it != conversionStates_.end()) ? it->second : contracts::ConversionJobState::Created;
    }

    void deleteProject(const std::string& projectId) {
        std::lock_guard<std::mutex> lock(mutex_);
        projects_.erase(projectId);
        conversionStates_[projectId] = contracts::ConversionJobState::Deleted;
    }

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, contracts::ProjectManifest> projects_;
    std::unordered_map<std::string, contracts::ConversionJobState> conversionStates_;
};

} // namespace reggaewave::storage
