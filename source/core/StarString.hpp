#pragma once

#include "StarByteArray.hpp"
#include "StarException.hpp"
#include "StarFormat.hpp"
#include "StarHash.hpp"
#include "StarUnicode.hpp"

import std;

namespace Star {

class StringList;
class StringView;

using StringException = ExceptionDerived<"StringException">;

// A Unicode string class, which is a basic UTF-8 aware wrapper around
// std::string.  Provides methods for accessing UTF-32 "char32_t" type, which
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
	using const_iterator = U8ToU32Iterator<std::string::const_iterator>;
	enum class CaseSensitivity : std::uint8_t {
		CaseSensitive,
		CaseInsensitive
	};

	// Space, horizontal tab, newline, carriage return, and BOM / ZWNBSP
	[[nodiscard]] static auto is_space(char32_t c) -> bool;
	[[nodiscard]] static auto is_ascii_number(char32_t c) -> bool;
	[[nodiscard]] static auto is_ascii_letter(char32_t c) -> bool;

	[[nodiscard]] static auto to_lower(char32_t c) -> char32_t;
	[[nodiscard]] static auto to_upper(char32_t c) -> char32_t;
	[[nodiscard]] static auto char_equal(char32_t c1, char32_t c2, CaseSensitivity cs) -> bool;

	[[nodiscard]] static auto join_with(String const& join, String const& left, String const& right) -> String;
	template <typename... StringType>
	[[nodiscard]] static auto join_with(String const& join, String const& first, String const& second, String const& third, StringType const&... rest) -> String;

	String() = default;
	String(String const& s) = default;
	String(String&& s) noexcept = default;

	// These assume utf8 input
	explicit String(char const* s);
	String(char const* s, std::size_t n);
	explicit String(std::string);
	explicit String(const std::string& s);
	explicit String(std::string&& s);
	explicit String(std::string_view s);
	explicit String(std::u8string const& s);
	explicit String(std::u8string&& s);
	explicit String(std::u8string_view s);

	explicit String(std::u32string const& s);
	explicit String(char32_t const* s);
	String(char32_t const* s, std::size_t n);
	String(char32_t c, std::size_t n);

	explicit String(char32_t c);

	// const& to internal utf8 data
	[[nodiscard]] auto utf8() const -> std::u8string const&;
	[[nodiscard]] explicit operator std::u8string_view() const noexcept { return m_string; }
	[[nodiscard]] auto take_utf8() -> std::u8string;
	[[nodiscard]] auto utf8_bytes() const -> ByteArray;
	// Pointer to internal utf8 data, null-terminated.
	[[nodiscard]] auto utf8_ptr() const -> char const*;
	[[nodiscard]] auto utf8_size() const -> std::size_t;

	[[nodiscard]] auto wstring() const -> std::wstring;
	[[nodiscard]] auto u32string() const -> std::u32string;

	[[nodiscard]] auto begin() const -> const_iterator;
	[[nodiscard]] auto end() const -> const_iterator;

	[[nodiscard]] auto size() const -> std::size_t;
	[[nodiscard]] auto length() const -> std::size_t;

	void clear();
	void reserve(std::size_t n);
	[[nodiscard]] auto empty() const -> bool;

	[[nodiscard]] auto operator[](std::size_t i) const -> char32_t;
	// Throws StringException if i out of range.
	[[nodiscard]] auto at(std::size_t i) const -> char32_t;

	[[nodiscard]] auto to_upper() const -> String;
	[[nodiscard]] auto to_lower() const -> String;
	[[nodiscard]] auto title_case() const -> String;

