#pragma once

#include "StarByteArray.hpp"
#include "StarException.hpp"
#include "StarFormat.hpp"
#include "StarHash.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
#include "StarUnicode.hpp"

import std;

namespace Star {

class StringList;
class StringView;

using StringException = ExceptionDerived<"StringException">;

// A Unicode string class, which is a basic UTF-8 aware wrapper around
// std::string.  Provides methods for accessing UTF-32 "Char" type, which
// provides access to each individual code point.  Printing, hashing, copying,
// and in-order access should be basically as fast as std::string, but the more
// complex string processing methods may be much worse.
//
// All case sensitive / insensitive functionality is based on ASCII tolower and
// toupper, and will have no effect on characters outside ASCII.  Therefore,
// case insensitivity is really only appropriate for code / script processing,
// not for general strings.
class String {
public:
  using Char = Utf32Type;

  // std::basic_string equivalent that guarantees const access time for
  // operator[], etc
  using WideString = std::basic_string<Char>;

  using const_iterator = U8ToU32Iterator<std::string::const_iterator>;
  using value_type = Char;
  using const_reference = value_type const&;

  enum CaseSensitivity {
    CaseSensitive,
    CaseInsensitive
  };

  // Space, horizontal tab, newline, carriage return, and BOM / ZWNBSP
  static auto isSpace(Char c) -> bool;
  static auto isAsciiNumber(Char c) -> bool;
  static auto isAsciiLetter(Char c) -> bool;

  // These methods only actually work on unicode characters below 127, i.e.
  // ASCII subset.
  static auto toLower(Char c) -> Char;
  static auto toUpper(Char c) -> Char;
  static auto charEqual(Char c1, Char c2, CaseSensitivity cs) -> bool;

  // Join two strings together with a joiner, so that only one instance of the
  // joiner is in between the left and right strings.  For example, joins "foo"
  // and "bar" with "?" to produce "foo?bar".  Gets rid of repeat joiners, so
  // "foo?" and "?bar" with "?" also becomes "foo?bar".  Also, if left or right
  // is empty, does not add a joiner, for example "" and "baz" joined with "?"
  // produces "baz".
  static auto joinWith(String const& join, String const& left, String const& right) -> String;
  template <typename... StringType>
  static auto joinWith(String const& join, String const& first, String const& second, String const& third, StringType const&... rest) -> String;

  String();
  String(String const& s);
  String(String&& s);

  // These assume utf8 input
  String(char const* s);
  String(char const* s, std::size_t n);
  String(std::string const& s);
  String(std::string&& s);

  String(std::wstring const& s);
  String(Char const* s);
  String(Char const* s, std::size_t n);
  String(Char c, std::size_t n);

  explicit String(Char c);

  // const& to internal utf8 data
  [[nodiscard]] auto utf8() const -> std::string const&;
  auto takeUtf8() -> std::string;
  [[nodiscard]] auto utf8Bytes() const -> ByteArray;
  // Pointer to internal utf8 data, null-terminated.
  [[nodiscard]] auto utf8Ptr() const -> char const*;
  [[nodiscard]] auto utf8Size() const -> std::size_t;

  [[nodiscard]] auto wstring() const -> std::wstring;
  [[nodiscard]] auto wideString() const -> WideString;

  [[nodiscard]] auto begin() const -> const_iterator;
  [[nodiscard]] auto end() const -> const_iterator;

  [[nodiscard]] auto size() const -> std::size_t;
  [[nodiscard]] auto length() const -> std::size_t;

  void clear();
  void reserve(std::size_t n);
  [[nodiscard]] auto empty() const -> bool;

  auto operator[](std::size_t i) const -> Char;
  // Throws StringException if i out of range.
  [[nodiscard]] auto at(std::size_t i) const -> Char;

  [[nodiscard]] auto toUpper() const -> String;
  [[nodiscard]] auto toLower() const -> String;
  [[nodiscard]] auto titleCase() const -> String;

  [[nodiscard]] auto endsWith(String const& end, CaseSensitivity cs = CaseSensitive) const -> bool;
  [[nodiscard]] auto endsWith(Char end, CaseSensitivity cs = CaseSensitive) const -> bool;
  [[nodiscard]] auto beginsWith(String const& beg, CaseSensitivity cs = CaseSensitive) const -> bool;
  [[nodiscard]] auto beginsWith(Char beg, CaseSensitivity cs = CaseSensitive) const -> bool;

