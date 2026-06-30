#pragma once

#include "StarString.hpp"
#include "StarEncode.hpp"
#include "StarBytes.hpp"
#include "StarFormat.hpp"

namespace Star {

// Currently the specification of the "language" is incredibly simple The only
// thing we process are quoted strings and backslashes Backslashes function as
// a useful subset of C++ This means: Newline: \n Tab: \t Backslash: \\ Single
// Quote: \' Double Quote: \" Null: \0 Space: "\ " (without quotes ofc, not
// actually C++) Also \v \b \a \f \r Plus Unicode \uxxxx Not implemented octal
// and hexadecimal, because it's possible to construct invalid unicode code
// points using them

struct ShellParsingExceptionTag { static constexpr char const* typeName = "ShellParsingException"; };
using ShellParsingException = TypedException<StarException, ShellParsingExceptionTag>;

class ShellParser {
public:
  ShellParser() = default;
  using Char = String::Char;

  enum class TokenType {
    Word,
    // TODO: braces, brackets, actual shell stuff

  };

  struct Token {
    TokenType type;
    String token;
  };

  [[nodiscard]] List<Token> tokenize(String const& command);
  [[nodiscard]] StringList tokenizeToStringList(String const& command);

private:
  void init(String const& command);

  [[nodiscard]] String word();
  [[nodiscard]] Char parseBackslash();
  [[nodiscard]] Char parseUnicodeEscapeSequence(Maybe<Char> previousCodepoint = {});

  [[nodiscard]] bool isSpace(Char letter) const;
  [[nodiscard]] bool isQuote(Char letter) const;

  [[nodiscard]] bool inQuotedString() const;
  [[nodiscard]] bool notDone() const;

  [[nodiscard]] Maybe<Char> current() const;
  [[nodiscard]] Maybe<Char> next();
  [[nodiscard]] Maybe<Char> previous();

  String::const_iterator m_begin;
  String::const_iterator m_current;
  String::const_iterator m_end;

  Char m_quotedType = '\0';
};

}
