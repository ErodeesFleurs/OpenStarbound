#pragma once

#include "StarException.hpp"
#include "StarString.hpp"

import std;

namespace Star {

// Currently the specification of the "language" is incredibly simple The only
// thing we process are quoted strings and backslashes Backslashes function as
// a useful subset of C++ This means: Newline: \n Tab: \t Backslash: \\ Single
// Quote: \' Double Quote: \" Null: \0 Space: "\ " (without quotes ofc, not
// actually C++) Also \v \b \a \f \r Plus Unicode \uxxxx Not implemented octal
// and hexadecimal, because it's possible to construct invalid unicode code
// points using them

using ShellParsingException = ExceptionDerived<"ShellParsingException">;

class ShellParser {
public:
  ShellParser();
  using Char = String::Char;

  enum class TokenType {
    Word,
    // TODO: braces, brackets, actual shell stuff

  };

  struct Token {
    TokenType type;
    String token;
  };

  auto tokenize(String const& command) -> List<Token>;
  auto tokenizeToStringList(String const& command) -> StringList;

private:
  void init(String const& command);

  auto word() -> String;
  auto parseBackslash() -> Char;
  auto parseUnicodeEscapeSequence(std::optional<Char> previousCodepoint = {}) -> Char;

  auto isSpace(Char letter) const -> bool;
  auto isQuote(Char letter) const -> bool;

  auto inQuotedString() const -> bool;
  auto notDone() const -> bool;

  auto current() const -> std::optional<Char>;
  auto next() -> std::optional<Char>;
  auto previous() -> std::optional<Char>;

  String::const_iterator m_begin;
  String::const_iterator m_current;
  String::const_iterator m_end;

  Char m_quotedType;
};

}
