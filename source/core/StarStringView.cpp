#include "StarStringView.hpp"

import std;

namespace Star {
// To string
String::String(StringView s) : m_string(s.utf8()) {}
String::String(std::string_view s) : m_string(s) {}

auto String::operator+=(StringView s) -> String& {
	m_string += s.utf8();
	return *this;
}

auto String::operator+=(std::string_view s) -> String& {
	m_string += s;
	return *this;
}

StringView::StringView() = default;
StringView::StringView(StringView const& s) = default;
StringView::StringView(StringView&& s) noexcept : m_view(std::move(s.m_view)) {};
StringView::StringView(String const& s) : m_view(s.utf8()) {};
StringView::StringView(char const* s) : m_view(s) {};
StringView::StringView(char const* s, std::size_t n) : m_view(s, n) {};

StringView::StringView(std::string_view const& s) : m_view(s) {};
StringView::StringView(std::string_view&& s) noexcept : m_view(std::move(s)) {};
StringView::StringView(std::string const& s) : m_view(s) {}

StringView::StringView(Char const* s) : m_view((char const*)s, sizeof(*s)) {}
StringView::StringView(Char const* s, std::size_t n) : m_view((char const*)s, n * sizeof(*s)) {};

auto StringView::utf8() const -> std::string_view {
	return m_view;
}

auto StringView::utf8Ptr() const -> char const* {
	return m_view.data();
}

auto StringView::utf8Bytes() const -> ByteArray {
	return ByteArray(m_view.data(), m_view.size());
}

auto StringView::utf8Size() const -> std::size_t {
	return m_view.size();
}

auto StringView::begin() const -> StringView::const_iterator {
	return {m_view.begin()};
}

auto StringView::end() const -> StringView::const_iterator {
	return {m_view.end()};
}

auto StringView::size() const -> std::size_t {
	return utf8Length(m_view.data(), m_view.size());
}

auto StringView::length() const -> std::size_t {
	return size();
}

auto StringView::empty() const -> bool {
	return m_view.empty();
}

auto StringView::operator[](std::size_t index) const -> StringView::Char {
	auto it = begin();
	for (std::size_t i = 0; i < index; ++i)
		++it;
	return *it;
}

auto StringView::at(std::size_t index) const -> StringView::Char {
	auto it = begin();
	auto itEnd = end();
	for (std::size_t i = 0; i < index; ++i) {
		++it;
		if (it == itEnd)
			throw OutOfRangeException(strf("Out of range in StringView::at({})", i));
	}
	return *it;
}

auto StringView::endsWith(StringView end, CaseSensitivity cs) const -> bool {
	auto endsize = end.size();
	if (endsize == 0)
		return true;

	auto mysize = size();
	if (endsize > mysize)
		return false;

	return compare(mysize - endsize, std::numeric_limits<std::size_t>::max(), end, 0, std::numeric_limits<std::size_t>::max(), cs) == 0;
}
auto StringView::endsWith(Char end, CaseSensitivity cs) const -> bool {
	if (m_view.empty())
		return false;

	return String::charEqual(end, operator[](size() - 1), cs);
}

auto StringView::beginsWith(StringView beg, CaseSensitivity cs) const -> bool {
	if (beg.m_view.empty())
		return true;

	std::size_t begSize = beg.size();
	auto it = begin();
	auto itEnd = end();
	for (std::size_t i = 0; i != begSize; ++i) {
		if (it == itEnd)
			return false;
		it++;
	}

	return compare(0, begSize, beg, 0, std::numeric_limits<std::size_t>::max(), cs) == 0;
}

auto StringView::beginsWith(Char beg, CaseSensitivity cs) const -> bool {
	if (m_view.empty())
		return false;

	return String::charEqual(beg, operator[](0), cs);
}

void StringView::forEachSplitAnyView(StringView chars, SplitCallback callback) const {
	if (chars.empty())
		return;

	std::size_t beg = 0;
	while (true) {
		std::size_t end = m_view.find_first_of(chars.m_view, beg);
		if (end == std::numeric_limits<std::size_t>::max()) {
			callback(StringView(m_view.substr(beg)), beg, m_view.size() - beg);
			break;
		}
		callback(StringView(m_view.substr(beg, end - beg)), beg, end - beg);
		beg = end + 1;
	}
}

void StringView::forEachSplitView(StringView pattern, SplitCallback callback) const {
	if (pattern.empty())
		return;

	std::size_t beg = 0;
	while (true) {
		std::size_t end = m_view.find(pattern.m_view, beg);
		if (end == std::numeric_limits<std::size_t>::max()) {
			callback(StringView(m_view.substr(beg)), beg, m_view.size() - beg);
			break;
		}
		callback(StringView(m_view.substr(beg, end - beg)), beg, end - beg);
		beg = end + pattern.m_view.size();
	}
}

auto StringView::hasChar(Char c) const -> bool {
	for (Char ch : *this)
		if (ch == c)
			return true;
	return false;
}

auto StringView::hasCharOrWhitespace(Char c) const -> bool {
	return empty() ? String::isSpace(c) : hasChar(c);
}

auto StringView::find(Char c, std::size_t pos, CaseSensitivity cs) const -> std::size_t {
	auto it = begin();
	for (std::size_t i = 0; i < pos; ++i) {
		if (it == end())
			break;
		++it;
	}

	while (it != end()) {
		if (String::charEqual(c, *it, cs))
			return pos;
		++pos;
		++it;
	}

	return std::numeric_limits<std::size_t>::max();
}

auto StringView::find(StringView str, std::size_t pos, CaseSensitivity cs) const -> std::size_t {
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
		if (String::charEqual(*sit, *mit, cs)) {
			do {
				++mit;
				++sit;
				if (sit == str.end())
					return pos;
				else if (mit == end())
					break;
			} while (String::charEqual(*sit, *mit, cs));
			sit = str.begin();
		}
		++pos;
		mit = ++it;
	}

