#pragma once

#include "StarException.hpp"
#include "StarString.hpp"
#include "StarStringView.hpp"

import std;

namespace Star {

using BadLexicalCast = ExceptionDerived<"BadLexicalCast">;

void throwLexicalCastError(std::errc ec, const char* first, const char* last);

template <typename Type>
auto tryLexicalCast(Type& result, const char* first, const char* last) -> bool {
  auto [ptr, ec] = std::from_chars(first, last, result);
  return ptr == last && (ec == std::errc{} || ec == std::errc::result_out_of_range);
}

template <>
auto tryLexicalCast(bool& result, const char* first, const char* last) -> bool;

template <typename Type>
auto tryLexicalCast(Type& result, String const& s) -> bool {
  return tryLexicalCast<Type>(result, s.utf8Ptr(), s.utf8Ptr() + s.utf8Size());
}

template <typename Type>
auto tryLexicalCast(Type& result, StringView s) -> bool {
  return tryLexicalCast<Type>(result, s.utf8Ptr(), s.utf8Ptr() + s.utf8Size());
}

template <typename Type>
[[nodiscard]] auto maybeLexicalCast(const char* first, const char* last) -> std::optional<Type> {
  Type result{};
  if (tryLexicalCast(result, first, last))
    return result;
  else
    return std::nullopt;
}

template <typename Type>
[[nodiscard]] auto maybeLexicalCast(StringView s) -> std::optional<Type> {
  return maybeLexicalCast<Type>(s.utf8Ptr(), s.utf8Ptr() + s.utf8Size());
}

template <typename Type>
[[nodiscard]] auto lexicalCast(const char* first, const char* last) -> Type {
  Type result{};
  auto [ptr, ec] = std::from_chars(first, last, result);
  if (ec != std::errc{} || ptr != last)
    throwLexicalCastError(ec, first, last);

  return result;
}

template <>
auto lexicalCast(const char* first, const char* last) -> bool;

template <typename Type>
auto lexicalCast(StringView s) -> Type {
  return lexicalCast<Type>(s.utf8Ptr(), s.utf8Ptr() + s.utf8Size());
}

}// namespace Star
