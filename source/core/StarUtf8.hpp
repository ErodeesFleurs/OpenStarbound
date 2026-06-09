#pragma once

#include "StarUnicode.hpp"

namespace Star {

enum class CaseSensitivity {
  CaseSensitive,
  CaseInsensitive
};

inline bool isSpace(Utf32Type c) {
  return
    c == 0x20 || // space
    c == 0x09 || // horizontal tab
    c == 0x0a || // newline
    c == 0x0d || // carriage return
    c == 0xfeff; // BOM or ZWNBSP
}

inline bool isAsciiNumber(Utf32Type c) {
  return c >= '0' && c <= '9';
}

inline bool isAsciiLetter(Utf32Type c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

inline Utf32Type toLower(Utf32Type c) {
  if (c >= 'A' && c <= 'Z')
    return c + 32;
  else
    return c;
}

inline Utf32Type toUpper(Utf32Type c) {
  if (c >= 'a' && c <= 'z')
    return c - 32;
  else
    return c;
}

inline bool charEqual(Utf32Type c1, Utf32Type c2, CaseSensitivity cs) {
  if (cs == CaseSensitivity::CaseInsensitive)
    return toLower(c1) == toLower(c2);
  else
    return c1 == c2;
}

}
