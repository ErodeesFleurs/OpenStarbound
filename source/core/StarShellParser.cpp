#include "StarShellParser.hpp"

namespace Star {

[[nodiscard]] auto ShellParser::tokenize(String const& command) -> List<Token> {
  List<Token> res;

  init(command);

  while (notDone()) {
    res.append(Token{TokenType::Word, word()});
  }

  return res;
}

[[nodiscard]] StringList ShellParser::tokenizeToStringList(String const& command) {
  StringList res;
  for (auto token : tokenize(command)) {
    if (token.type == TokenType::Word) {
      res.append(std::move(token.token));
    }
  }

  return res;
}

void ShellParser::init(String const& string) {
  m_begin = string.begin();
  m_current = m_begin;
  m_end = string.end();
  m_quotedType = '\0';
}

[[nodiscard]] String ShellParser::word() {
  String res;

  while (notDone()) {
    auto letter = *current();
    bool escapedLetter = false;

    if (letter == '\\') {
      escapedLetter = true;
      letter = parseBackslash();
    }

    if (!escapedLetter) {
      if (isSpace(letter) && !inQuotedString()) {
        (void)next();
        if (res.size()) {
          return res;
        }
        continue;
      }

      if (isQuote(letter)) {
        if (inQuotedString() && letter == m_quotedType) {
          m_quotedType = '\0';
          (void)next();
          continue;
        }

        if (!inQuotedString()) {
          m_quotedType = letter;
          (void)next();
          continue;
        }
      }
    }

    res.append(letter);
    (void)next();
  }

  return res;
}

[[nodiscard]] bool ShellParser::isSpace(Char letter) const {
  return String::isSpace(letter);
}

[[nodiscard]] bool ShellParser::isQuote(Char letter) const {
  return letter == '\'' || letter == '"';
}

[[nodiscard]] bool ShellParser::inQuotedString() const {
  return m_quotedType != '\0';
}

[[nodiscard]] auto ShellParser::current() const -> Maybe<Char> {
  if (m_current == m_end) {
    return {};
  }

  return *m_current;
}

[[nodiscard]] auto ShellParser::next() -> Maybe<Char> {
  if (m_current != m_end) {
    ++m_current;
  }

  return current();
}

[[nodiscard]] auto ShellParser::previous() -> Maybe<Char> {
  if (m_current != m_begin) {
    --m_current;
  }

  return current();
}

[[nodiscard]] auto ShellParser::parseBackslash() -> Char {
  auto letter = next();

  if (!letter) {
    return '\\';
  }

  switch (*letter) {
    case ' ':
      return ' ';
    case 'n':
      return '\n';
    case 't':
      return '\t';
    case 'r':
      return '\r';
    case 'b':
      return '\b';
    case 'v':
      return '\v';
    case 'f':
      return '\f';
    case 'a':
      return '\a';
    case '\'':
      return '\'';
    case '"':
      return '"';
    case '\\':
      return '\\';
    case '0':
      return '\0';
    case 'u': {
      auto letter = parseUnicodeEscapeSequence();
      if (isUtf16LeadSurrogate(letter)) {
        auto shouldBeSlash = next();
        if (shouldBeSlash && shouldBeSlash == '\\') {
          auto shouldBeU = next();
          if (shouldBeU && shouldBeU == 'u') {
            return parseUnicodeEscapeSequence(letter);
          } else {
            (void)previous();
          }
        }
        (void)previous();
        return Utf32ReplacementChar;
      } else {
        return letter;
      }
    }
    default:
      return *letter;
  }
}

[[nodiscard]] auto ShellParser::parseUnicodeEscapeSequence(Maybe<Char> previousCodepoint) -> Char {
  String codepoint;

  auto letter = current();

  while (!isSpace(*letter) && codepoint.size() < 4) {
    auto letter = next();
    if (!letter) {
      break;
    }

    if (!isxdigit(*letter)) {
      return Utf32ReplacementChar;
    }

    codepoint.append(*letter);
  }

  if (!codepoint.size()) {
    return 'u';
  }

  if (codepoint.size() != 4) // exactly 4 digits are required by \u
    return Utf32ReplacementChar;

  try {
    return hexStringToUtf32(codepoint.utf8(), previousCodepoint);
  } catch (UnicodeException const&) {
    return Utf32ReplacementChar;
  }
}

[[nodiscard]] bool ShellParser::notDone() const {
  return m_current != m_end;
}

}
