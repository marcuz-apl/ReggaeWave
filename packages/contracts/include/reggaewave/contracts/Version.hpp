#pragma once

#include <string_view>

#ifndef REGGAEWAVE_APP_SEMVER
#define REGGAEWAVE_APP_SEMVER "1.2.8"
#endif

#ifndef REGGAEWAVE_APP_VERSION_STRING
#define REGGAEWAVE_APP_VERSION_STRING "1.2.8-2608211"
#endif

namespace reggaewave::contracts {

inline constexpr std::string_view kAppSemver = REGGAEWAVE_APP_SEMVER;
inline constexpr std::string_view kAppVersion = REGGAEWAVE_APP_VERSION_STRING;

} // namespace reggaewave::contracts
