module;

export module star.array;

import std;
import star.hash;

export namespace star {

template <typename ElementT, std::size_t SizeN> class array : public std::array<ElementT, SizeN> {
  public:
    using Base = std::array<ElementT, SizeN>;
    using Element = ElementT;
    static constexpr std::size_t arraySize = SizeN;

    constexpr array() : Base{} {};

    template <typename... Args>
        requires(sizeof...(Args) == SizeN) && (std::convertible_to<Args, ElementT> && ...)
    constexpr explicit array(Args&&... args)
        : Base{static_cast<ElementT>(std::forward<Args>(args))...} {}

    template <typename Element2> constexpr explicit array(array<Element2, SizeN> const& a) {
        std::ranges::copy(a, this->begin());
    }

    [[nodiscard]] static constexpr auto filled(Element const& e) -> array {
        array a;
        a.fill(e);
        return a;
    }

    [[nodiscard]] static constexpr auto copy_from(std::span<const ElementT> source) -> array {
        array a;
        std::ranges::copy(source | std::views::take(SizeN), a.begin());
        return a;
    }

    template <std::size_t I> [[nodiscard]] constexpr auto get(this auto&& self) noexcept -> auto&& {
        static_assert(I < SizeN, "array index out of bounds");
        return std::get<I>(std::forward<decltype(self)>(self));
    }

    [[nodiscard]] constexpr auto ptr(this auto&& self) noexcept -> auto* { return self.data(); }

    auto operator<=>(array const&) const = default;

    template <std::size_t Size2>
        requires(Size2 <= SizeN)
    [[nodiscard]] constexpr auto to_size(this array const& self) -> array<Element, Size2> {
        array<Element, Size2> r;
        std::ranges::copy(self | std::views::take(Size2), r.begin());
        return r;
    }

    template <std::size_t I> friend constexpr auto get(array& a) noexcept -> auto&& {
        return a.template get<I>();
    }
    template <std::size_t I> friend constexpr auto get(array const& a) noexcept -> auto&& {
        return a.template get<I>();
    }
    template <std::size_t I> friend constexpr auto get(array&& a) noexcept -> auto&& {
        return std::move(a).template get<I>();
    }
};

template <typename DataT, std::size_t SizeT> struct hash<array<DataT, SizeT>> {
    auto operator()(array<DataT, SizeT> const& a) const -> std::size_t {
        std::size_t hashval = 0;
        star::hash<DataT> dataHasher;
        for (auto const& element : a) {
            hashCombine(hashval, dataHasher(element));
        }
        return hashval;
    }
};

template <typename Element, std::size_t Size>
auto operator<<(std::ostream& os, array<Element, Size> const& a) -> std::ostream& {
    os << '[';
    for (std::size_t i = 0; i < Size; ++i) {
        if (i != 0) {
            os << ", ";
        }
        os << a[i];
    }
    os << ']';
    return os;
}

using array2I = array<std::int32_t, 2>;
using array2S = array<std::size_t, 2>;
using array2U = array<std::uint32_t, 2>;
using array2F = array<std::float_t, 2>;
using array2D = array<std::double_t, 2>;

using array3I = array<std::int32_t, 3>;
using array3S = array<std::size_t, 3>;
using array3U = array<std::uint32_t, 3>;
using array3F = array<std::float_t, 3>;
using array3D = array<std::double_t, 3>;

using array4I = array<std::int32_t, 4>;
using array4S = array<std::size_t, 4>;
using array4U = array<std::uint32_t, 4>;
using array4F = array<std::float_t, 4>;
using array4D = array<std::double_t, 4>;

}// namespace star

template <typename Element, std::size_t Size> struct std::formatter<star::array<Element, Size>> {
    std::formatter<Element> element_formatter;
    constexpr auto parse(std::format_parse_context& ctx) { return element_formatter.parse(ctx); }
    auto format(star::array<Element, Size> const& a, std::format_context& ctx) const {
        auto out = ctx.out();
        *out++ = '[';
        for (std::size_t i = 0; i < Size; ++i) {
            if (i > 0) {
                *out++ = ',';
                *out++ = ' ';
            }
            ctx.advance_to(out);
            out = element_formatter.format(a[i], ctx);
        }
        *out++ = ']';
        return out;
    }
};

namespace std {
template <typename Element, size_t Size>
struct tuple_size<star::array<Element, Size>> : integral_constant<size_t, Size> {};

template <size_t I, typename Element, size_t Size>
struct tuple_element<I, star::array<Element, Size>> {
    using type = Element;
};

}// namespace std
