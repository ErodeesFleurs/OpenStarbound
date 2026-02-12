export module star.unicode;

import std;

export namespace star::unicode {

inline constexpr char32_t REPLACEMENT_CHAR = U'\u00b7';

enum class unicode_errc : std::uint8_t {
    success = 0,
    invalid_utf8_sequence,
    truncated_utf8,
    overlong_encoding,
    invalid_code_point,
    missing_surrogate,
    invalid_hex_digit
};

};// namespace star::unicode

template <> struct std::is_error_code_enum<star::unicode::unicode_errc> : true_type {};

namespace star::unicode {
inline auto make_error_code(unicode_errc e) noexcept -> std::error_code {
    static struct : std::error_category {
        [[nodiscard]] auto name() const noexcept -> const char* override { return "unicode"; }
        [[nodiscard]] auto message(int ev) const -> std::string override {
            using enum unicode_errc;
            switch (static_cast<unicode_errc>(ev)) {
            case success: return "success";
            case invalid_utf8_sequence: return "invalid UTF-8 sequence";
            case truncated_utf8: return "truncated UTF-8";
            case overlong_encoding: return "overlong encoding";
            case invalid_code_point: return "invalid code point";
            case missing_surrogate: return "missing surrogate";
            case invalid_hex_digit: return "invalid hex digit";
            default: return "unknown";
            }
        }
    } category;
    return {static_cast<int>(e), category};
}
}// namespace star::unicode

export namespace star::unicode {

[[nodiscard]] constexpr auto utf8_length(std::span<const char32_t> s) noexcept
  -> std::expected<std::size_t, unicode_errc>;

[[nodiscard]] constexpr auto utf8_length(const char8_t* null_terminated) noexcept
  -> std::expected<std::size_t, unicode_errc>;

[[nodiscard]] constexpr auto decode_utf8_char(std::span<const char8_t> s) noexcept
  -> std::expected<char32_t, unicode_errc>;

[[nodiscard]] constexpr auto encode_utf8_char(char32_t codepoint) noexcept
  -> std::expected<std::array<char8_t, 4>, unicode_errc>;

[[nodiscard]] auto utf8_to_utf32(std::span<const char8_t> s)
  -> std::expected<std::u32string, unicode_errc>;

[[nodiscard]] auto utf32_to_utf8(std::span<const char32_t> s)
  -> std::expected<std::u8string, unicode_errc>;

[[nodiscard]] constexpr auto hex_to_utf32(std::string_view hex) noexcept
  -> std::expected<char32_t, unicode_errc>;

[[nodiscard]] constexpr auto utf32_to_hex(char32_t codepoint) -> std::string;

[[nodiscard]] constexpr auto is_utf16_lead_surrogate(char32_t cp) noexcept -> bool;
[[nodiscard]] constexpr auto is_utf16_trail_surrogate(char32_t cp) noexcept -> bool;

[[nodiscard]] constexpr auto utf16_surrogate_pair_to_utf32(char32_t lead, char32_t trail) noexcept
  -> std::expected<char32_t, unicode_errc>;
[[nodiscard]] constexpr auto utf32_to_utf16_surrogate_pair(char32_t cp) noexcept
  -> std::pair<char32_t, std::optional<char32_t>>;

template <std::bidirectional_iterator BaseIt>
    requires std::same_as<std::iter_value_t<BaseIt>, char8_t>
class utf8_to_utf32_iterator {
  public:
    using iterator_concept = std::bidirectional_iterator_tag;
    using value_type = char32_t;
    using difference_type = std::iter_difference_t<BaseIt>;
    using pointer = const value_type*;
    using reference = const value_type&;

    utf8_to_utf32_iterator() = default;
    explicit utf8_to_utf32_iterator(BaseIt pos) : m_pos(pos) {}

    auto operator*() const -> reference;
    auto operator->() const -> pointer;

    auto operator++() -> utf8_to_utf32_iterator&;
    auto operator++(int) -> utf8_to_utf32_iterator;
    auto operator--() -> utf8_to_utf32_iterator&;
    auto operator--(int) -> utf8_to_utf32_iterator;

    auto operator==(const utf8_to_utf32_iterator& other) const -> bool = default;

  private:
    static constexpr auto skip_invalid_utf8(BaseIt& it) -> std::size_t;
    void decode() const;
    void advance_one_code_point();
    void retreat_one_code_point();

    BaseIt m_pos;
    mutable std::optional<char32_t> m_cached;
};

template <std::ranges::bidirectional_range R>
    requires std::same_as<std::ranges::range_value_t<R>, char8_t>
class utf8_view : public std::ranges::view_interface<utf8_view<R>> {
    R m_base;

  public:
    utf8_view() = default;
    explicit utf8_view(R base) : m_base(std::move(base)) {}

    auto begin() const { return utf8_to_utf32_iterator{std::ranges::begin(m_base)}; }
    auto end() const { return utf8_to_utf32_iterator{std::ranges::end(m_base)}; }
};

template <std::ranges::viewable_range R> utf8_view(R&&) -> utf8_view<std::views::all_t<R>>;

inline constexpr auto as_utf32 = std::views::transform(
  [](auto&& rng) -> auto { return utf8_view{std::views::all(std::forward<decltype(rng)>(rng))}; });

template <std::output_iterator<char8_t> OutIt> class utf8_encoder {
    OutIt m_out;

  public:
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = void;

    explicit utf8_encoder(OutIt out) : m_out(std::move(out)) {}

    auto operator*() noexcept -> utf8_encoder& { return *this; }

    auto operator=(char32_t cp) noexcept -> utf8_encoder& {
        if (auto encoded = encode_utf8_char(cp); encoded.has_value()) {
            m_out = std::ranges::copy(encoded->begin(), encoded->end(), m_out).out;
        }
        return *this;
    }

    auto operator++() noexcept -> utf8_encoder& { return *this; }
    auto operator++(int) noexcept -> utf8_encoder& { return *this; }
};

template <std::output_iterator<char8_t> OutIt> auto make_utf8_encoder(OutIt out) {
    return utf8_encoder<OutIt>(std::move(out));
}

}// namespace star::unicode
