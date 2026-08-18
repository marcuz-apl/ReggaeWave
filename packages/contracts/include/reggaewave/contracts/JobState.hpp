#pragma once

#include <stdexcept>
#include <string_view>
#include <unordered_set>

namespace reggaewave::contracts {

enum class ConversionJobState {
    Created,
    Importing,
    Validating,
    Queued,
    Normalizing,
    Separating,
    Analyzing,
    Arranging,
    Mixing,
    Transcribing,
    PreviewReady,
    Completed,
    Failed,
    Cancelled,
    Expired,
    Deleted
};

inline constexpr std::string_view toString(ConversionJobState state) noexcept {
    switch (state) {
        case ConversionJobState::Created:      return "CREATED";
        case ConversionJobState::Importing:    return "IMPORTING";
        case ConversionJobState::Validating:   return "VALIDATING";
        case ConversionJobState::Queued:       return "QUEUED";
        case ConversionJobState::Normalizing:  return "NORMALIZING";
        case ConversionJobState::Separating:   return "SEPARATING";
        case ConversionJobState::Analyzing:    return "ANALYZING";
        case ConversionJobState::Arranging:    return "ARRANGING";
        case ConversionJobState::Mixing:       return "MIXING";
        case ConversionJobState::Transcribing: return "TRANSCRIBING";
        case ConversionJobState::PreviewReady: return "PREVIEW_READY";
        case ConversionJobState::Completed:    return "COMPLETED";
        case ConversionJobState::Failed:       return "FAILED";
        case ConversionJobState::Cancelled:    return "CANCELLED";
        case ConversionJobState::Expired:      return "EXPIRED";
        case ConversionJobState::Deleted:      return "DELETED";
    }
    return "UNKNOWN";
}

inline constexpr bool isTerminalState(ConversionJobState state) noexcept {
    return state == ConversionJobState::Completed ||
           state == ConversionJobState::Failed ||
           state == ConversionJobState::Cancelled ||
           state == ConversionJobState::Expired ||
           state == ConversionJobState::Deleted;
}

/**
 * @brief Validates state transitions for the conversion pipeline state machine.
 */
inline bool isValidConversionTransition(ConversionJobState current, ConversionJobState next, bool subtitlesEnabled = false) noexcept {
    if (current == next) return true;

    // Terminal states can only transition to Deleted or Expired (if user cleans up)
    if (isTerminalState(current)) {
        return next == ConversionJobState::Deleted || next == ConversionJobState::Expired;
    }

    // Any non-terminal state can transition to Failed, Cancelled, or Deleted
    if (next == ConversionJobState::Failed ||
        next == ConversionJobState::Cancelled ||
        next == ConversionJobState::Deleted) {
        return true;
    }

    switch (current) {
        case ConversionJobState::Created:
            return next == ConversionJobState::Importing;
        case ConversionJobState::Importing:
            return next == ConversionJobState::Validating;
        case ConversionJobState::Validating:
            return next == ConversionJobState::Queued;
        case ConversionJobState::Queued:
            return next == ConversionJobState::Normalizing;
        case ConversionJobState::Normalizing:
            return next == ConversionJobState::Separating;
        case ConversionJobState::Separating:
            return next == ConversionJobState::Analyzing;
        case ConversionJobState::Analyzing:
            return next == ConversionJobState::Arranging;
        case ConversionJobState::Arranging:
            return next == ConversionJobState::Mixing;
        case ConversionJobState::Mixing:
            if (subtitlesEnabled) {
                return next == ConversionJobState::Transcribing;
            }
            return next == ConversionJobState::PreviewReady;
        case ConversionJobState::Transcribing:
            return next == ConversionJobState::PreviewReady;
        case ConversionJobState::PreviewReady:
            return next == ConversionJobState::Completed;
        default:
            return false;
    }
}

enum class ExportJobState {
    Created,
    Queued,
    Rendering,
    Validating,
    Completed,
    Failed,
    Cancelled,
    Expired,
    Deleted
};

inline constexpr std::string_view toString(ExportJobState state) noexcept {
    switch (state) {
        case ExportJobState::Created:    return "CREATED";
        case ExportJobState::Queued:     return "QUEUED";
        case ExportJobState::Rendering:  return "RENDERING";
        case ExportJobState::Validating: return "VALIDATING";
        case ExportJobState::Completed:  return "COMPLETED";
        case ExportJobState::Failed:     return "FAILED";
        case ExportJobState::Cancelled:  return "CANCELLED";
        case ExportJobState::Expired:    return "EXPIRED";
        case ExportJobState::Deleted:    return "DELETED";
    }
    return "UNKNOWN";
}

inline bool isValidExportTransition(ExportJobState current, ExportJobState next) noexcept {
    if (current == next) return true;
    if (current == ExportJobState::Completed ||
        current == ExportJobState::Failed ||
        current == ExportJobState::Cancelled ||
        current == ExportJobState::Expired ||
        current == ExportJobState::Deleted) {
        return next == ExportJobState::Deleted || next == ExportJobState::Expired;
    }
    if (next == ExportJobState::Failed || next == ExportJobState::Cancelled || next == ExportJobState::Deleted) {
        return true;
    }
    switch (current) {
        case ExportJobState::Created:    return next == ExportJobState::Queued;
        case ExportJobState::Queued:     return next == ExportJobState::Rendering;
        case ExportJobState::Rendering:  return next == ExportJobState::Validating;
        case ExportJobState::Validating: return next == ExportJobState::Completed;
        default: return false;
    }
}

} // namespace reggaewave::contracts