  [[nodiscard]] auto reverse() const -> String;

  [[nodiscard]] auto rot13() const -> String;

  [[nodiscard]] auto split(Char c, std::size_t maxSplit = std::numeric_limits<std::size_t>::max()) const -> StringList;
  [[nodiscard]] auto split(String const& pattern, std::size_t maxSplit = std::numeric_limits<std::size_t>::max()) const -> StringList;
  [[nodiscard]] auto rsplit(Char c, std::size_t maxSplit = std::numeric_limits<std::size_t>::max()) const -> StringList;
  [[nodiscard]] auto rsplit(String const& pattern, std::size_t maxSplit = std::numeric_limits<std::size_t>::max()) const -> StringList;

  // Splits on any number of contiguous instances of any of the given
  // characters.  Behaves differently than regular split in that leading and
  // trailing instances of the characters are also ignored, and in general no
  // empty strings will be in the resulting split list.  If chars is empty,
  // then splits on any whitespace.
  [[nodiscard]] auto splitAny(String const& chars = "", std::size_t maxSplit = std::numeric_limits<std::size_t>::max()) const -> StringList;
  [[nodiscard]] auto rsplitAny(String const& chars = "", std::size_t maxSplit = std::numeric_limits<std::size_t>::max()) const -> StringList;

  // Split any with '\n\r'
  [[nodiscard]] auto splitLines(std::size_t maxSplit = std::numeric_limits<std::size_t>::max()) const -> StringList;
  // Shorthand for splitAny("");
  [[nodiscard]] auto splitWhitespace(std::size_t maxSplit = std::numeric_limits<std::size_t>::max()) const -> StringList;

  // Splits a string once based on the given characters (defaulting to
  // whitespace), and returns the first part.  This string is set to the
  // second part.
  auto extract(String const& chars = "") -> String;
  auto rextract(String const& chars = "") -> String;

  [[nodiscard]] auto hasChar(Char c) const -> bool;
  // Identical to hasChar, except, if string is empty, tests if c is
  // whitespace.
  [[nodiscard]] auto hasCharOrWhitespace(Char c) const -> bool;

  [[nodiscard]] auto replace(String const& rplc, String const& val, CaseSensitivity cs = CaseSensitive) const -> String;

  [[nodiscard]] auto trimEnd(String const& chars = "") const -> String;
  [[nodiscard]] auto trimBeg(String const& chars = "") const -> String;
  [[nodiscard]] auto trim(String const& chars = "") const -> String;

  [[nodiscard]] auto find(Char c, std::size_t beg = 0, CaseSensitivity cs = CaseSensitive) const -> std::size_t;
  [[nodiscard]] auto find(String const& s, std::size_t beg = 0, CaseSensitivity cs = CaseSensitive) const -> std::size_t;
  [[nodiscard]] auto findLast(Char c, CaseSensitivity cs = CaseSensitive) const -> std::size_t;
  [[nodiscard]] auto findLast(String const& s, CaseSensitivity cs = CaseSensitive) const -> std::size_t;

  // If pattern is empty, finds first whitespace
  [[nodiscard]] auto findFirstOf(String const& chars = "", std::size_t beg = 0) const -> std::size_t;

  // If pattern is empty, finds first non-whitespace
  [[nodiscard]] auto findFirstNotOf(String const& chars = "", std::size_t beg = 0) const -> std::size_t;

  // finds the the start of the next 'boundary' in a string.  used for quickly
  // scanning a string
  [[nodiscard]] auto findNextBoundary(std::size_t index, bool backwards = false) const -> std::size_t;

  [[nodiscard]] auto slice(SliceIndex a = SliceIndex(), SliceIndex b = SliceIndex(), int i = 1) const -> String;

  void append(String const& s);
  void append(std::string const& s);
  void append(Char const* s);
  void append(Char const* s, std::size_t n);
  void append(char const* s);
  void append(char const* s, std::size_t n);
  void append(Char c);

