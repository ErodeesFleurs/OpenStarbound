import std;
import star.format;
import star.string_view;

import star.lexical_cast;

namespace Star {

BadLexicalCast lexicalCastError(std::errc ec, const char* first, const char* last) {
  std::string_view str(first, last - first);
  if (ec == std::errc::invalid_argument)
    return BadLexicalCast(strf("Lexical cast failed on '{}' (invalid argument)", str));
  else
    return BadLexicalCast(strf("Lexical cast failed on '{}'", str));
}

template <>
bool tryLexicalCast(bool& result, const char* first, const char* last) {
  std::size_t len = last - first;
  // Compare the whole literal, not just its first len characters: strncmp with
  // the input length accepted short prefixes such as "t" or "tru".
  if (len == 4 && std::memcmp(first, "true", 4) == 0) {
    result = true;
    return true;
  }
  if (len == 5 && std::memcmp(first, "false", 5) == 0) {
    result = false;
    return true;
  }

  // The "true" case used to fall through to the assignment below, so a
  // successful cast of "true" reported success but stored false.
  return false;
}

template <>
std::expected<bool, StarException> lexicalCast(const char* first, const char* last) {
  std::size_t len = last - first;
  if (len == 4 && std::memcmp(first, "true", 4) == 0)
    return true;
  else if (len != 5 || std::memcmp(first, "false", 5) != 0)
    return std::unexpected(lexicalCastError(std::errc(), first, last));

  return false;
}

}