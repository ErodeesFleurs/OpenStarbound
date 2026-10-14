module;

#include <windows.h>

import std;
import star.string;

namespace Star {

String utf16ToString(WCHAR const* s);
std::unique_ptr<WCHAR[]> stringToUtf16(String const& s);

}

export module star.string_windows;

export namespace Star {
  using ::Star::utf16ToString;
  using ::Star::stringToUtf16;
}
