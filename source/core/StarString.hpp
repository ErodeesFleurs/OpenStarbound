#pragma once

#include "StarUnicode.hpp"
#include "StarUtf8.hpp"
#include "StarHash.hpp"
#include "StarByteArray.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
#include "StarFormat.hpp"

namespace Star {

class StringList;
class String;
using StringConstPtr = SharedPtr<String const>;
class StringView;

struct StringExceptionTag { static constexpr char const* typeName = "StringException"; };
using StringException = TypedException<StarException, StringExceptionTag>;

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

  // Delegates to Star::CaseSensitivity (defined in StarUtf8.hpp)
  using CaseSensitivity = Star::CaseSensitivity;

  static constexpr auto CaseSensitive = Star::CaseSensitivity::CaseSensitive;
  static constexpr auto CaseInsensitive = Star::CaseSensitivity::CaseInsensitive;

  // Space, horizontal tab, newline, carriage return, and BOM / ZWNBSP
  static bool isSpace(Char c) { return Star::isSpace(c); }
  static bool isAsciiNumber(Char c) { return Star::isAsciiNumber(c); }
  static bool isAsciiLetter(Char c) { return Star::isAsciiLetter(c); }

  // These methods only actually work on unicode characters below 127, i.e.
  // ASCII subset.
  static Char toLower(Char c) { return Star::toLower(c); }
  static Char toUpper(Char c) { return Star::toUpper(c); }
  static bool charEqual(Char c1, Char c2, CaseSensitivity cs) { return Star::charEqual(c1, c2, cs); }

  // Join two strings together with a joiner, so that only one instance of the
  // joiner is in between the left and right strings.  For example, joins "foo"
  // and "bar" with "?" to produce "foo?bar".  Gets rid of repeat joiners, so
  // "foo?" and "?bar" with "?" also becomes "foo?bar".  Also, if left or right
  // is empty, does not add a joiner, for example "" and "baz" joined with "?"
  // produces "baz".
  [[nodiscard]] static String joinWith(String const& join, String const& left, String const& right);
  template <typename... StringType>
  [[nodiscard]] static String joinWith(String const& join, String const& first, String const& second, String const& third, StringType const&... rest);

  String();
  String(String const& s);
  String(String&& s) noexcept;

  // These assume utf8 input
  String(char const* s);
  String(char const* s, size_t n);
  String(std::string const& s);
  String(std::string&& s) noexcept;

  String(std::wstring const& s);
  String(Char const* s);
  String(Char const* s, size_t n);
  String(Char c, size_t n);

  explicit String(Char c);

  // const& to internal utf8 data
  std::string const& utf8() const;
  std::string takeUtf8();
  [[nodiscard]] ByteArray utf8Bytes() const;
  // Pointer to internal utf8 data, null-terminated.
  [[nodiscard]] char const* utf8Ptr() const;
  [[nodiscard]] size_t utf8Size() const;

  [[nodiscard]] std::wstring wstring() const;
  [[nodiscard]] WideString wideString() const;

  [[nodiscard]] const_iterator begin() const;
  [[nodiscard]] const_iterator end() const;

  [[nodiscard]] size_t size() const;
  [[nodiscard]] size_t length() const;

  void clear();
  void reserve(size_t n);
  [[nodiscard]] bool empty() const;

  [[nodiscard]] Char operator[](size_t i) const;
  // Throws StringException if i out of range.
  [[nodiscard]] Char at(size_t i) const;

  [[nodiscard]] String toUpper() const;
  [[nodiscard]] String toLower() const;
  [[nodiscard]] String titleCase() const;

  [[nodiscard]] bool endsWith(String const& end, CaseSensitivity cs = CaseSensitive) const;
  [[nodiscard]] bool endsWith(Char end, CaseSensitivity cs = CaseSensitive) const;
  [[nodiscard]] bool beginsWith(String const& beg, CaseSensitivity cs = CaseSensitive) const;
  [[nodiscard]] bool beginsWith(Char beg, CaseSensitivity cs = CaseSensitive) const;

  [[nodiscard]] String reverse() const;

  [[nodiscard]] String rot13() const;

  [[nodiscard]] StringList split(Char c, size_t maxSplit = NPos) const;
  [[nodiscard]] StringList split(String const& pattern, size_t maxSplit = NPos) const;
  [[nodiscard]] StringList rsplit(Char c, size_t maxSplit = NPos) const;
  [[nodiscard]] StringList rsplit(String const& pattern, size_t maxSplit = NPos) const;

