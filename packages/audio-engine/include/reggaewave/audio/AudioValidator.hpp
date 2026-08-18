#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <string_view>

namespace reggaewave::audio {

enum class ValidationErrorCode {
    None,
    FileNotFound,
    FileTooLarge,        // > 200 MB
    DurationTooLong,     // > 10 minutes (600s)
    ZeroDuration,        // Empty audio
    CorruptStream,       // Unrecognized / unparseable audio stream
    RightsNotAttested,   // No valid rights confirmation
    UnsupportedChannels  // < 1 channel
};

inline constexpr std::string_view getErrorMessage(ValidationErrorCode code) noexcept {
    switch (code) {
        case ValidationErrorCode::None:
            return "Valid";
        case ValidationErrorCode::FileNotFound:
            return "Source audio file does not exist";
        case ValidationErrorCode::FileTooLarge:
            return "File size exceeds the 200 MB limit";
        case ValidationErrorCode::DurationTooLong:
            return "Audio duration exceeds the 10-minute maximum limit";
        case ValidationErrorCode::ZeroDuration:
            return "Audio file contains zero decodable samples";
        case ValidationErrorCode::CorruptStream:
            return "Corrupted, encrypted, or unsupported audio stream";
        case ValidationErrorCode::RightsNotAttested:
            return "Rights attestation must be confirmed before processing";
        case ValidationErrorCode::UnsupportedChannels:
            return "Audio must contain at least one audio channel";
    }
    return "Unknown validation error";
}

struct ValidationResult {
    bool isValid = false;
    ValidationErrorCode errorCode = ValidationErrorCode::None;
    std::string sanitizedMessage;
    
    uint64_t fileSizeBytes = 0;
    double durationSeconds = 0.0;
    int sampleRate = 0;
    int numChannels = 0;
    std::string detectedFormat;
};

/**
 * @brief Enforces audio intake constraints from PRD Section 7.1.
 */
class AudioValidator {
public:
    static constexpr uint64_t MAX_FILE_SIZE_BYTES = 200ULL * 1024ULL * 1024ULL; // 200 MB
    static constexpr double MAX_DURATION_SECONDS = 600.0; // 10 minutes
    static constexpr double MIN_DURATION_SECONDS = 0.1;   // Minimum 100ms

    static ValidationResult validateAudioMetadata(uint64_t fileSizeBytes,
                                                  double durationSeconds,
                                                  int sampleRate,
                                                  int numChannels,
                                                  bool hasRightsAttestation) {
        ValidationResult result;
        result.fileSizeBytes = fileSizeBytes;
        result.durationSeconds = durationSeconds;
        result.sampleRate = sampleRate;
        result.numChannels = numChannels;

        if (!hasRightsAttestation) {
            result.errorCode = ValidationErrorCode::RightsNotAttested;
            result.sanitizedMessage = std::string(getErrorMessage(result.errorCode));
            return result;
        }

        if (fileSizeBytes == 0) {
            result.errorCode = ValidationErrorCode::ZeroDuration;
            result.sanitizedMessage = std::string(getErrorMessage(result.errorCode));
            return result;
        }

        if (fileSizeBytes > MAX_FILE_SIZE_BYTES) {
            result.errorCode = ValidationErrorCode::FileTooLarge;
            result.sanitizedMessage = std::string(getErrorMessage(result.errorCode));
            return result;
        }

        if (numChannels < 1) {
            result.errorCode = ValidationErrorCode::UnsupportedChannels;
            result.sanitizedMessage = std::string(getErrorMessage(result.errorCode));
            return result;
        }

        if (durationSeconds <= 0.0) {
            result.errorCode = ValidationErrorCode::ZeroDuration;
            result.sanitizedMessage = std::string(getErrorMessage(result.errorCode));
            return result;
        }

        if (durationSeconds > MAX_DURATION_SECONDS) {
            result.errorCode = ValidationErrorCode::DurationTooLong;
            result.sanitizedMessage = std::string(getErrorMessage(result.errorCode));
            return result;
        }

        if (sampleRate < 8000 || sampleRate > 192000) {
            result.errorCode = ValidationErrorCode::CorruptStream;
            result.sanitizedMessage = "Sample rate out of supported audible range (8 kHz - 192 kHz)";
            return result;
        }

        result.isValid = true;
        result.errorCode = ValidationErrorCode::None;
        result.sanitizedMessage = "Success";
        return result;
    }
};

} // namespace reggaewave::audio
