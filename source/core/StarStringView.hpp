#pragma once

#include "StarString.hpp"

import std;

namespace Star {

// This is a StringView version of Star::String
// I literally just copy-pasted it all from there
class StringView {
public:
  typedef String::Char Char;

  typedef U8ToU32Iterator<std::string_view::const_iterator> const_iterator;
  typedef Char value_type;
  typedef value_type const& const_reference;

  using CaseSensitivity = String::CaseSensitivity;

  StringView();
  StringView(StringView const& s);
  StringView(StringView&& s) noexcept;
  StringView(String const& s);

  // These assume utf8 input
  StringView(char const* s);
  StringView(char const* s, std::size_t n);
  StringView(std::string_view const& s);
  StringView(std::string_view&& s) noexcept;
  StringView(std::string const& s);

  StringView(Char const* s);
  StringView(Char const* s, std::size_t n);

  // const& to internal utf8 data
  std::string_view const& utf8() const;
  std::string_view takeUtf8();
  ByteArray utf8Bytes() const;
  // Pointer to internal utf8 data, null-terminated.
  char const* utf8Ptr() const;
  std::size_t utf8Size() const;

  const_iterator begin() const;
  const_iterator end() const;

  std::size_t size() const;
  std::size_t length() const;

  bool empty() const;

  Char operator[](std::size_t index) const;
  // Throws StringException if i out of range.
  Char at(std::size_t i) const;

  bool endsWith(StringView end, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const;
  bool endsWith(Char end, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const;
  bool beginsWith(StringView beg, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const;
  bool beginsWith(Char beg, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const;

  using SplitCallback = std::function<void(StringView, std::size_t, std::size_t)>;
  void forEachSplitAnyView(StringView pattern, SplitCallback) const;
  void forEachSplitView(StringView pattern, SplitCallback) const;

  bool hasChar(Char c) const;
  // Identical to hasChar, except, if string is empty, tests if c is
  // whitespace.
  bool hasCharOrWhitespace(Char c) const;

  size_t find(Char c, size_t beg = 0, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const;
  size_t find(StringView s, size_t beg = 0, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const;
  size_t findLast(Char c, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const;
  size_t findLast(StringView s, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const;

  // If pattern is empty, finds first whitespace
  size_t findFirstOf(StringView chars = "", size_t beg = 0) const;

  // If pattern is empty, finds first non-whitespace
  size_t findFirstNotOf(StringView chars = "", size_t beg = 0) const;

  // finds the the start of the next 'boundary' in a string.  used for quickly
  // scanning a string
  size_t findNextBoundary(size_t index, bool backwards = false) const;

  bool contains(StringView s, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const;

  int compare(StringView s, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const;
  bool equals(StringView s, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const;
  // Synonym for equals(s, String::CaseInsensitive)
  bool equalsIgnoreCase(StringView s) const;

  StringView substr(std::size_t position, std::size_t n = std::numeric_limits<std::size_t>::max()) const;

  StringView& operator=(StringView s);

  friend bool operator==(StringView s1, const char* s2);
  friend bool operator==(StringView s1, std::string const& s2);
  friend bool operator==(StringView s1, String const& s2);
  friend bool operator==(StringView s1, StringView s2);
  friend bool operator!=(StringView s1, StringView s2);
  friend bool operator<(StringView s1, StringView s2);

  friend std::ostream& operator<<(std::ostream& os, StringView const& s);

private:
  int compare(std::size_t selfOffset,
      std::size_t selfLen,
      StringView other,
      std::size_t otherOffset,
      std::size_t otherLen,
      CaseSensitivity cs) const;

  std::string_view m_view;
};

}

template <>
struct std::formatter<Star::StringView> : std::formatter<std::string_view> {
  auto format(Star::StringView const& s, std::format_context& ctx) const {
    return std::formatter<std::string_view>::format(s.utf8(), ctx);
  }
};
