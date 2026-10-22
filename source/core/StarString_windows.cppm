module;

#include <windows.h>

import std;
import star.format;
import star.string;

namespace Star {

[[nodiscard]] std::expected<String, StarException> utf16ToString(WCHAR const* s);
[[nodiscard]] std::expected<std::unique_ptr<WCHAR[]>, StarException> stringToUtf16(String const& s);

}

export module star.string_windows;

export namespace Star {
  using ::Star::utf16ToString;
  using ::Star::stringToUtf16;
}
