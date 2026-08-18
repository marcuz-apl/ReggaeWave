#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <stdexcept>

namespace reggaewave::audio {

struct LyricSegment {
    double startTimeSeconds = 0.0;
    double endTimeSeconds = 0.0;
    std::string text;
};

/**
 * @brief Manages optional lyric transcription, user revisions, and SRT/VTT formatting.
 * 
 * Invariants from PRD Section 7.3 & 8.1:
 * - Subtitles are disabled by default on every new project.
 * - Machine transcript is stored separately from user manual revisions.
 * - Subtitle failures never block or invalidate audio exports.
 */
class SubtitleManager {
public:
    SubtitleManager() = default;

    void setMachineTranscript(std::vector<LyricSegment> segments) {
        machineTranscript_ = std::move(segments);
        if (userRevisedTranscript_.empty()) {
            userRevisedTranscript_ = machineTranscript_;
        }
    }

    void setUserRevisions(std::vector<LyricSegment> segments) {
        userRevisedTranscript_ = std::move(segments);
    }

    [[nodiscard]] const std::vector<LyricSegment>& getMachineTranscript() const noexcept {
        return machineTranscript_;
    }

    [[nodiscard]] const std::vector<LyricSegment>& getUserRevisions() const noexcept {
        return userRevisedTranscript_;
    }

    [[nodiscard]] bool hasTranscript() const noexcept {
        return !userRevisedTranscript_.empty();
    }

    /**
     * @brief Generates standard SubRip (.srt) subtitle text.
     */
    [[nodiscard]] std::string formatSrt() const {
        std::ostringstream ss;
        const auto& segments = userRevisedTranscript_;

        for (size_t i = 0; i < segments.size(); ++i) {
            ss << (i + 1) << "\n";
            ss << formatTimestampSrt(segments[i].startTimeSeconds) << " --> "
               << formatTimestampSrt(segments[i].endTimeSeconds) << "\n";
            ss << segments[i].text << "\n\n";
        }

        return ss.str();
    }

    /**
     * @brief Generates WebVTT (.vtt) subtitle text.
     */
    [[nodiscard]] std::string formatVtt() const {
        std::ostringstream ss;
        ss << "WEBVTT\n\n";
        const auto& segments = userRevisedTranscript_;

        for (size_t i = 0; i < segments.size(); ++i) {
            ss << (i + 1) << "\n";
            ss << formatTimestampVtt(segments[i].startTimeSeconds) << " --> "
               << formatTimestampVtt(segments[i].endTimeSeconds) << "\n";
            ss << segments[i].text << "\n\n";
        }

        return ss.str();
    }

    /**
     * @brief Find active lyric text at a specific timestamp.
     */
    [[nodiscard]] std::string getActiveLyricAtTime(double timestampSeconds) const {
        for (const auto& seg : userRevisedTranscript_) {
            if (timestampSeconds >= seg.startTimeSeconds && timestampSeconds <= seg.endTimeSeconds) {
                return seg.text;
            }
        }
        return "";
    }

private:
    static std::string formatTimestampSrt(double seconds) {
        int totalSec = static_cast<int>(std::floor(seconds));
        int ms = static_cast<int>(std::round((seconds - totalSec) * 1000.0));
        if (ms >= 1000) { ms = 999; }

        int hours = totalSec / 3600;
        int mins = (totalSec % 3600) / 60;
        int secs = totalSec % 60;

        std::ostringstream ss;
        ss << std::setfill('0')
           << std::setw(2) << hours << ":"
           << std::setw(2) << mins << ":"
           << std::setw(2) << secs << ","
           << std::setw(3) << ms;
        return ss.str();
    }

    static std::string formatTimestampVtt(double seconds) {
        int totalSec = static_cast<int>(std::floor(seconds));
        int ms = static_cast<int>(std::round((seconds - totalSec) * 1000.0));
        if (ms >= 1000) { ms = 999; }

        int hours = totalSec / 3600;
        int mins = (totalSec % 3600) / 60;
        int secs = totalSec % 60;

        std::ostringstream ss;
        ss << std::setfill('0')
           << std::setw(2) << hours << ":"
           << std::setw(2) << mins << ":"
           << std::setw(2) << secs << "."
           << std::setw(3) << ms;
        return ss.str();
    }

    std::vector<LyricSegment> machineTranscript_;
    std::vector<LyricSegment> userRevisedTranscript_;
};

} // namespace reggaewave::audio
