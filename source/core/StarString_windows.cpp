#include "StarString_windows.hpp"

namespace Star {

size_t wcharLen(WCHAR const* s) {
  size_t size = 0;
  while (*s) {
    ++size;
    ++s;
  }
  return size;
}

String utf16ToString(WCHAR const* s) {
  if (!s)
    return "";
  int sLen = wcharLen(s);
  int utf8Len = WideCharToMultiByte(CP_UTF8, 0, s, sLen + 1, nullptr, 0, nullptr, nullptr);
  auto utf8Buffer = new char[utf8Len];
  WideCharToMultiByte(CP_UTF8, 0, s, sLen + 1, utf8Buffer, utf8Len, nullptr, nullptr);
  auto result = String(utf8Buffer, utf8Len - 1);
  delete[] utf8Buffer;
  return result;
}

unique_ptr<WCHAR[]> stringToUtf16(String const& s) {
  int utf16Len = MultiByteToWideChar(CP_UTF8, 0, s.utf8Ptr(), s.utf8Size() + 1, nullptr, 0);
  auto result = make_unique<WCHAR[]>(utf16Len);
  MultiByteToWideChar(CP_UTF8, 0, s.utf8Ptr(), s.utf8Size() + 1, result.get(), utf16Len);
  return result;
}

}
