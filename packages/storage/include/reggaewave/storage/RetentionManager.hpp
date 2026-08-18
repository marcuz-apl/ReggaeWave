#pragma once

#include <reggaewave/storage/LocalDatabase.hpp>
#include <string>
#include <vector>
#include <chrono>
#include <mutex>
#include <unordered_set>

namespace reggaewave::storage {

struct StoredArtifact {
    std::string artifactId;
    std::string projectId;
    std::string filePath;
    bool isIntermediateStem = true; // Stems vs Final Export
    std::chrono::system_clock::time_point createdAtUtc;
    std::chrono::system_clock::time_point expiresAtUtc;
};

/**
 * @brief Enforces privacy and retention lifecycles for local audio files and projects.
 */
class RetentionManager {
public:
    static constexpr int INTERMEDIATE_STEM_RETENTION_HOURS = 24;
    static constexpr int FINAL_EXPORT_RETENTION_DAYS = 30;

    RetentionManager(LocalDatabase& database) : database_(database) {}

    void registerIntermediateStem(const std::string& projectId, const std::string& filePath) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::system_clock::now();
        StoredArtifact artifact{
            .artifactId = projectId + "_stem_" + std::to_string(artifacts_.size()),
            .projectId = projectId,
            .filePath = filePath,
            .isIntermediateStem = true,
            .createdAtUtc = now,
            .expiresAtUtc = now + std::chrono::hours(INTERMEDIATE_STEM_RETENTION_HOURS)
        };
        artifacts_.push_back(artifact);
    }

    void registerFinalExport(const std::string& projectId, const std::string& filePath) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::system_clock::now();
        StoredArtifact artifact{
            .artifactId = projectId + "_export_" + std::to_string(artifacts_.size()),
            .projectId = projectId,
            .filePath = filePath,
            .isIntermediateStem = false,
            .createdAtUtc = now,
            .expiresAtUtc = now + std::chrono::hours(24 * FINAL_EXPORT_RETENTION_DAYS)
        };
        artifacts_.push_back(artifact);
    }

    /**
     * @brief Immediately deletes a project and all its associated stems/artifacts.
     */
    void deleteProjectImmediately(const std::string& projectId) {
        std::lock_guard<std::mutex> lock(mutex_);
        database_.deleteProject(projectId);

        std::erase_if(artifacts_, [&](const StoredArtifact& a) {
            return a.projectId == projectId;
        });
    }

    [[nodiscard]] size_t getActiveArtifactCount(const std::string& projectId) const {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t count = 0;
        for (const auto& a : artifacts_) {
            if (a.projectId == projectId) count++;
        }
        return count;
    }

    /**
     * @brief Purges expired intermediate stems that passed their 24h retention window.
     */
    size_t purgeExpiredArtifacts(std::chrono::system_clock::time_point currentTime) {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t initialSize = artifacts_.size();
        std::erase_if(artifacts_, [&](const StoredArtifact& a) {
            return currentTime >= a.expiresAtUtc;
        });
        return initialSize - artifacts_.size();
    }

private:
    LocalDatabase& database_;
    mutable std::mutex mutex_;
    std::vector<StoredArtifact> artifacts_;
};

} // namespace reggaewave::storage