  // Splits on any number of contiguous instances of any of the given
  // characters.  Behaves differently than regular split in that leading and
  // trailing instances of the characters are also ignored, and in general no
  // empty strings will be in the resulting split list.  If chars is empty,
  // then splits on any whitespace.
  [[nodiscard]] StringList splitAny(String const& chars = "", size_t maxSplit = NPos) const;
  [[nodiscard]] StringList rsplitAny(String const& chars = "", size_t maxSplit = NPos) const;

  // Split any with '\n\r'
  [[nodiscard]] StringList splitLines(size_t maxSplit = NPos) const;
  // Shorthand for splitAny("");
  [[nodiscard]] StringList splitWhitespace(size_t maxSplit = NPos) const;

  // Splits a string once based on the given characters (defaulting to
  // whitespace), and returns the first part.  This string is set to the
  // second part.
  [[nodiscard]] String extract(String const& chars = "");
  [[nodiscard]] String rextract(String const& chars = "");

  [[nodiscard]] bool hasChar(Char c) const;
  // Identical to hasChar, except, if string is empty, tests if c is
  // whitespace.
  [[nodiscard]] bool hasCharOrWhitespace(Char c) const;

  [[nodiscard]] String replace(String const& rplc, String const& val, CaseSensitivity cs = CaseSensitive) const;

  [[nodiscard]] String trimEnd(String const& chars = "") const;
  [[nodiscard]] String trimBeg(String const& chars = "") const;
  [[nodiscard]] String trim(String const& chars = "") const;

  [[nodiscard]] size_t find(Char c, size_t beg = 0, CaseSensitivity cs = CaseSensitive) const;
  [[nodiscard]] size_t find(String const& s, size_t beg = 0, CaseSensitivity cs = CaseSensitive) const;
  [[nodiscard]] size_t findLast(Char c, CaseSensitivity cs = CaseSensitive) const;
  [[nodiscard]] size_t findLast(String const& s, CaseSensitivity cs = CaseSensitive) const;

  // If pattern is empty, finds first whitespace
  [[nodiscard]] size_t findFirstOf(String const& chars = "", size_t beg = 0) const;

  // If pattern is empty, finds first non-whitespace
  [[nodiscard]] size_t findFirstNotOf(String const& chars = "", size_t beg = 0) const;

  // finds the the start of the next 'boundary' in a string.  used for quickly
  // scanning a string
  [[nodiscard]] size_t findNextBoundary(size_t index, bool backwards = false) const;

  [[nodiscard]] String slice(SliceIndex a = SliceIndex(), SliceIndex b = SliceIndex(), int i = 1) const;

  void append(String const& s);
  void append(std::string const& s);
  void append(Char const* s);
  void append(Char const* s, size_t n);
  void append(char const* s);
  void append(char const* s, size_t n);
  void append(Char c);

  void prepend(String const& s);
  void prepend(std::string const& s);
  void prepend(Char const* s);
  void prepend(Char const* s, size_t n);
  void prepend(char const* s);
  void prepend(char const* s, size_t n);
  void prepend(Char c);

  void push_back(Char c);
  void push_front(Char c);

  [[nodiscard]] bool contains(String const& s, CaseSensitivity cs = CaseSensitive) const;

  // Does this string match the given regular expression?
  [[nodiscard]] bool regexMatch(String const& regex, bool full = true, bool caseSensitive = true) const;

  [[nodiscard]] int compare(String const& s, CaseSensitivity cs = CaseSensitive) const;
  [[nodiscard]] bool equals(String const& s, CaseSensitivity cs = CaseSensitive) const;
  // Synonym for equals(s, String::CaseInsensitive)
  [[nodiscard]] bool equalsIgnoreCase(String const& s) const;

  [[nodiscard]] String substr(size_t position, size_t n = NPos) const;
  void erase(size_t pos = 0, size_t n = NPos);

  [[nodiscard]] String padLeft(size_t size, String const& filler) const;
  [[nodiscard]] String padRight(size_t size, String const& filler) const;

  // Replace angle bracket tags in the string with values given by the given
  // lookup function.  Will be called as:
  // String lookup(String const& key);
  template <typename Lookup>
  [[nodiscard]] String lookupTags(Lookup&& lookup) const;