  void prepend(String const& s);
  void prepend(std::string const& s);
  void prepend(Char const* s);
  void prepend(Char const* s, std::size_t n);
  void prepend(char const* s);
  void prepend(char const* s, std::size_t n);
  void prepend(Char c);

  void push_back(Char c);
  void push_front(Char c);

  [[nodiscard]] auto contains(String const& s, CaseSensitivity cs = CaseSensitive) const -> bool;

  // Does this string match the given regular expression?
  [[nodiscard]] auto regexMatch(String const& regex, bool full = true, bool caseSensitive = true) const -> bool;

  [[nodiscard]] auto compare(String const& s, CaseSensitivity cs = CaseSensitive) const -> int;
  [[nodiscard]] auto equals(String const& s, CaseSensitivity cs = CaseSensitive) const -> bool;
  // Synonym for equals(s, String::CaseInsensitive)
  [[nodiscard]] auto equalsIgnoreCase(String const& s) const -> bool;

  [[nodiscard]] auto substr(std::size_t position, std::size_t n = std::numeric_limits<std::size_t>::max()) const -> String;
  void erase(std::size_t pos = 0, std::size_t n = std::numeric_limits<std::size_t>::max());

  [[nodiscard]] auto padLeft(std::size_t size, String const& filler) const -> String;
  [[nodiscard]] auto padRight(std::size_t size, String const& filler) const -> String;

  // Replace angle bracket tags in the string with values given by the given
  // lookup function.  Will be called as:
  // String lookup(String const& key);
  template <typename Lookup>
  auto lookupTags(Lookup&& lookup) const -> String;

  // StringView variant
  template <typename Lookup>
  auto maybeLookupTagsView(Lookup&& lookup) const -> std::optional<String>;

  template <typename Lookup>
  auto lookupTagsView(Lookup&& lookup) const -> String;

  // Replace angle bracket tags in the string with values given by the tags
  // map.  If replaceWithDefault is true, then values that are not found in the
  // tags map are replace with the default string.  If replaceWithDefault is
  // false, tags that are not found are not replaced at all.
  template <typename MapType>
  auto replaceTags(MapType const& tags, bool replaceWithDefault = false, String defaultValue = "") const -> String;

  auto operator=(String const& s) -> String&;
  auto operator=(String&& s) -> String&;

  auto operator+=(String const& s) -> String&;
  auto operator+=(std::string const& s) -> String&;
  auto operator+=(Char const* s) -> String&;
  auto operator+=(char const* s) -> String&;
  auto operator+=(Char c) -> String&;

  friend auto operator==(String const& s1, String const& s2) -> bool;
  friend auto operator==(String const& s1, std::string const& s2) -> bool;
  friend auto operator==(String const& s1, Char const* s2) -> bool;
  friend auto operator==(String const& s1, char const* s2) -> bool;
  friend auto operator==(std::string const& s1, String const& s2) -> bool;
  friend auto operator==(Char const* s1, String const& s2) -> bool;
  friend auto operator==(char const* s1, String const& s2) -> bool;

  friend auto operator!=(String const& s1, String const& s2) -> bool;
  friend auto operator!=(String const& s1, std::string const& s2) -> bool;
  friend auto operator!=(String const& s1, Char const* s2) -> bool;
  friend auto operator!=(String const& s1, char const* c) -> bool;
  friend auto operator!=(std::string const& s1, String const& s2) -> bool;
  friend auto operator!=(Char const* s1, String const& s2) -> bool;
  friend auto operator!=(char const* s1, String const& s2) -> bool;

  friend auto operator<(String const& s1, String const& s2) -> bool;
  friend auto operator<(String const& s1, std::string const& s2) -> bool;
  friend auto operator<(String const& s1, Char const* s2) -> bool;
  friend auto operator<(String const& s1, char const* s2) -> bool;
  friend auto operator<(std::string const& s1, String const& s2) -> bool;
  friend auto operator<(Char const* s1, String const& s2) -> bool;
  friend auto operator<(char const* s1, String const& s2) -> bool;

  friend auto operator+(String s1, String const& s2) -> String;
  friend auto operator+(String s1, std::string const& s2) -> String;
  friend auto operator+(String s1, Char const* s2) -> String;
  friend auto operator+(String s1, char const* s2) -> String;
  friend auto operator+(std::string const& s1, String const& s2) -> String;
  friend auto operator+(Char const* s1, String const& s2) -> String;
  friend auto operator+(char const* s1, String const& s2) -> String;

