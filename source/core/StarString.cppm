export module star.string;

import std;

export namespace star {

[[nodiscard]] constexpr auto to_sv(std::u8string_view u8sv) noexcept -> std::string_view {
    return {reinterpret_cast<const char*>(u8sv.data()), u8sv.size()};
}

[[nodiscard]] constexpr auto to_u8sv(std::string_view sv) noexcept -> std::u8string_view {
    return {reinterpret_cast<const char8_t*>(sv.data()), sv.size()};
}

template <typename To, typename From>
    requires(sizeof(To) == 1 && sizeof(From) == 1)
[[nodiscard]] constexpr auto string_cast(const From* data,
                                         [[maybe_unused]] std::size_t size) noexcept {
    using to_char = std::iter_value_t<To>;
    return reinterpret_cast<const to_char*>(data);
}

template <typename To, typename From> [[nodiscard]] auto string_cast(const From& f) {

    if constexpr (std::is_same_v<To, std::string_view> || std::is_same_v<To, std::u8string_view>) {
        return To(reinterpret_cast<const typename To::value_type*>(f.data()), f.size());
    } else {
        return To(reinterpret_cast<const typename To::value_type*>(f.data()),
                  reinterpret_cast<const typename To::value_type*>(f.data() + f.size()));
    }
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
