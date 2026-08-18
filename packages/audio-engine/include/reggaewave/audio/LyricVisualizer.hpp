#pragma once

#include <reggaewave/audio/SubtitleManager.hpp>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace reggaewave::audio {

struct VisualizerFrame {
    int frameIndex = 0;
    double timestampSeconds = 0.0;
    int width = 1920;
    int height = 1080;
    std::string activeLyricText;
    float audioRmsLevel = 0.0f;
    std::string trackTitle = "ReggaeWave Master";
};

/**
 * @brief Lyric visualizer renderer for MP4 video export.
 */
class LyricVisualizer {
public:
    static constexpr int DEFAULT_WIDTH = 1920;
    static constexpr int DEFAULT_HEIGHT = 1080;
    static constexpr double DEFAULT_FPS = 30.0;

    LyricVisualizer() = default;

    /**
     * @brief Computes visualizer frame parameters at a specific video frame index.
     */
    static VisualizerFrame renderFrame(int frameIndex,
                                       double fps,
                                       const SubtitleManager& subtitleManager,
                                       const std::vector<std::vector<float>>& audioChannels,
                                       const std::string& trackTitle = "ReggaeWave Master") {
        VisualizerFrame frame;
        frame.frameIndex = frameIndex;
        frame.timestampSeconds = static_cast<double>(frameIndex) / (fps > 0.0 ? fps : DEFAULT_FPS);
        frame.width = DEFAULT_WIDTH;
        frame.height = DEFAULT_HEIGHT;
        frame.trackTitle = trackTitle;

        // 1. Fetch current active lyric line
        frame.activeLyricText = subtitleManager.getActiveLyricAtTime(frame.timestampSeconds);

        // 2. Measure audio RMS level at this frame timestamp
        if (!audioChannels.empty() && !audioChannels[0].empty()) {
            size_t sampleRate = 44100;
            size_t centerSample = static_cast<size_t>(frame.timestampSeconds * sampleRate);
            size_t windowSize = static_cast<size_t>(sampleRate / fps); // One frame of audio
            size_t start = (centerSample >= windowSize / 2) ? centerSample - windowSize / 2 : 0;
            size_t end = std::min(start + windowSize, audioChannels[0].size());

            float sumSq = 0.0f;
            size_t count = 0;
            for (size_t ch = 0; ch < audioChannels.size(); ++ch) {
                for (size_t i = start; i < end; ++i) {
                    float s = audioChannels[ch][i];
                    sumSq += s * s;
                    count++;
                }
            }
            frame.audioRmsLevel = (count > 0) ? std::clamp(std::sqrt(sumSq / count), 0.0f, 1.0f) : 0.0f;
        }

        return frame;
    }
};

} // namespace reggaewave::audio
