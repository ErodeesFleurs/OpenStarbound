#include "StarString.hpp"

import re2_module;
import std;

namespace Star {

auto String::isSpace(Char c) -> bool {
  return c == 0x20 ||// space
    c == 0x09 ||     // horizontal tab
    c == 0x0a ||     // newline
    c == 0x0d ||     // carriage return
    c == 0xfeff;     // BOM or ZWNBSP
}

auto String::isAsciiNumber(Char c) -> bool {
  return c >= '0' && c <= '9';
}

auto String::isAsciiLetter(Char c) -> bool {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

auto String::toLower(Char c) -> String::Char {
  if (c >= 'A' && c <= 'Z')
    return c + 32;
  else
    return c;
}

auto String::toUpper(Char c) -> String::Char {
  if (c >= 'a' && c <= 'z')
    return c - 32;
  else
    return c;
}

auto String::charEqual(Char c1, Char c2, CaseSensitivity cs) -> bool {
  if (cs == CaseInsensitive)
    return toLower(c1) == toLower(c2);
  else
    return c1 == c2;
}

auto String::joinWith(String const& join, String const& left, String const& right) -> String {
  if (left.empty())
    return right;
  if (right.empty())
    return left;

  if (left.endsWith(join)) {
    if (right.beginsWith(join)) {
      return left + right.substr(join.size());
    }
    return left + right;
  } else {
    if (right.beginsWith(join)) {
      return left + right;
    }
    return left + join + right;
  }
}

String::String() = default;
String::String(String const& s) = default;
String::String(String&& s) : m_string(std::move(s.m_string)) {}
String::String(char const* s) : m_string(s) {}
String::String(char const* s, std::size_t n) : m_string(s, n) {}
String::String(std::string const& s) : m_string(s) {}
String::String(std::string&& s) : m_string(std::move(s)) {}

String::String(std::wstring const& s) {
  reserve(s.length());
  for (Char c : s)
    append(c);
}

String::String(Char const* s) {
  while (*s) {
    append(*s);
    ++s;
  }
}

String::String(Char const* s, std::size_t n) {
  reserve(n);
  for (std::size_t idx = 0; idx < n; ++idx) {
    append(*s);
    ++s;
  }
}

String::String(Char c, std::size_t n) {
  reserve(n);
  for (std::size_t i = 0; i < n; ++i)
    append(c);
}

String::String(Char c) {
  append(c);
}

auto String::utf8() const -> std::string const& {
  return m_string;
}

auto String::takeUtf8() -> std::string {
  return take(m_string);
}

auto String::utf8Bytes() const -> ByteArray {
  return {m_string.c_str(), m_string.size()};
}

auto String::utf8Ptr() const -> char const* {
  return m_string.c_str();
}

auto String::utf8Size() const -> std::size_t {
  return m_string.size();
}

auto String::wstring() const -> std::wstring {
  std::wstring string;
  for (Char c : *this)
    string.push_back(c);
  return string;
}

auto String::wideString() const -> String::WideString {
  WideString string;
  string.reserve(m_string.size());
  for (Char c : *this)
    string.push_back(c);
  return string;
}

auto String::begin() const -> String::const_iterator {
  return {m_string.begin()};
}

auto String::end() const -> String::const_iterator {
  return {m_string.end()};
}

auto String::size() const -> std::size_t {
  return utf8Length(m_string.c_str(), m_string.size());
}

auto String::length() const -> std::size_t {
  return size();
}

void String::clear() {
  m_string.clear();
}

void String::reserve(std::size_t n) {
  m_string.reserve(n);
}

auto String::empty() const -> bool {
  return m_string.empty();
}

auto String::operator[](std::size_t index) const -> String::Char {
  auto it = begin();
  for (std::size_t i = 0; i < index; ++i)
    ++it;
  return *it;
}

auto CaseInsensitiveStringHash::operator()(String const& s) const -> std::size_t {
  PLHasher hash;
  for (auto c : s)
    hash.put(String::toLower(c));
  return hash.hash();
}

auto CaseInsensitiveStringCompare::operator()(String const& lhs, String const& rhs) const -> bool {
  return lhs.equalsIgnoreCase(rhs);
}

auto String::at(std::size_t i) const -> String::Char {
  if (i > size())
    throw OutOfRangeException(strf("Out of range in String::at({})", i));
  return operator[](i);
}

auto String::toUpper() const -> String {
  String s;
  s.reserve(m_string.length());
  for (Char c : *this)
    s.append(toUpper(c));
  return s;
}

auto String::toLower() const -> String {
  String s;
  s.reserve(m_string.length());
  for (Char c : *this)
    s.append(toLower(c));
  return s;
}

auto String::titleCase() const -> String {
  String s;
  s.reserve(m_string.length());
  bool capNext = true;
  for (Char c : *this) {
    if (capNext)
      s.append(toUpper(c));
    else
      s.append(toLower(c));
    capNext = !std::isalpha(c);
  }
  return s;
}

auto String::endsWith(String const& end, CaseSensitivity cs) const -> bool {
  auto endsize = end.size();
  if (endsize == 0)
    return true;

  auto mysize = size();
  if (endsize > mysize)
    return false;

  return compare(mysize - endsize, std::numeric_limits<std::size_t>::max(), end, 0, std::numeric_limits<std::size_t>::max(), cs) == 0;
}

auto String::endsWith(Char end, CaseSensitivity cs) const -> bool {
  if (size() == 0)
    return false;

  return charEqual(end, operator[](size() - 1), cs);
}

auto String::beginsWith(String const& beg, CaseSensitivity cs) const -> bool {
  auto begSize = beg.size();
  if (begSize == 0)
    return true;

  auto mysize = size();
  if (begSize > mysize)
    return false;

  return compare(0, begSize, beg, 0, std::numeric_limits<std::size_t>::max(), cs) == 0;
}

auto String::beginsWith(Char beg, CaseSensitivity cs) const -> bool {
  if (size() == 0)
    return false;

  return charEqual(beg, operator[](0), cs);
}

auto String::reverse() const -> String {
  String ret;
  ret.reserve(m_string.length());
  auto i = end();
  while (i != begin()) {
    --i;
    ret.append(*i);
  }
  return ret;
}

auto String::rot13() const -> String {
  String ret;
  ret.reserve(m_string.length());
  for (auto c : *this) {
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
      c += 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
      c -= 13;
    ret.append(c);
  }
  return ret;
}

auto String::split(Char c, std::size_t maxSplit) const -> StringList {
  return split(String(c), maxSplit);
}

auto String::split(String const& pattern, std::size_t maxSplit) const -> StringList {
  StringList ret;
  if (pattern.empty())
    return StringList(1, *this);

  std::size_t beg = 0;
  while (true) {
    if (ret.size() == maxSplit) {
      ret.append(m_string.substr(beg));
      break;
    }

    std::size_t end = m_string.find(pattern.m_string, beg);
    if (end == std::numeric_limits<std::size_t>::max()) {
      ret.append(m_string.substr(beg));
      break;
    }
    ret.append(m_string.substr(beg, end - beg));
    beg = end + pattern.m_string.size();
  }

  return ret;
}

auto String::rsplit(Char c, std::size_t maxSplit) const -> StringList {
  return rsplitAny(String(c), maxSplit);
}

auto String::rsplit(String const& pattern, std::size_t maxSplit) const -> StringList {
  // This is really inefficient!
  String v = reverse();
  String p = pattern.reverse();
  StringList l = v.split(p, maxSplit);
  for (auto& s : l)
    s = s.reverse();

  Star::reverse(l);
  return l;
}

auto String::splitAny(String const& chars, std::size_t maxSplit) const -> StringList {
  StringList ret;
  String next;
  bool doneSplitting = false;
  for (auto c : *this) {
    if (!doneSplitting && chars.hasCharOrWhitespace(c)) {
      if (!next.empty())
        ret.append(take(next));
    } else {
      if (ret.size() == maxSplit)
        doneSplitting = true;
      next.append(c);
    }
  }
  if (!next.empty())
    ret.append(std::move(next));
  return ret;
}

auto String::rsplitAny(String const& chars, std::size_t maxSplit) const -> StringList {
  // This is really inefficient!
  String v = reverse();
  StringList l = v.splitAny(chars, maxSplit);
  for (auto& s : l)
    s = s.reverse();

  Star::reverse(l);
  return l;
}

auto String::splitLines(std::size_t maxSplit) const -> StringList {
  return splitAny("\r\n", maxSplit);
}

auto String::splitWhitespace(std::size_t maxSplit) const -> StringList {
  return splitAny("", maxSplit);
}

auto String::extract(String const& chars) -> String {
  StringList l = splitAny(chars, 1);
  if (l.size() == 0) {
    return {};
  } else if (l.size() == 1) {
    clear();
    return l.at(0);
  } else {
    *this = l.at(1);
    return l.at(0);
  }
}

auto String::rextract(String const& chars) -> String {
  if (empty())
    return {};

  StringList l = rsplitAny(chars, 1);
  if (l.size() == 1) {
    clear();
    return l.at(0);
  } else {
    *this = l.at(0);
    return l.at(1);
  }
}

auto String::hasChar(Char c) const -> bool {
  for (Char ch : *this)
    if (ch == c)
      return true;
  return false;
}

auto String::hasCharOrWhitespace(Char c) const -> bool {
  if (empty())
    return isSpace(c);
  else
    return hasChar(c);
}

auto String::replace(String const& rplc, String const& val, CaseSensitivity cs) const -> String {
  std::size_t index;
  std::size_t sz = size();
  std::size_t rsz = rplc.size();
  String ret;
  ret.reserve(m_string.length());

  if (rplc.empty())
    return *this;

  index = find(rplc, 0, cs);
  if (index == std::numeric_limits<std::size_t>::max())
    return *this;

  auto it = begin();
  for (std::size_t i = 0; i < index; ++i)
    ret.append(*it++);

  while (index < sz) {
    ret.append(val);
    index += rsz;
    for (std::size_t i = 0; i < rsz; ++i)
      ++it;

    std::size_t nindex = find(rplc, index, cs);
    for (std::size_t i = index; i < nindex && i < sz; ++i)
      ret.append(*it++);

    index = nindex;
  }
  return ret;
}

auto String::trimEnd(String const& pattern) const -> String {
  std::size_t end;
  for (end = size(); end > 0; --end) {
    Char ec = (*this)[end - 1];
    if (!pattern.hasCharOrWhitespace(ec))
      break;
  }
  return substr(0, end);
}

auto String::trimBeg(String const& pattern) const -> String {
  std::size_t beg;
  for (beg = 0; beg < size(); ++beg) {
    Char bc = (*this)[beg];
    if (!pattern.hasCharOrWhitespace(bc))
      break;
  }
  return substr(beg);
}

auto String::trim(String const& pattern) const -> String {
  return trimEnd(pattern).trimBeg(pattern);
}

auto String::find(Char c, std::size_t pos, CaseSensitivity cs) const -> std::size_t {
  auto it = begin();
  for (std::size_t i = 0; i < pos; ++i) {
    if (it == end())
      break;
    ++it;
  }

  while (it != end()) {
    if (charEqual(c, *it, cs))
      return pos;
    ++pos;
    ++it;
  }

  return std::numeric_limits<std::size_t>::max();
}

auto String::find(String const& str, std::size_t pos, CaseSensitivity cs) const -> std::size_t {
  if (str.empty())
    return 0;

  auto it = begin();
  for (std::size_t i = 0; i < pos; ++i) {
    if (it == end())
      break;
    ++it;
  }

  const_iterator sit = str.begin();
  const_iterator mit = it;
  while (it != end()) {
    if (charEqual(*sit, *mit, cs)) {
      do {
        ++mit;
        ++sit;
        if (sit == str.end())
          return pos;
        else if (mit == end())
          break;
      } while (charEqual(*sit, *mit, cs));
      sit = str.begin();
    }
    ++pos;
    mit = ++it;
  }

  return std::numeric_limits<std::size_t>::max();
}

auto String::findLast(Char c, CaseSensitivity cs) const -> std::size_t {
  auto it = begin();

  std::size_t found = std::numeric_limits<std::size_t>::max();
  std::size_t pos = 0;
  while (it != end()) {
    if (charEqual(c, *it, cs))
      found = pos;
    ++pos;
    ++it;
  }

  return found;
}

auto String::findLast(String const& str, CaseSensitivity cs) const -> std::size_t {
  if (str.empty())
    return 0;

  std::size_t pos = 0;
  auto it = begin();
  std::size_t result = std::numeric_limits<std::size_t>::max();
  const_iterator sit = str.begin();
  const_iterator mit = it;
  while (it != end()) {
    if (charEqual(*sit, *mit, cs)) {
      do {
        ++mit;
        ++sit;
        if (sit == str.end()) {
          result = pos;
          break;
        }
        if (mit == end())
          break;
      } while (charEqual(*sit, *mit, cs));
      sit = str.begin();
    }
    ++pos;
    mit = ++it;
  }

  return result;
}

auto String::findFirstOf(String const& pattern, std::size_t beg) const -> std::size_t {
  auto it = begin();
  std::size_t i;
  for (i = 0; i < beg; ++i)
    ++it;

  while (it != end()) {
    if (pattern.hasCharOrWhitespace(*it))
      return i;
    ++it;
    ++i;
  }
  return std::numeric_limits<std::size_t>::max();
}

auto String::findFirstNotOf(String const& pattern, std::size_t beg) const -> std::size_t {
  auto it = begin();
  std::size_t i;
  for (i = 0; i < beg; ++i)
    ++it;

  while (it != end()) {
    if (!pattern.hasCharOrWhitespace(*it))
      return i;
    ++it;
    ++i;
  }
  return std::numeric_limits<std::size_t>::max();
}

auto String::findNextBoundary(std::size_t index, bool backwards) const -> std::size_t {
  if (!backwards && (index == size()))
    return index;
  if (backwards) {
    if (index == 0)
      return 0;
    index--;
  }
  Char c = this->at(index);
  while (!isSpace(c)) {
    if (backwards && (index == 0))
      return 0;
    index += backwards ? -1 : 1;
    if (index == size())
      return size();
    c = this->at(index);
  }
  while (isSpace(c)) {
    if (backwards && (index == 0))
      return 0;
    index += backwards ? -1 : 1;
    if (index == size())
      return size();
    c = this->at(index);
  }
  if (backwards && !(index == size()))
    return index + 1;
  return index;
}

auto String::slice(SliceIndex a, SliceIndex b, int i) const -> String {
  auto wide = wideString();
  wide = Star::slice(wide, a, b, i);
  return {wide.c_str()};
}

void String::append(String const& string) {
  m_string.append(string.m_string);
}

void String::append(std::string const& s) {
  m_string.append(s);
}

void String::append(Char const* s) {
  while (*s)
    append(*s++);
}

void String::append(Char const* s, std::size_t n) {
  for (std::size_t i = 0; i < n; ++i)
    append(s[i]);
}

void String::append(char const* s) {
  m_string.append(s);
}

void String::append(char const* s, std::size_t n) {
  m_string.append(s, n);
}

void String::append(Char c) {
  std::array<char, 6> conv;
  std::size_t size = utf8EncodeChar(conv.data(), c, 6);
  append(conv.data(), size);
}

void String::prepend(String const& s) {
  auto ns = s;
  ns.append(*this);
  *this = std::move(ns);
}

void String::prepend(std::string const& s) {
  auto ns = String(s);
  ns.append(*this);
  *this = std::move(ns);
}

void String::prepend(Char const* s) {
  auto ns = String(s);
  ns.append(*this);
  *this = std::move(ns);
}

void String::prepend(Char const* s, std::size_t n) {
  auto ns = String(s, n);
  ns.append(*this);
  *this = std::move(ns);
}

void String::prepend(char const* s) {
  auto ns = String(s);
  ns.append(*this);
  *this = std::move(ns);
}

void String::prepend(char const* s, std::size_t n) {
  auto ns = String(s, n);
  ns.append(*this);
  *this = std::move(ns);
}

void String::prepend(Char c) {
  auto ns = String(c, 1);
  ns.append(*this);
  *this = std::move(ns);
}

void String::push_back(Char c) {
  append(c);
}

void String::push_front(Char c) {
  prepend(c);
}

auto String::contains(String const& s, CaseSensitivity cs) const -> bool {
  return find(s, 0, cs) != std::numeric_limits<std::size_t>::max();
}

auto String::regexMatch(String const& regex, bool full, bool caseSensitive) const -> bool {
  re2::RE2::Options options;
  options.set_case_sensitive(caseSensitive);
  re2::RE2 re(regex.utf8(), options);
  if (!re.ok())
    throw StringException::format("Invalid regex pattern '{}': {}", regex, re.error());

  return full ? re2::RE2::FullMatch(utf8(), re) : re2::RE2::PartialMatch(utf8(), re);
}

auto String::compare(String const& s, CaseSensitivity cs) const -> int {
  if (cs == CaseSensitive)
    return m_string.compare(s.m_string);
  else
    return compare(0, std::numeric_limits<std::size_t>::max(), s, 0, std::numeric_limits<std::size_t>::max(), cs);
}

auto String::equals(String const& s, CaseSensitivity cs) const -> bool {
  return compare(s, cs) == 0;
}

auto String::equalsIgnoreCase(String const& s) const -> bool {
  return compare(s, CaseInsensitive) == 0;
}

auto String::substr(std::size_t position, std::size_t n) const -> String {
  auto len = size();
  if (position > len)
    throw OutOfRangeException(strf("out of range in String::substr({}, {})", position, n));

  if (position == 0 && n >= len)
    return *this;

  String ret;
  ret.reserve(std::min(n, len - position));

  auto it = begin();
  std::advance(it, position);

  for (std::size_t i = 0; i < n; ++i) {
    if (it == end())
      break;
    ret.append(*it);
    ++it;
  }

  return ret;
}

void String::erase(std::size_t pos, std::size_t n) {
  String ns;
  ns.reserve(m_string.size() - std::min(n, m_string.size()));
  auto it = begin();
  for (std::size_t i = 0; i < pos; ++i)
    ns.append(*it++);
  for (std::size_t i = 0; i < n; ++i) {
    if (it == end())
      break;
    ++it;
  }
  while (it != end())
    ns.append(*it++);
  *this = ns;
}

auto String::padLeft(std::size_t size, String const& filler) const -> String {
  if (!filler.length())
    return *this;
  String rs;
  while (rs.length() + length() < size) {
    rs.append(filler);
  }
  if (rs.length())
    return rs + *this;
  return *this;
}

auto String::padRight(std::size_t size, String const& filler) const -> String {
  if (!filler.length())
    return *this;
  String rs = *this;
  while (rs.length() < size) {
    rs.append(filler);
  }
  return rs;
}

auto String::operator=(String const& s) -> String& = default;

auto String::operator=(String&& s) -> String& {
  m_string = std::move(s.m_string);
  return *this;
}

auto String::operator+=(String const& s) -> String& {
  append(s);
  return *this;
}

auto String::operator+=(std::string const& s) -> String& {
  append(s);
  return *this;
}

auto String::operator+=(Char const* s) -> String& {
  append(s);
  return *this;
}

auto String::operator+=(char const* s) -> String& {
  append(s);
  return *this;
}

auto String::operator+=(Char c) -> String& {
  append(c);
  return *this;
}

auto operator==(String const& s1, String const& s2) -> bool {
  return s1.m_string == s2.m_string;
}

auto operator==(String const& s1, std::string const& s2) -> bool {
  return s1.m_string == s2;
}

auto operator==(String const& s1, String::Char const* s2) -> bool {
  return s1 == String(s2);
}

auto operator==(String const& s1, char const* s2) -> bool {
  return s1.m_string == s2;
}

auto operator==(std::string const& s1, String const& s2) -> bool {
  return s1 == s2.m_string;
}

auto operator==(String::Char const* s1, String const& s2) -> bool {
  return String(s1) == s2;
}

auto operator==(char const* s1, String const& s2) -> bool {
  return s1 == s2.m_string;
}

auto operator!=(String const& s1, String const& s2) -> bool {
  return s1.m_string != s2.m_string;
}

auto operator!=(String const& s1, std::string const& s2) -> bool {
  return s1.m_string != s2;
}

auto operator!=(String const& s1, String::Char const* s2) -> bool {
  return s1 != String(s2);
}

auto operator!=(String const& s1, char const* s2) -> bool {
  return s1.m_string != s2;
}

auto operator!=(std::string const& s1, String const& s2) -> bool {
  return s1 != s2.m_string;
}

auto operator!=(String::Char const* s1, String const& s2) -> bool {
  return String(s1) != s2;
}

auto operator!=(char const* s1, String const& s2) -> bool {
  return s1 != s2.m_string;
}

auto operator<(String const& s1, String const& s2) -> bool {
  return s1.m_string < s2.m_string;
}

auto operator<(String const& s1, std::string const& s2) -> bool {
  return s1.m_string < s2;
}

auto operator<(String const& s1, String::Char const* s2) -> bool {
  return s1 < String(s2);
}

auto operator<(String const& s1, char const* s2) -> bool {
  return s1.m_string < s2;
}

auto operator<(std::string const& s1, String const& s2) -> bool {
  return s1 < s2.m_string;
}

auto operator<(String::Char const* s1, String const& s2) -> bool {
  return String(s1) < s2;
}

auto operator<(char const* s1, String const& s2) -> bool {
  return s1 < s2.m_string;
}

auto operator+(String s1, String const& s2) -> String {
  s1.append(s2);
  return s1;
}

auto operator+(String s1, std::string const& s2) -> String {
  s1.append(s2);
  return s1;
}

auto operator+(String s1, String::Char const* s2) -> String {
  s1.append(s2);
  return s1;
}

auto operator+(String s1, char const* s2) -> String {
  s1.append(s2);
  return s1;
}

auto operator+(std::string const& s1, String const& s2) -> String {
  return s1 + s2.m_string;
}

auto operator+(String::Char const* s1, String const& s2) -> String {
  return String(s1) + s2;
}

auto operator+(char const* s1, String const& s2) -> String {
  return s1 + s2.m_string;
}

auto operator+(String s, String::Char c) -> String {
  s.append(c);
  return s;
}

auto operator+(String::Char c, String const& s) -> String {
  String res(c);
  res.append(s);
  return res;
}

auto operator*(String const& s, unsigned times) -> String {
  String res;
  for (unsigned i = 0; i < times; ++i)
    res.append(s);
  return res;
}

auto operator*(unsigned times, String const& s) -> String {
  return s * times;
}

auto operator<<(std::ostream& os, String const& s) -> std::ostream& {
  os << s.utf8();
  return os;
}

auto operator>>(std::istream& is, String& s) -> std::istream& {
  std::string temp;
  is >> temp;
  s = String(std::move(temp));
  return is;
}

auto String::compare(std::size_t selfOffset, std::size_t selfLen, String const& other,
                     std::size_t otherOffset, std::size_t otherLen, CaseSensitivity cs) const -> int {
  auto selfIt = begin();
  auto otherIt = other.begin();

  while (selfOffset > 0 && selfIt != end()) {
    ++selfIt;
    --selfOffset;
  }

  while (otherOffset > 0 && otherIt != other.end()) {
    ++otherIt;
    --otherLen;
  }

  while (true) {
    if ((selfIt == end() || selfLen == 0) && (otherIt == other.end() || otherLen == 0))
      return 0;
    else if (selfIt == end() || selfLen == 0)
      return -1;
    else if (otherIt == other.end() || otherLen == 0)
      return 1;

    auto c1 = *selfIt;
    auto c2 = *otherIt;

    if (cs == CaseInsensitive) {
      c1 = toLower(c1);
      c2 = toLower(c2);
    }

    if (c1 < c2)
      return -1;
    else if (c2 < c1)
      return 1;

    ++selfIt;
    ++otherIt;
    --selfLen;
    --otherLen;
  }
}

StringList::StringList() : Base() {}

StringList::StringList(Base const& l) : Base(l) {}

StringList::StringList(Base&& l) : Base(std::move(l)) {}

StringList::StringList(StringList const& l) = default;

StringList::StringList(StringList&& l) : Base(std::move(l)) {}

StringList::StringList(std::size_t n, String::Char const* const* list) {
  for (std::size_t i = 0; i < n; ++i)
    append(String(list[i]));
}

StringList::StringList(std::size_t n, char const* const* list) {
  for (std::size_t i = 0; i < n; ++i)
    append(String(list[i]));
}

StringList::StringList(std::size_t len, String const& s1) : Base(len, s1) {}

StringList::StringList(std::initializer_list<String> list) : Base(list) {}

auto StringList::operator=(Base const& rhs) -> StringList& {
  Base::operator=(rhs);
  return *this;
}

auto StringList::operator=(Base&& rhs) -> StringList& {
  Base::operator=(std::move(rhs));
  return *this;
}

auto StringList::operator=(StringList const& rhs) -> StringList& = default;

auto StringList::operator=(StringList&& rhs) -> StringList& {
  Base::operator=(std::move(rhs));
  return *this;
}

auto StringList::operator=(std::initializer_list<String> list) -> StringList& {
  Base::operator=(std::move(list));
  return *this;
}

auto StringList::contains(String const& s, String::CaseSensitivity cs) const -> bool {
  for (const auto& i : *this) {
    if (s.compare(i, cs) == 0)
      return true;
  }
  return false;
}

auto StringList::trimAll(String const& pattern) const -> StringList {
  StringList r;
  for (auto const& s : *this)
    r.append(s.trim(pattern));
  return r;
}

auto StringList::join(String const& separator) const -> String {
  String joinedString;
  for (auto i = begin(); i != end(); ++i) {
    if (i != begin())
      joinedString += separator;
    joinedString += *i;
  }

  return joinedString;
}

auto StringList::slice(SliceIndex a, SliceIndex b, int i) const -> StringList {
  return Star::slice(*this, a, b, i);
}

auto StringList::sorted() const -> StringList {
  StringList l = *this;
  l.sort();
  return l;
}

auto operator<<(std::ostream& os, const StringList& list) -> std::ostream& {
  os << "(";
  for (auto i = list.begin(); i != list.end(); ++i) {
    if (i != list.begin())
      os << ", ";

    os << '\'' << *i << '\'';
  }
  os << ")";
  return os;
}

auto hash<StringList>::operator()(StringList const& sl) const -> std::size_t {
  std::size_t h = 0;
  for (auto const& s : sl)
    hashCombine(h, hashOf(s));
  return h;
}

}// namespace Star
