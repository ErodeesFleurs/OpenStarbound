#include "StarLexicalCast.hpp"

import std;

namespace Star {

void throwLexicalCastError(std::errc ec, const char* first, const char* last) {
  StringView str(first, last - first);
  if (ec == std::errc::invalid_argument)
    throw BadLexicalCast(strf("Lexical cast failed on '{}' (invalid argument)", str));
  else
    throw BadLexicalCast(strf("Lexical cast failed on '{}'", str));
}

template <>
auto tryLexicalCast(bool& result, const char* first, const char* last) -> bool {
  std::size_t len = last - first;
  if (std::strncmp(first, "true", len) == 0)
    result = true;
  else if (std::strncmp(first, "false", len) != 0)
    return false;

  result = false;
  return true;
}

template <>
auto lexicalCast(const char* first, const char* last) -> bool {
  std::size_t len = last - first;
  if (std::strncmp(first, "true", len) == 0)
    return true;
  else if (std::strncmp(first, "false", len) != 0)
    throwLexicalCastError(std::errc(), first, last);

  return false;
}

}// namespace Star
