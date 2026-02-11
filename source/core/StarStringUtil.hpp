#pragma once

#include "StarList.hpp"
#include "StarUnicode.hpp"

import std;

namespace Star::StringUtils {
using Char = Utf32Type;
using StdString = std::string;
using StdStringList = List<StdString>;

// C++26 friendly check for space
inline auto isSpace(Char c) -> bool {
	return c == 0x20 || c == 0x09 || c == 0x0a || c == 0x0d || c == 0xfeff;
}

inline auto isAsciiNumber(Char c) -> bool {
	return c >= '0' && c <= '9';
}

inline auto isAsciiLetter(Char c) -> bool {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

inline auto toLower(Char c) -> Char {
	if (c >= 'A' && c <= 'Z')
		return c + 32;
	return c;
}

inline auto toUpper(Char c) -> Char {
	if (c >= 'a' && c <= 'z')
		return c - 32;
	return c;
}

// UTF-8 aware length
inline auto length(std::string_view s) -> std::size_t {
	return utf8Length(s.data(), s.size());
}

// C++26 Range-based split
inline auto split(std::string_view s, std::string_view pattern) -> StdStringList {
	if (pattern.empty())
		return {StdString(s)};
	StdStringList result;
	for (const auto word : s | std::views::split(pattern)) {
		result.emplace_back(std::string_view(word));
	}
	return result;
}

// Advanced join with cleaner implementation
inline auto joinWith(std::string_view joiner, std::span<const StdString> strings) -> StdString {
	if (strings.empty())
		return {};
	auto view = strings
	  | std::views::filter([](auto const& s) -> auto { return !s.empty(); })
	  | std::views::join_with(joiner);

	std::string result;
	for (char c : view)
		result.push_back(c);
	return result;
}

// Porting the tag lookup logic to a generic utility
template <typename Lookup>
auto replaceTags(std::string_view s, Lookup&& lookup) -> StdString {
	std::string result;
	result.reserve(s.size());
	std::size_t start = 0;
	while (true) {
		auto beginTag = s.find('<', start);
		auto endTag = s.find('>', beginTag);
		if (beginTag != std::string_view::npos && endTag != std::string_view::npos) {
			result.append(s.substr(start, beginTag - start));
			std::string_view key = s.substr(beginTag + 1, endTag - beginTag - 1);
			result.append(lookup(key));
			start = endTag + 1;
		} else {
			result.append(s.substr(start));
			break;
		}
	}
	return result;
}

}// namespace Star::StringUtils
