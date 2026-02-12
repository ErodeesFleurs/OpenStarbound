export module star.lexical_cast;

import star.exception;
import std;

export namespace star {

struct lexical_cast_error {
    std::error_code ec;
    std::string_view input;
};

template <typename T>
concept lexical_castable = std::is_arithmetic_v<T> || std::is_same_v<T, bool>;

template <typename T, typename CharT>
constexpr auto common_lexical_cast(std::basic_string_view<CharT> s) noexcept
  -> std::expected<T, lexical_cast_error>;

template <typename T, typename CharT>
[[nodiscard]] auto lexical_cast_opt(std::basic_string_view<CharT> s) noexcept -> std::optional<T> {
    auto exp = common_lexical_cast<T, CharT>(s);
    if (exp) {
        return *exp;
    }
    return std::nullopt;
}

template <lexical_castable T>
constexpr auto lexical_cast(std::string_view s) noexcept -> std::expected<T, lexical_cast_error> {
    return lexical_cast<T, char>(s);
}

template <lexical_castable T>
constexpr auto lexical_cast(std::u8string_view s) noexcept -> std::expected<T, lexical_cast_error> {
    return common_lexical_cast<T>(s);
}
}// namespace star