	[[nodiscard]] auto ends_with(std::u8string_view end, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const -> bool;
	[[nodiscard]] auto ends_with(char32_t end, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const -> bool;
	[[nodiscard]] auto begins_with(std::u8string_view beg, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const -> bool;
	[[nodiscard]] auto begins_with(char32_t beg, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const -> bool;

	[[nodiscard]] auto reverse() const -> String;

	[[nodiscard]] auto rot13() const -> String;

	[[nodiscard]] auto split(char32_t c, std::size_t max_split = std::numeric_limits<std::size_t>::max()) const -> StringList;
	[[nodiscard]] auto split(std::string_view pattern, std::size_t max_split = std::numeric_limits<std::size_t>::max()) const -> StringList;
	[[nodiscard]] auto rsplit(char32_t c, std::size_t max_split = std::numeric_limits<std::size_t>::max()) const -> StringList;
	[[nodiscard]] auto rsplit(std::string_view pattern, std::size_t max_split = std::numeric_limits<std::size_t>::max()) const -> StringList;

	// Splits on any number of contiguous instances of any of the given
	// characters.  Behaves differently than regular split in that leading and
	// trailing instances of the characters are also ignored, and in general no
	// empty strings will be in the resulting split list.  If chars is empty,
	// then splits on any whitespace.
	[[nodiscard]] auto split_any(std::u8string_view chars = {}, std::size_t max_split = std::numeric_limits<std::size_t>::max()) const -> StringList;
	[[nodiscard]] auto rsplit_any(std::u8string_view chars = {}, std::size_t max_split = std::numeric_limits<std::size_t>::max()) const -> StringList;

	// Split any with '\n\r'
	[[nodiscard]] auto split_lines(std::size_t max_split = std::numeric_limits<std::size_t>::max()) const -> StringList;
	// Shorthand for splitAny("");
	[[nodiscard]] auto split_whitespace(std::size_t max_split = std::numeric_limits<std::size_t>::max()) const -> StringList;

	// Splits a string once based on the given characters (defaulting to
	// whitespace), and returns the first part.  This string is set to the
	// second part.
	[[nodiscard]] auto extract(std::u8string_view chars = {}) -> String;
	[[nodiscard]] auto rextract(std::u8string_view chars = {}) -> String;

	[[nodiscard]] auto has_char(char32_t c) const -> bool;
	// Identical to haschar32_t, except, if string is empty, tests if c is
	// whitespace.
	[[nodiscard]] auto has_char_or_whitespace(char32_t c) const -> bool;

	[[nodiscard]] auto replace(std::string_view rplc, std::string_view val, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const -> String;

	[[nodiscard]] auto trim_end(std::string_view chars = {}) const -> String;
	[[nodiscard]] auto trim_beg(std::string_view chars = {}) const -> String;
	[[nodiscard]] auto trim(std::string_view chars = {}) const -> String;

	[[nodiscard]] auto find(char32_t c, std::size_t beg = 0, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const -> std::size_t;
	[[nodiscard]] auto find(std::string_view s, std::size_t beg = 0, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const -> std::size_t;
	[[nodiscard]] auto find_last(char32_t c, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const -> std::size_t;
	[[nodiscard]] auto find_last(std::string_view s, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const -> std::size_t;

	// If pattern is empty, finds first whitespace
	[[nodiscard]] auto find_first_of(std::string_view chars = {}, std::size_t beg = 0) const -> std::size_t;

	// If pattern is empty, finds first non-whitespace
	[[nodiscard]] auto find_first_not_of(std::string_view chars = {}, std::size_t beg = 0) const -> std::size_t;

	// finds the the start of the next 'boundary' in a string.  used for quickly
	// scanning a string
	[[nodiscard]] auto find_next_boundary(std::size_t index, bool backwards = false) const -> std::size_t;

	// [[nodiscard]] auto slice(SliceIndex a = SliceIndex(), SliceIndex b = SliceIndex(), int i = 1) const -> String;

	void append(std::string_view s);
	void append(char32_t const* s);
	void append(char32_t const* s, std::size_t n);
	void append(char32_t c);

	void prepend(std::string_view s);
	void prepend(char32_t const* s);
	void prepend(char32_t const* s, std::size_t n);
	void prepend(char32_t c);

	void push_back(char32_t c);
	void push_front(char32_t c);

	[[nodiscard]] auto contains(std::u8string_view s, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const -> bool;

	// Does this string match the given regular expression?
	[[nodiscard]] auto regex_match(std::u8string_view regex, bool full = true, bool case_sensitive = true) const -> bool;

	[[nodiscard]] auto compare(std::u8string_view s, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const -> int;
	[[nodiscard]] auto equals(std::u8string_view s, CaseSensitivity cs = CaseSensitivity::CaseSensitive) const -> bool;
	// Synonym for equals(s, String::CaseInsensitive)
	[[nodiscard]] auto equals_ignore_case(std::u8string_view s) const -> bool;

	[[nodiscard]] auto substr(std::size_t position, std::size_t n = std::numeric_limits<std::size_t>::max()) const -> String;
	void erase(std::size_t pos = 0, std::size_t n = std::numeric_limits<std::size_t>::max());

	[[nodiscard]] auto pad_left(std::size_t size, std::u8string_view filler) const -> String;
	[[nodiscard]] auto pad_right(std::size_t size, std::u8string_view filler) const -> String;

	// Replace angle bracket tags in the string with values given by the given
	// lookup function.  Will be called as:
	// String lookup(String const& key);
	template <typename Lookup>
	auto lookup_tags(Lookup&& lookup) const -> String;

	// StringView variant
	template <typename Lookup>
	auto maybe_lookup_tags_view(Lookup&& lookup) const -> std::optional<String>;

	template <typename Lookup>
	auto lookup_tags_view(Lookup&& lookup) const -> String;

	// Replace angle bracket tags in the string with values given by the tags
	// map.  If replaceWithDefault is true, then values that are not found in the
	// tags map are replace with the default string.  If replaceWithDefault is
	// false, tags that are not found are not replaced at all.
	template <typename MapType>
	[[nodiscard]] auto replace_tags(MapType const& tags, bool replace_with_default = false, std::u8string_view defaultValue = {}) const -> String;

	auto operator=(String const& s) -> String& = default;
	auto operator=(String&& s) noexcept -> String& = default;

	auto operator+=(std::string_view s) -> String&;
	auto operator+=(char32_t const* s) -> String&;
	auto operator+=(char32_t c) -> String&;

	auto operator==(String const& other) const -> bool = default;
	auto operator<=>(String const& other) const = default;

	auto operator==(std::string_view other) const -> bool;
	auto operator<=>(std::string_view other) const;

	auto operator==(char32_t const* other) const -> bool;
	auto operator<=>(char32_t const* other) const;

	friend auto operator+(String s1, String const& s2) -> String;
	friend auto operator+(String s1, std::string_view s2) -> String;
	friend auto operator+(std::string_view s1, String const& s2) -> String;
	friend auto operator+(String s1, char32_t const* s2) -> String;
	friend auto operator+(char32_t const* s1, String const& s2) -> String;

	friend auto operator+(String s, char32_t c) -> String;
	friend auto operator+(char32_t c, String const& s) -> String;

	friend auto operator*(String const& s, unsigned times) -> String;
	friend auto operator*(unsigned times, String const& s) -> String;

	friend auto operator<<(std::ostream& os, String const& s) -> std::ostream&;
	friend auto operator>>(std::istream& is, String& s) -> std::istream&;

	// String view functions
	explicit String(StringView s);

	auto operator+=(StringView s) -> String&;

  private:
	[[nodiscard]] auto compare(std::size_t selfOffset,
	                           std::size_t selfLen,
	                           String const& other,
	                           std::size_t otherOffset,
	                           std::size_t otherLen,
	                           CaseSensitivity cs) const -> int;

	std::u8string m_string;
};

class StringList : public std::vector<String> {
  public:
	using Base = std::vector<String>;

	using iterator = Base::iterator;
	using const_iterator = Base::const_iterator;
	using value_type = Base::value_type;
	using reference = Base::reference;
	using const_reference = Base::const_reference;

	template <typename Container>
	static auto from(Container const& m) -> StringList {
		StringList l;
		for (auto const& i : m) {
			l.push_back(String(i));
		}
		return l;
	}

	StringList();
	explicit StringList(Base const& l);
	explicit StringList(Base&& l);
	StringList(StringList const& l);
	StringList(StringList&& l) noexcept;
	StringList(std::size_t len, char32_t const* const* list);
	StringList(std::size_t len, char const* const* list);
	explicit StringList(std::size_t len, String const& s1 = String());
	StringList(std::initializer_list<String> list);

	template <typename InputIterator>
	StringList(InputIterator beg, InputIterator end)
	    : Base(beg, end) {}

	auto operator=(Base const& rhs) -> StringList&;
	auto operator=(Base&& rhs) -> StringList&;
	auto operator=(StringList const& rhs) -> StringList&;
	auto operator=(StringList&& rhs) noexcept -> StringList&;
	auto operator=(std::initializer_list<String> list) -> StringList&;

	[[nodiscard]] auto contains(const std::string& s, String::CaseSensitivity cs = String::CaseSensitivity::CaseSensitive) const -> bool;
	[[nodiscard]] auto trim_all(const std::string& chars = "") const -> StringList;
	[[nodiscard]] auto join(const std::string& separator = "") const -> String;

	// [[nodiscard]] auto slice(SliceIndex a = SliceIndex(), SliceIndex b = SliceIndex(), int i = 1) const -> StringList;

	template <typename Filter>
	auto filtered(Filter&& filter) const -> StringList;

	template <typename Comparator>
	auto sorted(Comparator&& comparator) const -> StringList;

	[[nodiscard]] auto sorted() const -> StringList;

	auto filtered(const std::function<bool(String const&)>& filter) const -> StringList;
};

auto operator<<(std::ostream& os, StringList const& list) -> std::ostream&;

template <>
struct hash<String> {
	[[nodiscard]] auto operator()(String const& s) const -> std::size_t;
};

struct CaseInsensitiveStringHash {
	[[nodiscard]] auto operator()(String const& s) const -> std::size_t;
};

struct CaseInsensitiveStringCompare {
	[[nodiscard]] auto operator()(String const& s1, String const& s2) const -> bool;
};

using StringSet = std::unordered_set<String>;

using CaseInsensitiveStringSet = std::unordered_set<String, CaseInsensitiveStringHash, CaseInsensitiveStringCompare>;

template <typename MappedT, typename HashT = hash<String>, typename ComparatorT = std::equal_to<String>>
using StringMap = std::unordered_map<String, MappedT, HashT, ComparatorT>;

template <typename MappedT, typename HashT = hash<String>, typename ComparatorT = std::equal_to<String>>
using StableStringMap = std::map<String, MappedT, HashT, ComparatorT>;

template <typename MappedT>
using CaseInsensitiveStringMap = StringMap<MappedT, CaseInsensitiveStringHash, CaseInsensitiveStringCompare>;

template <>
struct hash<StringList> {
	[[nodiscard]] auto operator()(StringList const& s) const -> std::size_t;
};

template <typename... StringType>
auto String::join_with(
  String const& join, String const& first, String const& second, String const& third, StringType const&... rest) -> String {
	return join_with(join, join_with(join, first, second), third, rest...);
}

template <typename Lookup>
auto String::lookup_tags(Lookup&& lookup) const -> String {
	// Operates directly on the utf8 representation of the strings, rather than
	// using unicode find / replace methods

	auto substr_into = [](std::string const& ref, std::size_t position, std::size_t n, std::string& result) -> auto {
		auto len = ref.size();
		if (position > len) {
			throw OutOfRangeException(strf("out of range in substrInto: {}", position));
		}

		auto it = ref.begin();
		std::advance(it, position);

		for (std::size_t i = 0; i < n; ++i) {
			if (it == ref.end()) {
				break;
			}
			result.push_back(*it);
			++it;
		}
	};

	std::u8string finalString;

	std::size_t start = 0;
	std::size_t size = String::size();

	finalString.reserve(size);

	String key;

	while (true) {
		if (start >= size) {
			break;
		}

		std::size_t beginTag = m_string.find('<', start);
		std::size_t endTag = m_string.find('>', beginTag);
		if (beginTag != std::numeric_limits<std::size_t>::max() && endTag != std::numeric_limits<std::size_t>::max()) {
			substr_into(m_string, beginTag + 1, endTag - beginTag - 1, key.m_string);
			substr_into(m_string, start, beginTag - start, finalString);
			finalString += lookup(key).m_string;
			key.m_string.clear();
			start = endTag + 1;

		} else {
			substr_into(m_string, start, std::numeric_limits<std::size_t>::max(), finalString);
			break;
		}
	}

	return String(finalString);
}

template <typename Lookup>
auto String::maybe_lookup_tags_view(Lookup&& lookup) const -> std::optional<String> {
	std::vector<std::u8string_view> final_views = {};
	std::u8string_view view(utf8());

	std::size_t start = 0;
	while (true) {
		if (start >= view.size()) {
			break;
		}

		std::size_t begin_tag = view.find_first_of('<', start);
		if (begin_tag == std::numeric_limits<std::size_t>::max() && !start) {
			return std::nullopt;
		}

		std::size_t end_tag = view.find_first_of('>', begin_tag);
		if (begin_tag != std::numeric_limits<std::size_t>::max() && end_tag != std::numeric_limits<std::size_t>::max()) {
			final_views.push_back(view.substr(start, begin_tag - start));
			final_views.push_back(lookup(view.substr(begin_tag + 1, end_tag - begin_tag - 1)).takeUtf8());
			start = end_tag + 1;
		} else {
			final_views.push_back(view.substr(start));
			break;
		}
	}

	std::u8string finalString;
	std::size_t finalSize = 0;
	for (auto& view : final_views) {
		finalSize += view.size();
	}

	finalString.reserve(finalSize);

	for (auto& view : final_views) {
		finalString += view;
	}

	return String(finalString);
}

template <typename Lookup>
auto String::lookup_tags_view(Lookup&& lookup) const -> String {
	auto result = maybe_lookup_tags_view(lookup);
	return result ? std::move(*result) : String();
}

template <typename MapType>
auto String::replace_tags(MapType const& tags, bool replace_with_default, std::u8string_view defaultValue) const -> String {
	return lookup_tags([&](String const& key) -> String {
		auto i = tags.find(key);
		if (i == tags.end()) {
			if (replace_with_default) {
				return String(defaultValue);
			} else {
				return "<" + key + ">";
			}
		} else {
			return i->second;
		}
	});
}

inline auto hash<String>::operator()(String const& s) const -> std::size_t {
	PLHasher hash;
	for (auto c : s.utf8()) {
		hash.put(c);
	}
	return hash.hash();
}

template <typename Filter>
auto StringList::filtered(Filter&& filter) const -> StringList {
	StringList l;
	std::ranges::copy_if(begin(), end(), std::back_inserter(l), forward<Filter>(filter));
	return l;
}

template <typename Comparator>
auto StringList::sorted(Comparator&& comparator) const -> StringList {
	StringList l;
	std::ranges::copy(begin(), end(), std::back_inserter(l));
	std::ranges::sort(l, forward<Comparator>(comparator));
	return l;
}

}// namespace Star
