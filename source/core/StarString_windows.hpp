#pragma once

#include <windows.h>

#include "StarString.hpp"

namespace Star {

[[nodiscard]] String utf16ToString(WCHAR const* s);
[[nodiscard]] unique_ptr<WCHAR[]> stringToUtf16(String const& s);

}
