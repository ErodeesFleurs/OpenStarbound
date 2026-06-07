#pragma once

#include "StarFormat.hpp"
#include "StarString.hpp"
#include "StarStringView.hpp"
#include "StarMaybe.hpp"

#include "fast_float.h"

namespace Star {

struct BadLexicalCastTag { static constexpr char const* typeName = "BadLexicalCast"; };
using BadLexicalCast = TypedException<StarException, BadLexicalCastTag>;

void throwLexicalCastError(std::errc ec, const char* first, const char* last);

template <typename Type>
[[nodiscard]] bool tryLexicalCast(Type& result, const char* first, const char* last) {
  auto res = fast_float::from_chars(first, last, result);
  return res.ptr == last && (res.ec == std::errc() || res.ec == std::errc::result_out_of_range);
}

template <>
[[nodiscard]] bool tryLexicalCast(bool& result, const char* first, const char* last);

template <typename Type>
[[nodiscard]] bool tryLexicalCast(Type& result, String const& s) {
  return tryLexicalCast<Type>(result, s.utf8Ptr(), s.utf8Ptr() + s.utf8Size());
}

template <typename Type>
[[nodiscard]] bool tryLexicalCast(Type& result, StringView s) {
  return tryLexicalCast<Type>(result, s.utf8Ptr(), s.utf8Ptr() + s.utf8Size());
}

template <typename Type>
[[nodiscard]] Maybe<Type> maybeLexicalCast(const char* first, const char* last) {
  Type result{};
  if (tryLexicalCast(result, first, last))
    return result;
  else
    return {};
}

template <typename Type>
[[nodiscard]] Maybe<Type> maybeLexicalCast(StringView s) {
  return maybeLexicalCast<Type>(s.utf8Ptr(), s.utf8Ptr() + s.utf8Size());
}

template <typename Type>
[[nodiscard]] Type lexicalCast(const char* first, const char* last) {
  Type result{};
  auto res = fast_float::from_chars(first, last, result);
  if ((res.ec != std::errc() && res.ec != std::errc::result_out_of_range) || res.ptr != last)
    throwLexicalCastError(res.ec, first, last);
  
  return result;
}

template <>
[[nodiscard]] bool lexicalCast(const char* first, const char* last);

template <typename Type>
[[nodiscard]] Type lexicalCast(StringView s) {
  return lexicalCast<Type>(s.utf8Ptr(), s.utf8Ptr() + s.utf8Size());
}

}
