module star.unicode;

import std;

namespace star::unicode::detail {

constexpr auto is_continuation_byte(char8_t c) noexcept -> bool {
    return (static_cast<unsigned char>(c) & 0xC0) == 0x80;//NOLINT
}

constexpr auto utf8_sequence_length(char8_t lead) noexcept -> unsigned {
    auto uc = static_cast<std::uint8_t>(lead);
    if ((uc & 0x80) == 0) {//NOLINT
        return 1;
    }
    if ((uc & 0xE0) == 0xC0) {//NOLINT
        return 2;
    }
    if ((uc & 0xF0) == 0xE0) {//NOLINT
        return 3;
    }
    if ((uc & 0xF8) == 0xF0) {//NOLINT
        return 4;
    }
    return 0;
}

constexpr auto is_overlong(char32_t cp, std::uint32_t len) noexcept -> bool {
    if (len == 2 && cp < 0x80) {
        return true;
    }
    if (len == 3 && cp < 0x800) {
        return true;
    }
    if (len == 4 && cp < 0x10000) {
        return true;
    }
    return false;
}

constexpr auto valid_code_point(char32_t cp) noexcept -> bool {
    return cp <= 0x10FFFF && (cp < 0xD800 || cp > 0xDFFF);
}

constexpr auto decode_with_length(std::span<const char8_t> s) noexcept
  -> std::expected<std::pair<char32_t, std::size_t>, unicode_errc> {
    if (s.empty()) {
        return std::unexpected(unicode_errc::truncated_utf8);
    }
    unsigned len = utf8_sequence_length(s[0]);
    if (len == 0) {
        return std::unexpected(unicode_errc::invalid_utf8_sequence);
    }
    if (s.size() < len) {
        return std::unexpected(unicode_errc::truncated_utf8);
    }

    char32_t cp = 0;
    if (len == 1) {
        cp = static_cast<unsigned char>(s[0]);
    } else {
        cp = static_cast<unsigned char>(s[0]) & (0x7F >> len);//NOLINT
        for (unsigned i = 1; i < len; ++i) {
            auto cb = static_cast<unsigned char>(s[i]);
            if (!is_continuation_byte(static_cast<char8_t>(cb))) {
                return std::unexpected(unicode_errc::invalid_utf8_sequence);
            }
            cp = (cp << 6) | (cb & 0x3F);//NOLINT
        }
    }

    if (is_overlong(cp, len)) {
        return std::unexpected(unicode_errc::overlong_encoding);
    }
    if (!valid_code_point(cp)) {
        return std::unexpected(unicode_errc::invalid_code_point);
    }

    return std::pair{cp, len};
}

}// namespace star::unicode::detail

