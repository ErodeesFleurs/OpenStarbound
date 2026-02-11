#pragma once

#include "StarString.hpp"

import std;

namespace Star {

class StringView {
  public:
	using Char = String::Char;

	using const_iterator = U8ToU32Iterator<std::string_view::const_iterator>;
	using value_type = Char;
	using const_reference = value_type const&;

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
	[[nodiscard]] auto utf8() const -> std::string_view;
	[[nodiscard]] auto utf8Bytes() const -> ByteArray;
	// Pointer to internal utf8 data, null-terminated.
	[[nodiscard]] auto utf8Ptr() const -> char const*;
	[[nodiscard]] auto utf8Size() const -> std::size_t;

	[[nodiscard]] auto begin() const -> const_iterator;
	[[nodiscard]] auto end() const -> const_iterator;

	[[nodiscard]] auto size() const -> std::size_t;
	[[nodiscard]] auto length() const -> std::size_t;

	[[nodiscard]] auto empty() const -> bool;

	auto operator[](std::size_t index) const -> Char;
	// Throws StringException if i out of range.
	[[nodiscard]] auto at(std::size_t i) const -> Char;

	[[nodiscard]] auto endsWith(StringView end, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const -> bool;
	[[nodiscard]] auto endsWith(Char end, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const -> bool;
	[[nodiscard]] auto beginsWith(StringView beg, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const -> bool;
	[[nodiscard]] auto beginsWith(Char beg, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const -> bool;

	[[nodiscard]] auto hasChar(Char c) const -> bool;
	// Identical to hasChar, except, if string is empty, tests if c is
	// whitespace.
	[[nodiscard]] auto hasCharOrWhitespace(Char c) const -> bool;

	[[nodiscard]] auto find(Char c, std::size_t beg = 0, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const -> std::size_t;
	[[nodiscard]] auto find(StringView s, std::size_t beg = 0, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const -> std::size_t;
	[[nodiscard]] auto findLast(Char c, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const -> std::size_t;
	[[nodiscard]] auto findLast(StringView s, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const -> std::size_t;

	// If pattern is empty, finds first whitespace
	[[nodiscard]] auto findFirstOf(StringView chars = "", std::size_t beg = 0) const -> std::size_t;

	// If pattern is empty, finds first non-whitespace
	[[nodiscard]] auto findFirstNotOf(StringView chars = "", std::size_t beg = 0) const -> std::size_t;

	// finds the the start of the next 'boundary' in a string.  used for quickly
	// scanning a string
	[[nodiscard]] auto findNextBoundary(std::size_t index, bool backwards = false) const -> std::size_t;

	using SplitCallback = std::function<void(StringView, std::size_t, std::size_t)>;
	void forEachSplitAnyView(StringView pattern, SplitCallback) const;
	void forEachSplitView(StringView pattern, SplitCallback) const;

	[[nodiscard]] auto contains(StringView s, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const -> bool;

	[[nodiscard]] auto compare(StringView s, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const -> int;
	[[nodiscard]] auto equals(StringView s, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const -> bool;
	// Synonym for equals(s, String::CaseInsensitive)
	[[nodiscard]] auto equalsIgnoreCase(StringView s) const -> bool;

	[[nodiscard]] auto substr(std::size_t position, std::size_t n = std::numeric_limits<std::size_t>::max()) const -> StringView;

	auto operator=(StringView s) -> StringView&;

	friend auto operator==(StringView s1, const char* s2) -> bool;
	friend auto operator==(StringView s1, std::string const& s2) -> bool;
	friend auto operator==(StringView s1, String const& s2) -> bool;
	friend auto operator==(StringView s1, StringView s2) -> bool;
	friend auto operator!=(StringView s1, StringView s2) -> bool;
	friend auto operator<(StringView s1, StringView s2) -> bool;

	friend auto operator<<(std::ostream& os, StringView const& s) -> std::ostream&;

  private:
	[[nodiscard]] auto compare(std::size_t selfOffset,
	                           std::size_t selfLen,
	                           StringView other,
	                           std::size_t otherOffset,
	                           std::size_t otherLen,
	                           CaseSensitivity cs) const -> int;

	std::string_view m_view;
};

}// namespace Star

template <>
struct std::formatter<Star::StringView> : std::formatter<std::string_view> {
	auto format(Star::StringView const& s, std::format_context& ctx) const {
		return std::formatter<std::string_view>::format(s.utf8(), ctx);
	}
};