  // StringView variant
  template <typename Lookup>
  [[nodiscard]] Maybe<String> maybeLookupTagsView(Lookup&& lookup) const;

  template <typename Lookup>
  [[nodiscard]] String lookupTagsView(Lookup&& lookup) const;

  // Replace angle bracket tags in the string with values given by the tags
  // map.  If replaceWithDefault is true, then values that are not found in the
  // tags map are replace with the default string.  If replaceWithDefault is
  // false, tags that are not found are not replaced at all.
  template <typename MapType>
  [[nodiscard]] String replaceTags(MapType const& tags, bool replaceWithDefault = false, String defaultValue = "") const;

  String& operator=(String const& s);
  String& operator=(String&& s) noexcept;

  String& operator+=(String const& s);
  String& operator+=(std::string const& s);
  String& operator+=(Char const* s);
  String& operator+=(char const* s);
  String& operator+=(Char c);

  friend bool operator==(String const& s1, String const& s2);
  friend bool operator==(String const& s1, std::string const& s2);
  friend bool operator==(String const& s1, Char const* s2);
  friend bool operator==(String const& s1, char const* s2);
  friend bool operator==(std::string const& s1, String const& s2);
  friend bool operator==(Char const* s1, String const& s2);
  friend bool operator==(char const* s1, String const& s2);

  friend bool operator!=(String const& s1, String const& s2);
  friend bool operator!=(String const& s1, std::string const& s2);
  friend bool operator!=(String const& s1, Char const* s2);
  friend bool operator!=(String const& s1, char const* c);
  friend bool operator!=(std::string const& s1, String const& s2);
  friend bool operator!=(Char const* s1, String const& s2);
  friend bool operator!=(char const* s1, String const& s2);

  friend bool operator<(String const& s1, String const& s2);
  friend bool operator<(String const& s1, std::string const& s2);
  friend bool operator<(String const& s1, Char const* s2);
  friend bool operator<(String const& s1, char const* s2);
  friend bool operator<(std::string const& s1, String const& s2);
  friend bool operator<(Char const* s1, String const& s2);
  friend bool operator<(char const* s1, String const& s2);

  friend String operator+(String s1, String const& s2);
  friend String operator+(String s1, std::string const& s2);
  friend String operator+(String s1, Char const* s2);
  friend String operator+(String s1, char const* s2);
  friend String operator+(std::string const& s1, String const& s2);
  friend String operator+(Char const* s1, String const& s2);
  friend String operator+(char const* s1, String const& s2);

  friend String operator+(String s, Char c);
  friend String operator+(Char c, String const& s);

  friend String operator*(String const& s, unsigned times);
  friend String operator*(unsigned times, String const& s);

  friend std::ostream& operator<<(std::ostream& os, String const& s);
  friend std::istream& operator>>(std::istream& is, String& s);

  // String view functions
  String(StringView s);
  String(std::string_view s);

  String& operator+=(StringView s);
  String& operator+=(std::string_view s);

private:
  int compare(size_t selfOffset,
      size_t selfLen,
      String const& other,
      size_t otherOffset,
      size_t otherLen,
      CaseSensitivity cs) const;

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
  [[nodiscard]] static StringList from(Container const& m);

  StringList();
  StringList(Base const& l);
  StringList(Base&& l) noexcept;
  StringList(StringList const& l);
  StringList(StringList&& l) noexcept;
  StringList(size_t len, String::Char const* const* list);
  StringList(size_t len, char const* const* list);
  explicit StringList(size_t len, String const& s1 = String());
  StringList(std::initializer_list<String> list);

  template <typename InputIterator>
  StringList(InputIterator beg, InputIterator end)
    : Base(beg, end) {}

  StringList& operator=(Base const& rhs);
  StringList& operator=(Base&& rhs) noexcept;
  StringList& operator=(StringList const& rhs);
  StringList& operator=(StringList&& rhs) noexcept;
  StringList& operator=(initializer_list<String> list);

  [[nodiscard]] bool contains(String const& s, String::CaseSensitivity cs = String::CaseSensitive) const;
  [[nodiscard]] StringList trimAll(String const& chars = "") const;
  [[nodiscard]] String join(String const& separator = "") const;

  [[nodiscard]] StringList slice(SliceIndex a = SliceIndex(), SliceIndex b = SliceIndex(), int i = 1) const;

