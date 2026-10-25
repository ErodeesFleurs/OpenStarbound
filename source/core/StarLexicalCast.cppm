module;
import std;
import star.format;
import star.string;
import star.string_view;

namespace Star {

struct BadLexicalCastTag {
  static constexpr std::string_view name() { return "BadLexicalCast"; }
};
using BadLexicalCast = StarError<BadLexicalCastTag, StarException>;

BadLexicalCast lexicalCastError(std::errc ec, std::string_view text);

// Full-token range errors remain successful: integers retain the destination,
// while the pinned libc++ floating parser writes signed infinity or zero.
template <typename Type>
[[nodiscard]] bool tryLexicalCast(Type& result, std::string_view text) {
  auto res = std::from_chars(text.data(), text.data() + text.size(), result);
  return res.ptr == text.data() + text.size() && (res.ec == std::errc() || res.ec == std::errc::result_out_of_range);
}

template <>
[[nodiscard]] bool tryLexicalCast(bool& result, std::string_view text);

template <typename Type>
[[nodiscard]] bool tryLexicalCast(Type& result, String const& s) {
  return tryLexicalCast<Type>(result, std::string_view(s.utf8()));
}

template <typename Type>
[[nodiscard]] bool tryLexicalCast(Type& result, StringView s) {
  return tryLexicalCast<Type>(result, s.utf8());
}

template <typename Type>
[[nodiscard]] std::optional<Type> maybeLexicalCast(std::string_view text) {
  Type result{};
  if (tryLexicalCast(result, text))
    return result;
  else
    return {};
}

template <typename Type>
[[nodiscard]] std::optional<Type> maybeLexicalCast(StringView s) {
  return maybeLexicalCast<Type>(s.utf8());
}

template <typename Type>
[[nodiscard]] std::expected<Type, StarException> lexicalCast(std::string_view text) {
  Type result{};
  auto res = std::from_chars(text.data(), text.data() + text.size(), result);
  if ((res.ec != std::errc() && res.ec != std::errc::result_out_of_range) || res.ptr != text.data() + text.size())
    return std::unexpected(lexicalCastError(res.ec, text));
  
  return result;
}

template <>
[[nodiscard]] std::expected<bool, StarException> lexicalCast(std::string_view text);

template <typename Type>
[[nodiscard]] std::expected<Type, StarException> lexicalCast(StringView s) {
  return lexicalCast<Type>(s.utf8());
}

template <typename Type, std::size_t N>
[[nodiscard]] bool tryLexicalCast(Type& result, char const (&text)[N]) {
  return tryLexicalCast<Type>(result, std::string_view(text));
}

template <typename Type>
[[nodiscard]] bool tryLexicalCast(Type& result, std::string const& text) {
  return tryLexicalCast<Type>(result, std::string_view(text));
}

template <typename Type, std::size_t N>
[[nodiscard]] std::optional<Type> maybeLexicalCast(char const (&text)[N]) {
  return maybeLexicalCast<Type>(std::string_view(text));
}

template <typename Type>
[[nodiscard]] std::optional<Type> maybeLexicalCast(std::string const& text) {
  return maybeLexicalCast<Type>(std::string_view(text));
}

template <typename Type, std::size_t N>
[[nodiscard]] std::expected<Type, StarException> lexicalCast(char const (&text)[N]) {
  return lexicalCast<Type>(std::string_view(text));
}

template <typename Type>
[[nodiscard]] std::expected<Type, StarException> lexicalCast(std::string const& text) {
  return lexicalCast<Type>(std::string_view(text));
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