namespace star::unicode {

constexpr auto utf8_length(std::span<const char8_t> s) noexcept
  -> std::expected<std::size_t, unicode_errc> {
    std::size_t count = 0;
    auto it = s.begin();
    while (it < s.end()) {
        auto result = detail::decode_with_length(std::span(it, s.end() - it));
        if (!result) {
            return std::unexpected(result.error());
        }
        ++count;
        it += result->second;//NOLINT
    }
    return count;
}

constexpr auto utf8_length(const char8_t* null_terminated) noexcept
  -> std::expected<std::size_t, unicode_errc> {
    std::size_t count = 0;
    while (*null_terminated != char8_t{}) {
        auto result = detail::decode_with_length(std::span(null_terminated, 4));// 最多4字节
        if (!result) {
            return std::unexpected(result.error());
        }
        ++count;
        null_terminated += result->second;
    }
    return count;
}

constexpr auto decode_utf8_char(std::span<const char8_t> s) noexcept
  -> std::expected<char32_t, unicode_errc> {
    auto result = detail::decode_with_length(s);
    if (result) {
        return result->first;
    }
    return std::unexpected(result.error());
}

constexpr auto encode_utf8_char(char32_t cp) noexcept
  -> std::expected<std::array<char8_t, 4>, unicode_errc> {
    if (!detail::valid_code_point(cp)) {
        return std::unexpected(unicode_errc::invalid_code_point);
    }

    std::array<char8_t, 4> out{};
    if (cp < 0x80) {
        out[0] = static_cast<char8_t>(cp);
        return out;
    }
    if (cp < 0x800) {
        out[0] = static_cast<char8_t>(0xC0 | (cp >> 6));  //NOLINT
        out[1] = static_cast<char8_t>(0x80 | (cp & 0x3F));//NOLINT
        return out;
    }
    if (cp < 0x10000) {
        out[0] = static_cast<char8_t>(0xE0 | (cp >> 12));        //NOLINT
        out[1] = static_cast<char8_t>(0x80 | ((cp >> 6) & 0x3F));//NOLINT
        out[2] = static_cast<char8_t>(0x80 | (cp & 0x3F));       //NOLINT
        return out;
    }
    // cp <= 0x10FFFF
    out[0] = static_cast<char8_t>(0xF0 | (cp >> 18));         //NOLINT
    out[1] = static_cast<char8_t>(0x80 | ((cp >> 12) & 0x3F));//NOLINT
    out[2] = static_cast<char8_t>(0x80 | ((cp >> 6) & 0x3F)); //NOLINT
    out[3] = static_cast<char8_t>(0x80 | (cp & 0x3F));        //NOLINT
    return out;
}

auto utf8_to_utf32(std::span<const char8_t> s) -> std::expected<std::u32string, unicode_errc> {
    auto len = utf8_length(s);
    if (!len) {
        return std::unexpected(len.error());
    }
    std::u32string result;
    result.reserve(*len);
    auto it = s.begin();
    while (it < s.end()) {
        auto dec = detail::decode_with_length(std::span(it, s.end() - it));
        if (!dec) {
            return std::unexpected(dec.error());// 传递错误
        }
        result.push_back(dec->first);
        it += dec->second;//NOLINT
    }
    return result;
}

auto utf32_to_utf8(std::span<const char32_t> s) -> std::expected<std::u8string, unicode_errc> {
    std::u8string result;
    for (char32_t cp : s) {
        auto enc = encode_utf8_char(cp);
        if (!enc) {
            return std::unexpected(enc.error());
        }
        result.append(enc->begin(), enc->end());
    }
    return result;
}

constexpr auto hex_to_utf32(std::string_view hex) noexcept
  -> std::expected<char32_t, unicode_errc> {
    char32_t cp = 0;
    for (char ch : hex) {
        int digit;
        if (ch >= '0' && ch <= '9') {
            digit = ch - '0';
        } else if (ch >= 'a' && ch <= 'f') {
            digit = ch - 'a' + 10;
        } else if (ch >= 'A' && ch <= 'F') {
            digit = ch - 'A' + 10;
        } else {
            return std::unexpected(unicode_errc::invalid_hex_digit);
        }
        cp = (cp << 4) | static_cast<char32_t>(digit);//NOLINT
    }
    if (!detail::valid_code_point(cp)) {
        return std::unexpected(unicode_errc::invalid_code_point);
    }
    return cp;
}

constexpr auto utf32_to_hex(char32_t cp) -> std::string {
    std::array<char, 11> buf{};
    auto [ptr, ec] =
      std::to_chars(buf.data(), buf.data() + buf.size(), static_cast<std::uint32_t>(cp), 16);
    return {buf.data(), static_cast<std::size_t>(ptr - buf.data())};
}

constexpr auto is_utf16_lead_surrogate(char32_t cp) noexcept -> bool {
    return cp >= 0xD800 && cp <= 0xDBFF;
}
constexpr auto is_utf16_trail_surrogate(char32_t cp) noexcept -> bool {
    return cp >= 0xDC00 && cp <= 0xDFFF;
}

constexpr auto utf16_surrogate_pair_to_utf32(char32_t lead, char32_t trail) noexcept
  -> std::expected<char32_t, unicode_errc> {
    if (!is_utf16_lead_surrogate(lead) || !is_utf16_trail_surrogate(trail)) {
        return std::unexpected(unicode_errc::missing_surrogate);
    }
    return 0x10000 + ((lead & 0x3FF) << 10) + (trail & 0x3FF);//NOLINT
}

constexpr auto utf32_to_utf16_surrogate_pair(char32_t cp) noexcept
  -> std::pair<char32_t, std::optional<char32_t>> {
    if (cp < 0x10000 || cp > 0x10FFFF) {
        return {cp, std::nullopt};
    }
    cp -= 0x10000;
    char32_t lead = 0xD800 + (cp >> 10);   //NOLINT
    char32_t trail = 0xDC00 + (cp & 0x3FF);//NOLINT
    return {lead, trail};
}

template <std::bidirectional_iterator BaseIt>
    requires std::same_as<std::iter_value_t<BaseIt>, char8_t>
void utf8_to_utf32_iterator<BaseIt>::decode() const {
    auto it = m_pos;
    auto result = detail::decode_with_length(std::span(&*it, 4));
    if (result) {
        m_cached = result->first;
    } else {
        m_cached = REPLACEMENT_CHAR;
    }
}

template <std::bidirectional_iterator BaseIt>
    requires std::same_as<std::iter_value_t<BaseIt>, char8_t>
auto utf8_to_utf32_iterator<BaseIt>::operator*() const -> reference {
    if (!m_cached.has_value()) {
        decode();
    }
    return *m_cached;
}

template <std::bidirectional_iterator BaseIt>
    requires std::same_as<std::iter_value_t<BaseIt>, char8_t>
auto utf8_to_utf32_iterator<BaseIt>::operator->() const -> pointer {
    if (!m_cached.has_value()) {
        decode();
    }
    return &*m_cached;
}

template <std::bidirectional_iterator BaseIt>
    requires std::same_as<std::iter_value_t<BaseIt>, char8_t>
void utf8_to_utf32_iterator<BaseIt>::advance_one_code_point() {
    m_cached.reset();
    auto it = m_pos;
    auto result = detail::decode_with_length(std::span(&*it, 4));
    if (result) {
        std::advance(m_pos, result->second);
    } else {
        // 解码失败：跳过当前首字节（保守策略）
        ++m_pos;
    }
}

template <std::bidirectional_iterator BaseIt>
    requires std::same_as<std::iter_value_t<BaseIt>, char8_t>
auto utf8_to_utf32_iterator<BaseIt>::operator++() -> utf8_to_utf32_iterator& {
    advance_one_code_point();
    return *this;
}

template <std::bidirectional_iterator BaseIt>
    requires std::same_as<std::iter_value_t<BaseIt>, char8_t>
auto utf8_to_utf32_iterator<BaseIt>::operator++(int) -> utf8_to_utf32_iterator {
    auto tmp = *this;
    ++*this;
    return tmp;
}

template <std::bidirectional_iterator BaseIt>
    requires std::same_as<std::iter_value_t<BaseIt>, char8_t>
void utf8_to_utf32_iterator<BaseIt>::retreat_one_code_point() {
    m_cached.reset();
    while ((static_cast<unsigned char>(*--m_pos) & 0xC0) == 0x80) {}//NOLINT
}

template <std::bidirectional_iterator BaseIt>
    requires std::same_as<std::iter_value_t<BaseIt>, char8_t>
auto utf8_to_utf32_iterator<BaseIt>::operator--() -> utf8_to_utf32_iterator& {
    retreat_one_code_point();
    return *this;
}

template <std::bidirectional_iterator BaseIt>
    requires std::same_as<std::iter_value_t<BaseIt>, char8_t>
auto utf8_to_utf32_iterator<BaseIt>::operator--(int) -> utf8_to_utf32_iterator {
    auto tmp = *this;
    --*this;
    return tmp;
}

template class utf8_to_utf32_iterator<const char8_t*>;

}// namespace star::unicode