  friend auto operator+(String s, Char c) -> String;
  friend auto operator+(Char c, String const& s) -> String;

  friend auto operator*(String const& s, unsigned times) -> String;
  friend auto operator*(unsigned times, String const& s) -> String;

  friend auto operator<<(std::ostream& os, String const& s) -> std::ostream&;
  friend auto operator>>(std::istream& is, String& s) -> std::istream&;

  // String view functions
  String(StringView s);
  String(std::string_view s);

  auto operator+=(StringView s) -> String&;
  auto operator+=(std::string_view s) -> String&;

private:
  [[nodiscard]] auto compare(std::size_t selfOffset,
                             std::size_t selfLen,
                             String const& other,
                             std::size_t otherOffset,
                             std::size_t otherLen,
                             CaseSensitivity cs) const -> int;

  std::string m_string;
};

class StringList : public List<String> {
public:
  using Base = List<String>;

  using iterator = Base::iterator;
  using const_iterator = Base::const_iterator;
  using value_type = Base::value_type;
  using reference = Base::reference;
  using const_reference = Base::const_reference;

  template <typename Container>
  static auto from(Container const& m) -> StringList;

  StringList();
  StringList(Base const& l);
  StringList(Base&& l);
  StringList(StringList const& l);
  StringList(StringList&& l);
  StringList(std::size_t len, String::Char const* const* list);
  StringList(std::size_t len, char const* const* list);
  explicit StringList(std::size_t len, String const& s1 = String());
  StringList(std::initializer_list<String> list);

  template <typename InputIterator>
  StringList(InputIterator beg, InputIterator end)
      : Base(beg, end) {}

  auto operator=(Base const& rhs) -> StringList&;
  auto operator=(Base&& rhs) -> StringList&;
  auto operator=(StringList const& rhs) -> StringList&;
  auto operator=(StringList&& rhs) -> StringList&;
  auto operator=(std::initializer_list<String> list) -> StringList&;

  [[nodiscard]] auto contains(String const& s, String::CaseSensitivity cs = String::CaseSensitive) const -> bool;
  [[nodiscard]] auto trimAll(String const& chars = "") const -> StringList;
  [[nodiscard]] auto join(String const& separator = "") const -> String;

  [[nodiscard]] auto slice(SliceIndex a = SliceIndex(), SliceIndex b = SliceIndex(), int i = 1) const -> StringList;

  template <typename Filter>
  auto filtered(Filter&& filter) const -> StringList;

  template <typename Comparator>
  auto sorted(Comparator&& comparator) const -> StringList;

