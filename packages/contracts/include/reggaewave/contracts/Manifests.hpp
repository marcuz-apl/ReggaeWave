#pragma once

#include <string>
#include <vector>
#include <chrono>

namespace reggaewave::contracts {

struct MusicalAnalysisManifest {
    double bpm = 120.0;
    std::string key = "C Major";
    int timeSignatureNumerator = 4;
    int timeSignatureDenominator = 4;
    std::vector<std::string> detectedChords;
    std::vector<std::string> detectedSections; // e.g. "intro", "verse", "chorus"
    double confidenceScore = 1.0;              // [0.0, 1.0]
    bool requiresWarning = false;
};

struct VariationManifest {
    std::string variationId;     // "variation_a" or "variation_b"
    std::string label;           // e.g. "Variation A (Classic Roots / One-Drop)"
    std::string rhythmStyle;     // "ONE_DROP", "STEPPERS", "ROCKERS"
    double integratedLufs = -14.0;
    double truePeakDb = -1.0;
    double durationSeconds = 0.0;
    std::string audioFilePath;
};

struct ProjectManifest {
    std::string projectId;
    std::string projectName;
    std::string sourceFilePath;
    std::string rightsAttestationBasis;
    int reggaeIntensity = 70;
    int dubEffectsAmount = 20;
    double vocalLevelDb = 0.0;
    bool subtitlesEnabled = false;
    std::string selectedVariationId; // "variation_a" or "variation_b"
    std::chrono::system_clock::time_point createdAtUtc;
    std::chrono::system_clock::time_point updatedAtUtc;
};

} // namespace reggaewave::contracts
