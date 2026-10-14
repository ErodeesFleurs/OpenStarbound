module;
#include "StarConfig.hpp"

#define NOMINMAX
#include <windows.h>

import star.string;

namespace Star {

String utf16ToString(WCHAR const* s);
unique_ptr<WCHAR[]> stringToUtf16(String const& s);

}

export module star.string_windows;

export namespace Star {
  using ::Star::utf16ToString;
  using ::Star::stringToUtf16;
}