  [[nodiscard]] auto sorted() const -> StringList;
};

auto operator<<(std::ostream& os, StringList const& list) -> std::ostream&;

template <>
struct hash<String> {
  auto operator()(String const& s) const -> std::size_t;
};

struct CaseInsensitiveStringHash {
  auto operator()(String const& s) const -> std::size_t;
};

struct CaseInsensitiveStringCompare {
  auto operator()(String const& lhs, String const& rhs) const -> bool;
};

using StringSet = HashSet<String>;

using CaseInsensitiveStringSet = HashSet<String, CaseInsensitiveStringHash, CaseInsensitiveStringCompare>;

template <typename MappedT, typename HashT = hash<String>, typename ComparatorT = std::equal_to<String>>
using StringMap = HashMap<String, MappedT, HashT, ComparatorT>;

template <typename MappedT, typename HashT = hash<String>, typename ComparatorT = std::equal_to<String>>
using StableStringMap = StableHashMap<String, MappedT, HashT, ComparatorT>;

template <typename MappedT>
using CaseInsensitiveStringMap = StringMap<MappedT, CaseInsensitiveStringHash, CaseInsensitiveStringCompare>;

template <>
struct hash<StringList> {
  auto operator()(StringList const& s) const -> std::size_t;
};

template <typename... StringType>
auto String::joinWith(
  String const& join, String const& first, String const& second, String const& third, StringType const&... rest) -> String {
  return joinWith(join, joinWith(join, first, second), third, rest...);
}

template <typename Lookup>
auto String::lookupTags(Lookup&& lookup) const -> String {
  // Operates directly on the utf8 representation of the strings, rather than
  // using unicode find / replace methods

  auto substrInto = [](std::string const& ref, std::size_t position, std::size_t n, std::string& result) -> auto {
    auto len = ref.size();
    if (position > len)
      throw OutOfRangeException(strf("out of range in substrInto: {}", position));

    auto it = ref.begin();
    std::advance(it, position);

    for (std::size_t i = 0; i < n; ++i) {
      if (it == ref.end())
        break;
      result.push_back(*it);
      ++it;
    }
  };

  std::string finalString;

  std::size_t start = 0;
  std::size_t size = String::size();

  finalString.reserve(size);

  String key;

  while (true) {
    if (start >= size)
      break;

    std::size_t beginTag = m_string.find("<", start);
    std::size_t endTag = m_string.find(">", beginTag);
    if (beginTag != std::numeric_limits<std::size_t>::max() && endTag != std::numeric_limits<std::size_t>::max()) {
      substrInto(m_string, beginTag + 1, endTag - beginTag - 1, key.m_string);
      substrInto(m_string, start, beginTag - start, finalString);
      finalString += lookup(key).m_string;
      key.m_string.clear();
      start = endTag + 1;

    } else {
      substrInto(m_string, start, std::numeric_limits<std::size_t>::max(), finalString);
      break;
    }
  }

  return finalString;
}

template <typename Lookup>
auto String::maybeLookupTagsView(Lookup&& lookup) const -> std::optional<String> {
  List<std::string_view> finalViews = {};
  std::string_view view(utf8());

  std::size_t start = 0;
  while (true) {
    if (start >= view.size())
      break;

    std::size_t beginTag = view.find_first_of('<', start);
    if (beginTag == std::numeric_limits<std::size_t>::max() && !start)
      return std::nullopt;

    std::size_t endTag = view.find_first_of('>', beginTag);
    if (beginTag != std::numeric_limits<std::size_t>::max() && endTag != std::numeric_limits<std::size_t>::max()) {
      finalViews.append(view.substr(start, beginTag - start));
      finalViews.append(lookup(view.substr(beginTag + 1, endTag - beginTag - 1)).takeUtf8());
      start = endTag + 1;
    } else {
      finalViews.append(view.substr(start));
      break;
    }
  }

  std::string finalString;
  std::size_t finalSize = 0;
  for (auto& view : finalViews)
    finalSize += view.size();

  finalString.reserve(finalSize);

  for (auto& view : finalViews)
    finalString += view;

  return String(finalString);
}

template <typename Lookup>
auto String::lookupTagsView(Lookup&& lookup) const -> String {
  auto result = maybeLookupTagsView(lookup);
  return result ? std::move(*result) : String();
}

template <typename MapType>
auto String::replaceTags(MapType const& tags, bool replaceWithDefault, String defaultValue) const -> String {
  return lookupTags([&](String const& key) -> String {
    auto i = tags.find(key);
    if (i == tags.end()) {
      if (replaceWithDefault)
        return defaultValue;
      else
        return "<" + key + ">";
    } else {
      return i->second;
    }
  });
}

inline auto hash<String>::operator()(String const& s) const -> std::size_t {
  PLHasher hash;
  for (auto c : s.utf8())
    hash.put(c);
  return hash.hash();
}

template <typename Container>
auto StringList::from(Container const& m) -> StringList {
  return StringList(m.begin(), m.end());
}

template <typename Filter>
auto StringList::filtered(Filter&& filter) const -> StringList {
  StringList l;
  l.filter(forward<Filter>(filter));
  return l;
}

template <typename Comparator>
auto StringList::sorted(Comparator&& comparator) const -> StringList {
  StringList l;
  l.sort(forward<Comparator>(comparator));
  return l;
}

}// namespace Star

template <>
struct std::formatter<Star::String> : std::formatter<std::string> {
  auto format(Star::String const& s, std::format_context& ctx) const {
    return std::formatter<std::string>::format(s.utf8(), ctx);
  }
};

template <>
struct std::formatter<Star::StringList> : Star::ostream_formatter {};
