import std;
import star.memory;
import star.string_view;
import star.string;
import star.unicode;
import star.format;
import star.hash;
import star.byte_array;

namespace Star {
String::String(StringView s) : m_string(s.utf8()) {}
String::String(std::string_view s) : m_string(s) {}
String& String::operator+=(StringView s) { m_string += s.utf8(); return *this; }
String& String::operator+=(std::string_view s) { m_string += s; return *this; }
StringView::StringView() = default;
StringView::StringView(StringView const& s) = default;
StringView::StringView(StringView&& s) noexcept = default;
StringView::StringView(String const& s) : m_view(s.utf8()) {}
StringView::StringView(char const* s) : m_view(s) {}
StringView::StringView(char const* s, std::size_t n) : m_view(s, n) {}
StringView::StringView(std::string_view const& s) : m_view(s) {}
StringView::StringView(std::string_view&& s) noexcept : m_view(s) {}
StringView::StringView(std::string const& s) : m_view(s) {}
std::size_t hash<String>::operator()(StringView const& s) const { return (*this)(s.utf8()); }


std::string_view const& StringView::utf8() const { return m_view; }
std::string_view StringView::takeUtf8() { return std::exchange(m_view, {}); }
ByteArray StringView::utf8Bytes() const { return ByteArray(m_view.data(), m_view.size()); }
char const* StringView::utf8Ptr() const { return m_view.data(); }
std::size_t StringView::utf8Size() const { return m_view.size(); }
std::expected<StringView::const_iterator, StarException> StringView::begin() const { return const_iterator::create(m_view.begin(), m_view.end()); }
StringView::const_iterator StringView::end() const { return const_iterator::end(m_view.end()); }
std::expected<std::size_t, StarException> StringView::size() const { return utf8Length(m_view.data(), m_view.size()); }
std::expected<std::size_t, StarException> StringView::length() const { return size(); }
bool StringView::empty() const { return m_view.empty(); }
std::expected<StringView::Char, StarException> StringView::operator[](std::size_t index) const { return at(index); }
std::expected<StringView::Char, StarException> StringView::at(std::size_t index) const {
  Utf8Cursor cursor(m_view);
  for (std::size_t i = 0; i != index; ++i) {
    if (cursor.atEnd()) return std::unexpected(OutOfRangeException("Out of range in StringView::at"));
    auto moved = cursor.advance();
    if (!moved) return std::unexpected(std::move(moved.error()));
  }
  if (cursor.atEnd()) return std::unexpected(OutOfRangeException("Out of range in StringView::at"));
  return cursor.read();
}
std::expected<int, StarException> StringView::compare(std::size_t selfOffset, std::size_t selfLen, StringView other, std::size_t otherOffset, std::size_t otherLen, CaseSensitivity cs) const {
  Utf8Cursor a(m_view), b(other.m_view);
  while (selfOffset && !a.atEnd()) {
    auto moved = a.advance(); if (!moved) return std::unexpected(std::move(moved.error()));
    --selfOffset;
  }
  while (otherOffset && !b.atEnd()) {
    auto moved = b.advance(); if (!moved) return std::unexpected(std::move(moved.error()));
    --otherOffset;
  }
  for (;;) {
    bool ae = a.atEnd() || selfLen == 0, be = b.atEnd() || otherLen == 0;
    if (ae || be) return ae ? (be ? 0 : -1) : 1;
    auto ac = a.read(); if (!ac) return std::unexpected(std::move(ac.error()));
    auto bc = b.read(); if (!bc) return std::unexpected(std::move(bc.error()));
    auto av = cs == CaseSensitivity::CaseInsensitive ? String::toLower(*ac) : *ac;
    auto bv = cs == CaseSensitivity::CaseInsensitive ? String::toLower(*bc) : *bc;
    if (av != bv) return av < bv ? -1 : 1;
    auto am = a.advance(); if (!am) return std::unexpected(std::move(am.error()));
    auto bm = b.advance(); if (!bm) return std::unexpected(std::move(bm.error()));
    --selfLen; --otherLen;
  }
}
std::expected<int, StarException> StringView::compare(StringView other, CaseSensitivity cs) const {
  if (cs == CaseSensitivity::CaseSensitive) return m_view.compare(other.m_view);
  return compare(0, NPos, other, 0, NPos, cs);
}
std::expected<bool, StarException> StringView::equals(StringView other, CaseSensitivity cs) const {
  auto result = compare(other, cs);
  if (!result) return std::unexpected(std::move(result.error()));
  return *result == 0;
}
std::expected<bool, StarException> StringView::equalsIgnoreCase(StringView other) const { return equals(other, CaseSensitivity::CaseInsensitive); }
std::expected<bool, StarException> StringView::beginsWith(StringView prefix, CaseSensitivity cs) const {
  auto length = prefix.size();
  if (!length) return std::unexpected(std::move(length.error()));
  auto result = compare(0, *length, prefix, 0, NPos, cs);
  if (!result) return std::unexpected(std::move(result.error()));
  return *result == 0;
}
std::expected<bool, StarException> StringView::endsWith(StringView suffix, CaseSensitivity cs) const {
  auto suffixLength = suffix.size();
  if (!suffixLength) return std::unexpected(std::move(suffixLength.error()));
  if (*suffixLength == 0) return true;
  auto length = size();
  if (!length) return std::unexpected(std::move(length.error()));
  if (*suffixLength > *length) return false;
  auto result = compare(*length - *suffixLength, NPos, suffix, 0, NPos, cs);
  if (!result) return std::unexpected(std::move(result.error()));
  return *result == 0;
}
std::expected<bool, StarException> StringView::beginsWith(Char point, CaseSensitivity cs) const {
  if (empty()) return false;
  Utf8Cursor cursor(m_view);
  auto first = cursor.read();
  if (!first) return std::unexpected(std::move(first.error()));
  return String::charEqual(*first, point, cs);
}
std::expected<bool, StarException> StringView::endsWith(Char point, CaseSensitivity cs) const {
  auto first = begin();
  if (!first) return std::unexpected(std::move(first.error()));
  if (*first == end()) return false;
  auto last = end(); --last;
  return String::charEqual(*last, point, cs);
}
std::expected<std::size_t, StarException> StringView::find(Char point, std::size_t pos, CaseSensitivity cs) const {
  Utf8Cursor cursor(m_view);
  for (std::size_t i = 0; i < pos && !cursor.atEnd(); ++i) {
    auto moved = cursor.advance(); if (!moved) return std::unexpected(std::move(moved.error()));
  }
  while (!cursor.atEnd()) {
    auto current = cursor.read(); if (!current) return std::unexpected(std::move(current.error()));
    if (String::charEqual(*current, point, cs)) return pos;
    auto moved = cursor.advance(); if (!moved) return std::unexpected(std::move(moved.error()));
    ++pos;
  }
  return NPos;
}
std::expected<std::size_t, StarException> StringView::find(StringView needle, std::size_t pos, CaseSensitivity cs) const {
  if (needle.empty()) return 0;
  Utf8Cursor cursor(m_view);
  for (std::size_t i = 0; i < pos && !cursor.atEnd(); ++i) {
    auto moved = cursor.advance(); if (!moved) return std::unexpected(std::move(moved.error()));
  }
  while (!cursor.atEnd()) {
    auto a = cursor; Utf8Cursor b(needle.m_view);
    while (!a.atEnd() && !b.atEnd()) {
      auto bc = b.read(); if (!bc) return std::unexpected(std::move(bc.error()));
      auto ac = a.read(); if (!ac) return std::unexpected(std::move(ac.error()));
      if (!String::charEqual(*ac, *bc, cs)) break;
      auto am = a.advance(); if (!am) return std::unexpected(std::move(am.error()));
      auto bm = b.advance(); if (!bm) return std::unexpected(std::move(bm.error()));
    }
    if (b.atEnd()) return pos;
    auto moved = cursor.advance(); if (!moved) return std::unexpected(std::move(moved.error()));
    ++pos;
  }
  return NPos;
}
std::expected<std::size_t, StarException> StringView::findLast(Char point, CaseSensitivity cs) const {
  auto first = begin();
  if (!first) return std::unexpected(std::move(first.error()));
  std::size_t found = NPos, pos = 0;
  for (auto it = *first; it != end(); ++it, ++pos) if (String::charEqual(*it, point, cs)) found = pos;
  return found;
}
std::expected<std::size_t, StarException> StringView::findLast(StringView needle, CaseSensitivity cs) const {
  if (needle.empty()) return 0;
  Utf8Cursor cursor(m_view);
  std::size_t found = NPos, pos = 0;
  while (!cursor.atEnd()) {
    auto a = cursor; Utf8Cursor b(needle.m_view);
    while (!a.atEnd() && !b.atEnd()) {
      auto bc = b.read(); if (!bc) return std::unexpected(std::move(bc.error()));
      auto ac = a.read(); if (!ac) return std::unexpected(std::move(ac.error()));
      if (!String::charEqual(*ac, *bc, cs)) break;
      auto am = a.advance(); if (!am) return std::unexpected(std::move(am.error()));
      auto bm = b.advance(); if (!bm) return std::unexpected(std::move(bm.error()));
    }
    if (b.atEnd()) found = pos;
    auto moved = cursor.advance(); if (!moved) return std::unexpected(std::move(moved.error()));
    ++pos;
  }
  return found;
}
std::expected<bool, StarException> StringView::hasChar(Char point) const {
  auto result = find(point, 0, CaseSensitivity::CaseSensitive);
  if (!result) return std::unexpected(std::move(result.error()));
  return *result != NPos;
}
std::expected<bool, StarException> StringView::hasCharOrWhitespace(Char point) const {
  if (empty()) return String::isSpace(point);
  return hasChar(point);
}
std::expected<std::size_t, StarException> StringView::findFirstOf(StringView pattern, std::size_t pos) const {
  Utf8Cursor cursor(m_view);
  for (std::size_t i = 0; i < pos && !cursor.atEnd(); ++i) {
    auto moved = cursor.advance(); if (!moved) return std::unexpected(std::move(moved.error()));
  }
  while (!cursor.atEnd()) {
    auto point = cursor.read(); if (!point) return std::unexpected(std::move(point.error()));
    auto match = pattern.hasCharOrWhitespace(*point); if (!match) return std::unexpected(std::move(match.error()));
    if (*match) return pos;
    auto moved = cursor.advance(); if (!moved) return std::unexpected(std::move(moved.error()));
    ++pos;
  }
  return NPos;
}
std::expected<std::size_t, StarException> StringView::findFirstNotOf(StringView pattern, std::size_t pos) const {
  Utf8Cursor cursor(m_view);
  for (std::size_t i = 0; i < pos && !cursor.atEnd(); ++i) {
    auto moved = cursor.advance(); if (!moved) return std::unexpected(std::move(moved.error()));
  }
  while (!cursor.atEnd()) {
    auto point = cursor.read(); if (!point) return std::unexpected(std::move(point.error()));
    auto match = pattern.hasCharOrWhitespace(*point); if (!match) return std::unexpected(std::move(match.error()));
    if (!*match) return pos;
    auto moved = cursor.advance(); if (!moved) return std::unexpected(std::move(moved.error()));
    ++pos;
  }
  return NPos;
}
std::expected<std::size_t, StarException> StringView::findNextBoundary(std::size_t index, bool backwards) const {
  auto first = begin();
  if (!first) return std::unexpected(std::move(first.error()));
  auto it = *first;
  std::size_t pos = 0;
  while (pos != index && it != end()) { ++it; ++pos; }
  if (pos != index) return std::unexpected(OutOfRangeException("Out of range in StringView::findNextBoundary"));
  if ((!backwards && it == end()) || (backwards && index == 0)) return index;
  if (backwards) { --it; --index; }
  while (!String::isSpace(*it)) {
    if (backwards) { if (index == 0) return 0; --it; --index; }
    else { ++it; ++index; if (it == end()) return index; }
  }
  while (String::isSpace(*it)) {
    if (backwards) { if (index == 0) return 0; --it; --index; }
    else { ++it; ++index; if (it == end()) return index; }
  }
  return backwards ? index + 1 : index;
}
std::expected<bool, StarException> StringView::contains(StringView needle, CaseSensitivity cs) const {
  auto result = find(needle, 0, cs);
  if (!result) return std::unexpected(std::move(result.error()));
  return *result != NPos;
}
std::expected<StringView, StarException> StringView::substr(std::size_t position, std::size_t count) const {
  Utf8Cursor cursor(m_view);
  for (std::size_t i = 0; i != position; ++i) {
    if (cursor.atEnd()) return std::unexpected(OutOfRangeException("Out of range in StringView::substr"));
    auto moved = cursor.advance(); if (!moved) return std::unexpected(std::move(moved.error()));
  }
  auto start = cursor.offset();
  for (std::size_t i = 0; i != count && !cursor.atEnd(); ++i) {
    auto moved = cursor.advance(); if (!moved) return std::unexpected(std::move(moved.error()));
  }
  return StringView(m_view.substr(start, cursor.offset() - start));
}
std::expected<void, StarException> StringView::forEachSplitAnyView(StringView pattern, SplitCallback callback) const {
  if (pattern.empty()) return {};
  if (!callback) return std::unexpected(StringException("Split callback is empty"));
  std::size_t start = 0;
  for (;;) {
    auto stop = m_view.find_first_of(pattern.m_view, start);
    auto count = stop == NPos ? m_view.size() - start : stop - start;
    auto result = callback(m_view.substr(start, count), start, count);
    if (!result) return std::unexpected(std::move(result.error()));
    if (stop == NPos) return {};
    start = stop + 1;
  }
}
std::expected<void, StarException> StringView::forEachSplitView(StringView pattern, SplitCallback callback) const {
  if (pattern.empty()) return {};
  if (!callback) return std::unexpected(StringException("Split callback is empty"));
  std::size_t start = 0;
  for (;;) {
    auto stop = m_view.find(pattern.m_view, start);
    auto count = stop == NPos ? m_view.size() - start : stop - start;
    auto result = callback(m_view.substr(start, count), start, count);
    if (!result) return std::unexpected(std::move(result.error()));
    if (stop == NPos) return {};
    start = stop + pattern.m_view.size();
  }
}
StringView& StringView::operator=(StringView s) { m_view = s.m_view; return *this; }
bool operator==(StringView a, char const* b) { return a.m_view.compare(std::string_view(b)) == 0; }
bool operator==(StringView a, std::string const& b) { return a.m_view.compare(std::string_view(b)) == 0; }
bool operator==(StringView a, String const& b) { return a.m_view.compare(std::string_view(b.utf8())) == 0; }
bool operator==(StringView a, StringView b) { return a.m_view == b.m_view; }
bool operator!=(StringView a, StringView b) { return a.m_view != b.m_view; }
bool operator<(StringView a, StringView b) { return a.m_view < b.m_view; }
std::ostream& operator<<(std::ostream& os, StringView const& s) { return os << s.utf8(); }
}
