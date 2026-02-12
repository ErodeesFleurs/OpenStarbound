export module star.format;

import std;

export namespace star {

template <typename T> constexpr auto to_formattable(T&& arg) -> decltype(auto) {
    using D = std::decay_t<T>;
    if constexpr (std::is_same_v<D, std::u8string> || std::is_same_v<D, std::u8string_view>) {
        return std::string_view(reinterpret_cast<const char*>(arg.data()), arg.size());
    } else if constexpr (std::is_same_v<D, const char8_t*> || std::is_same_v<D, char8_t*>) {
        return reinterpret_cast<const char*>(arg);
    } else {
        return std::forward<T>(arg);
    }
}

template <typename... Args> struct u8_format_string {
    std::string_view fmt_as_char;

    template <std::size_t N>
    constexpr u8_format_string(const char8_t (&str)[N])// NOLINT
        : fmt_as_char(reinterpret_cast<const char*>(str), N - 1) {}

    constexpr explicit u8_format_string(std::string_view v) : fmt_as_char(v) {}
};

template <typename... Args>
[[nodiscard]] auto vformat(u8_format_string<std::type_identity_t<Args>...> fmt, Args&&... args) {
    auto result = [&fmt](auto&&... transformed_args) -> auto {
        return std::vformat(fmt.fmt_as_char, std::make_format_args(transformed_args...));
    }(to_formattable(std::forward<Args>(args))...);
    return std::u8string(reinterpret_cast<const char8_t*>(result.data()), result.size());
}

template <typename... Args>
[[nodiscard]] auto vformat(std::format_string<std::type_identity_t<Args>...> fmt, Args&&... args) {
    return std::vformat(fmt, std::make_format_args(args...));
}

template <typename... Args>
[[nodiscard]] auto format(std::format_string<std::type_identity_t<Args>...> fmt, Args&&... args) {
    return std::format(fmt, std::forward<Args>(args)...);
}

}// namespace star

template <> struct std::formatter<std::u8string, char> : std::formatter<std::string_view, char> {
    auto format(const std::u8string& s, format_context& ctx) const {
        return std::formatter<std::string_view, char>::format(
          std::string_view(reinterpret_cast<const char*>(s.data()), s.size()), ctx);
    }
};

template <>
struct std::formatter<std::u8string_view, char> : std::formatter<std::string_view, char> {
    auto format(std::u8string_view s, format_context& ctx) const {
        return std::formatter<std::string_view, char>::format(
          std::string_view(reinterpret_cast<const char*>(s.data()), s.size()), ctx);
    }
};