	return std::numeric_limits<std::size_t>::max();
}

auto StringView::findLast(Char c, CaseSensitivity cs) const -> std::size_t {
	auto it = begin();

	std::size_t found = std::numeric_limits<std::size_t>::max();
	std::size_t pos = 0;
	while (it != end()) {
		if (String::charEqual(c, *it, cs))
			found = pos;
		++pos;
		++it;
	}

	return found;
}

auto StringView::findLast(StringView str, CaseSensitivity cs) const -> std::size_t {
	if (str.empty())
		return 0;

	std::size_t pos = 0;
	auto it = begin();
	std::size_t result = std::numeric_limits<std::size_t>::max();
	const_iterator sit = str.begin();
	const_iterator mit = it;
	while (it != end()) {
		if (String::charEqual(*sit, *mit, cs)) {
			do {
				++mit;
				++sit;
				if (sit == str.end()) {
					result = pos;
					break;
				}
				if (mit == end())
					break;
			} while (String::charEqual(*sit, *mit, cs));
			sit = str.begin();
		}
		++pos;
		mit = ++it;
	}

	return result;
}

auto StringView::findFirstOf(StringView pattern, std::size_t beg) const -> std::size_t {
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

auto StringView::findFirstNotOf(StringView pattern, std::size_t beg) const -> std::size_t {
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

auto StringView::findNextBoundary(std::size_t index, bool backwards) const -> std::size_t {
	//TODO: Make this faster.
	std::size_t mySize = size();
	if (!backwards && (index == mySize))
		return index;
	if (backwards) {
		if (index == 0)
			return 0;
		index--;
	}
	Char c = this->at(index);
	while (!String::isSpace(c)) {
		if (backwards && (index == 0))
			return 0;
		index += backwards ? -1 : 1;
		if (index == mySize)
			return mySize;
		c = this->at(index);
	}
	while (String::isSpace(c)) {
		if (backwards && (index == 0))
			return 0;
		index += backwards ? -1 : 1;
		if (index == mySize)
			return mySize;
		c = this->at(index);
	}
	if (backwards && !(index == mySize))
		return index + 1;
	return index;
}

auto StringView::contains(StringView s, CaseSensitivity cs) const -> bool {
	return find(s, 0, cs) != std::numeric_limits<std::size_t>::max();
}

auto StringView::compare(StringView s, CaseSensitivity cs) const -> int {
	if (cs == CaseSensitivity::CaseSensitive)
		return m_view.compare(s.m_view);
	else
		return compare(0, std::numeric_limits<std::size_t>::max(), s, 0, std::numeric_limits<std::size_t>::max(), cs);
}

auto StringView::equals(StringView s, CaseSensitivity cs) const -> bool {
	return compare(s, cs) == 0;
}

auto StringView::equalsIgnoreCase(StringView s) const -> bool {
	return compare(s, CaseSensitivity::CaseInsensitive) == 0;
}

auto StringView::substr(std::size_t position, std::size_t n) const -> StringView {
	StringView ret;
	auto itEnd = end();
	auto it = begin();

	for (std::size_t i = 0; i != position; ++i) {
		if (it == itEnd)
			throw OutOfRangeException(strf("out of range in StringView::substr({}, {})", position, n));
		it++;
	}

	const auto itStart = it;

	for (std::size_t i = 0; i != n; ++i) {
		if (it == itEnd)
			break;
		++it;
	}

	return {&*itStart.base(), static_cast<std::size_t>(it.base() - itStart.base())};
}

auto StringView::compare(std::size_t selfOffset, std::size_t selfLen, StringView other,
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

		if (cs == CaseSensitivity::CaseInsensitive) {
			c1 = String::toLower(c1);
			c2 = String::toLower(c2);
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

auto StringView::operator=(StringView s) -> StringView& {
	m_view = s.m_view;
	return *this;
}

auto operator==(StringView s1, const char* s2) -> bool {
	return s1.m_view.compare(s2) == 0;
}

auto operator==(StringView s1, std::string const& s2) -> bool {
	return s1.m_view.compare(s2) == 0;
}

auto operator==(StringView s1, String const& s2) -> bool {
	return s1.m_view.compare(s2.utf8()) == 0;
}

auto operator==(StringView s1, StringView s2) -> bool {
	return s1.m_view == s2.m_view;
}

auto operator!=(StringView s1, StringView s2) -> bool {
	return s1.m_view != s2.m_view;
}

auto operator<(StringView s1, StringView s2) -> bool {
	return s1.m_view < s2.m_view;
}

auto operator<<(std::ostream& os, StringView const& s) -> std::ostream& {
	os << s.utf8();
	return os;
}

}// namespace Star
