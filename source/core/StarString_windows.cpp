#include <windows.h>
import std;
import star.string_windows;
import star.string;
import star.format;

namespace Star {

std::size_t wcharLen(WCHAR const* s) {
  std::size_t size = 0;
  while (*s) {
    ++size;
    ++s;
  }
  return size;
}

std::expected<String, StarException> utf16ToString(WCHAR const* s) {
  if (!s) return String();
  auto length = wcharLen(s);
  if (length == 0) return String();
  if (length > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    return std::unexpected(StringException("UTF-16 string exceeds Windows conversion limit"));
  auto count = static_cast<int>(length);
  int utf8Length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, s, count, nullptr, 0, nullptr, nullptr);
  if (utf8Length == 0)
    return std::unexpected(StringException::format("UTF-16 conversion failed: Windows error {}", GetLastError()));
  std::string bytes(static_cast<std::size_t>(utf8Length), '\0');
  if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, s, count, bytes.data(), utf8Length, nullptr, nullptr) == 0)
    return std::unexpected(StringException::format("UTF-16 conversion failed: Windows error {}", GetLastError()));
  return String(std::move(bytes));
}

std::expected<std::unique_ptr<WCHAR[]>, StarException> stringToUtf16(String const& s) {
  if (s.utf8Size() >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
    return std::unexpected(StringException("UTF-8 string exceeds Windows conversion limit"));
  auto count = static_cast<int>(s.utf8Size() + 1);
  int utf16Length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.utf8Ptr(), count, nullptr, 0);
  if (utf16Length == 0)
    return std::unexpected(StringException::format("UTF-8 conversion failed: Windows error {}", GetLastError()));
  auto result = std::make_unique<WCHAR[]>(static_cast<std::size_t>(utf16Length));
  if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.utf8Ptr(), count, result.get(), utf16Length) == 0)
    return std::unexpected(StringException::format("UTF-8 conversion failed: Windows error {}", GetLastError()));
  return result;
}

}
