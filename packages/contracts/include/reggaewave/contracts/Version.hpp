#pragma once

#include <string_view>

#ifndef REGGAEWAVE_APP_VERSION_STRING
#define REGGAEWAVE_APP_VERSION_STRING "1.2.6-260820b"
#endif

namespace reggaewave::contracts {

inline constexpr std::string_view kAppVersion = REGGAEWAVE_APP_VERSION_STRING;

} // namespace reggaewave::contracts
