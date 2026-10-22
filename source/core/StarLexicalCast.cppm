module;
import std;
import star.format;
import star.string;
import star.string_view;
#include "fast_float.h"

namespace Star {

struct BadLexicalCastTag {
  static constexpr char const* name() { return "BadLexicalCast"; }
};
using BadLexicalCast = StarError<BadLexicalCastTag, StarException>;

BadLexicalCast lexicalCastError(std::errc ec, const char* first, const char* last);

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
[[nodiscard]] std::optional<Type> maybeLexicalCast(const char* first, const char* last) {
  Type result{};
  if (tryLexicalCast(result, first, last))
    return result;
  else
    return {};
}

template <typename Type>
[[nodiscard]] std::optional<Type> maybeLexicalCast(StringView s) {
  return maybeLexicalCast<Type>(s.utf8Ptr(), s.utf8Ptr() + s.utf8Size());
}

template <typename Type>
[[nodiscard]] std::expected<Type, StarException> lexicalCast(const char* first, const char* last) {
  Type result{};
  auto res = fast_float::from_chars(first, last, result);
  if ((res.ec != std::errc() && res.ec != std::errc::result_out_of_range) || res.ptr != last)
    return std::unexpected(lexicalCastError(res.ec, first, last));
  
  return result;
}

template <>
[[nodiscard]] std::expected<bool, StarException> lexicalCast(const char* first, const char* last);

template <typename Type>
[[nodiscard]] std::expected<Type, StarException> lexicalCast(StringView s) {
  return lexicalCast<Type>(s.utf8Ptr(), s.utf8Ptr() + s.utf8Size());
}

}

export module star.lexical_cast;

export namespace Star {
  using ::Star::BadLexicalCastTag;
  using ::Star::BadLexicalCast;
  using ::Star::lexicalCastError;
  using ::Star::tryLexicalCast;
  using ::Star::maybeLexicalCast;
  using ::Star::lexicalCast;
}
