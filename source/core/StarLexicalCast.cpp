#include "StarLexicalCast.hpp"

namespace Star {

void throwLexicalCastError(std::errc ec, const char* first, const char* last) {
  StringView str(first, last - first);
  if (ec == std::errc::invalid_argument)
    throw BadLexicalCast(strf("Lexical cast failed on '{}' (invalid argument)", str));
  else
    throw BadLexicalCast(strf("Lexical cast failed on '{}'", str));
}

template <>
bool tryLexicalCast(bool& result, const char* first, const char* last) {
  size_t len = last - first;
  // Compare the whole literal, not just its first len characters: strncmp with
  // the input length accepted short prefixes such as "t" or "tru".
  if (len == 4 && memcmp(first, "true", 4) == 0) {
    result = true;
    return true;
  }
  if (len == 5 && memcmp(first, "false", 5) == 0) {
    result = false;
    return true;
  }

  // The "true" case used to fall through to the assignment below, so a
  // successful cast of "true" reported success but stored false.
  return false;
}

template <>
bool lexicalCast(const char* first, const char* last) {
  size_t len = last - first;
  if (len == 4 && memcmp(first, "true", 4) == 0)
    return true;
  else if (len != 5 || memcmp(first, "false", 5) != 0)
    throwLexicalCastError(std::errc(), first, last);

  return false;
}

}