  template <typename Filter>
  [[nodiscard]] StringList filtered(Filter&& filter) const;

  template <typename Comparator>
  [[nodiscard]] StringList sorted(Comparator&& comparator) const;

  [[nodiscard]] StringList sorted() const;
};

std::ostream& operator<<(std::ostream& os, StringList const& list);

template <>
struct hash<String> {
  [[nodiscard]] size_t operator()(String const& s) const;
};

struct CaseInsensitiveStringHash {
  [[nodiscard]] size_t operator()(String const& s) const;
};

struct CaseInsensitiveStringCompare {
  [[nodiscard]] bool operator()(String const& lhs, String const& rhs) const;
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
  [[nodiscard]] size_t operator()(StringList const& s) const;
};

template <typename... StringType>
String String::joinWith(
    String const& join, String const& first, String const& second, String const& third, StringType const&... rest) {
  return joinWith(join, joinWith(join, first, second), third, rest...);
}

template <typename Lookup>
String String::lookupTags(Lookup&& lookup) const {
  // Operates directly on the utf8 representation of the strings, rather than
  // using unicode find / replace methods

  auto substrInto = [](std::string const& ref, size_t position, size_t n, std::string& result) {
    auto len = ref.size();
    if (position > len)
      throw OutOfRangeException(strf("out of range in substrInto: {}", position));

    auto it = ref.begin();
    std::advance(it, position);

    for (size_t i = 0; i < n; ++i) {
      if (it == ref.end())
        break;
      result.push_back(*it);
      ++it;
    }
  };

  std::string finalString;

  size_t start = 0;
  size_t size = String::size();

  finalString.reserve(size);

  String key;

  while (true) {
    if (start >= size)
      break;

    size_t beginTag = m_string.find("<", start);
    size_t endTag = m_string.find(">", beginTag);
    if (beginTag != NPos && endTag != NPos) {
      substrInto(m_string, beginTag + 1, endTag - beginTag - 1, key.m_string);
      substrInto(m_string, start, beginTag - start, finalString);
      finalString += lookup(key).m_string;
      key.m_string.clear();
      start = endTag + 1;

    } else {
      substrInto(m_string, start, NPos, finalString);
      break;
    }
  }

  return finalString;
}

template <typename Lookup>
Maybe<String> String::maybeLookupTagsView(Lookup&& lookup) const {
  List<std::string_view> finalViews = {};
  std::string_view view(utf8());

  size_t start = 0;
  while (true) {
    if (start >= view.size())
      break;

    size_t beginTag = view.find_first_of('<', start);
    if (beginTag == NPos && !start)
      return Maybe<String>();

    size_t endTag = view.find_first_of('>', beginTag);
    if (beginTag != NPos && endTag != NPos) {
      finalViews.append(view.substr(start, beginTag - start));
      finalViews.append(lookup(view.substr(beginTag + 1, endTag - beginTag - 1)).takeUtf8());
      start = endTag + 1;
    } else {
      finalViews.append(view.substr(start));
      break;
    }
  }

  std::string finalString;
  size_t finalSize = 0;
  for (auto& sv : finalViews)
    finalSize += sv.size();

  finalString.reserve(finalSize);

  for (auto& sv : finalViews)
    finalString += sv;

  return String(finalString);
}

template <typename Lookup>
String String::lookupTagsView(Lookup&& lookup) const {
  auto result = maybeLookupTagsView(lookup);
  return result ? std::move(result.take()) : String();
}

template <typename MapType>
String String::replaceTags(MapType const& tags, bool replaceWithDefault, String defaultValue) const {
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

inline size_t hash<String>::operator()(String const& s) const {
  PLHasher hash;
  for (auto c : s.utf8())
    hash.put(c);
  return hash.hash();
}

template <typename Container>
StringList StringList::from(Container const& m) {
  return StringList(m.begin(), m.end());
}

template <typename Filter>
StringList StringList::filtered(Filter&& filter) const {
  StringList l;
  l.filter(forward<Filter>(filter));
  return l;
}

template <typename Comparator>
StringList StringList::sorted(Comparator&& comparator) const {
  StringList l;
  l.sort(forward<Comparator>(comparator));
  return l;
}

}

template <> struct std::formatter<Star::StringList> : Star::OstreamFormatter {};
