module star.lexical_cast;

import std;

namespace star {

// 核心：统一解析逻辑
template <typename T, typename CharT>
constexpr auto common_lexical_cast(std::basic_string_view<CharT> s) noexcept
  -> std::expected<T, lexical_cast_error> {
    if constexpr (std::is_same_v<T, bool>) {
        if (s == (const CharT*)u8"true") {
            return true;
        }
        if (s == (const CharT*)u8"false") {
            return false;
        }
        return std::unexpected(
          lexical_cast_error{.ec = std::make_error_code(std::errc::invalid_argument),
                             .input = std::string_view(to_char_ptr(s.data()), s.size())});
    } else if constexpr (std::is_arithmetic_v<T>) {
        T value{};
        const char* first = to_char_ptr(s.data());
        const char* last = first + s.size();

        auto [ptr, ec] = std::from_chars(first, last, value);

        if (ec == std::errc{} && ptr == last) {
            return value;
        }

        return std::unexpected(lexical_cast_error{
          .ec = std::make_error_code(ec == std::errc{} ? std::errc::invalid_argument : ec),
          .input = std::string_view(first, s.size())});
    }
}

}// namespace star
