module;

export module star.vector;

import std;
import star.array;

export namespace star {

template <typename T, std::size_t N> class vector : public star::array<T, N> {
  public:
    using Base = star::array<T, N>;

    constexpr vector() = default;

    explicit constexpr vector(T const& e1)
        requires(N > 0)
    {
        this->fill(e1);
    }

    template <typename... TN>
        requires(sizeof...(TN) + 1 == N) && (std::convertible_to<TN, T> && ...)
    constexpr explicit vector(T const& e1, TN const&... rest) : Base{e1, static_cast<T>(rest)...} {}

    template <typename T2> constexpr explicit vector(star::array<T2, N> const& v) : Base(v) {}

    template <typename T2, typename T3>
    constexpr vector(star::array<T2, N - 1> const& u, T3 const& v) {
        std::ranges::copy(u, this->begin());
        (*this)[N - 1] = static_cast<T>(v);
    }

    static constexpr auto filled(T const& t) -> vector {
        vector v;
        v.fill(t);
        return v;
    }

    template <typename Self>
    [[nodiscard]] constexpr auto x(this Self&& self) -> auto&&
        requires(N >= 1)
    {
        return std::forward<Self>(self)[0];
    }

    template <typename Self>
    [[nodiscard]] constexpr auto y(this Self&& self) -> auto&&
        requires(N >= 2)
    {
        return std::forward<Self>(self)[1];
    }

    template <typename Self>
    [[nodiscard]] constexpr auto z(this Self&& self) -> auto&&
        requires(N >= 3)
    {
        return std::forward<Self>(self)[2];
    }

    template <typename Self>
    [[nodiscard]] constexpr auto w(this Self&& self) -> auto&&
        requires(N >= 4)
    {
        return std::forward<Self>(self)[3];
    }

    template <std::size_t M>
    [[nodiscard]] constexpr auto to_size(this vector const& self) -> vector<T, M>
        requires(M <= N)
    {
        vector<T, M> r;
        std::ranges::copy(self | std::views::take(M), r.begin());
        return r;
    }

    // Math operations
    [[nodiscard]] constexpr auto magnitude_squared() const -> T {
        T sum{0};
        for (auto v : *this) {
            sum += v * v;
        }
        return sum;
    }

    [[nodiscard]] constexpr auto magnitude() const -> T { return std::sqrt(magnitude_squared()); }

    constexpr void normalize() {
        if (T m = magnitude(); m > 0) {
            *this /= m;
        }
    }

    [[nodiscard]] constexpr auto normalized() const -> vector {
        vector r = *this;
        r.normalize();
        return r;
    }

    constexpr auto clamp(T min, T max) -> vector& {
        for (auto& v : *this) {
            v = std::clamp(v, min, max);
        }
        return *this;
    }

    [[nodiscard]] constexpr auto clamped(T min, T max) const -> vector {
        vector r;
        for (auto&& [res, v] : std::views::zip(r, *this)) {
            res = std::clamp(v, min, max);
        }
        return r;
    }

    // Dot
    constexpr auto operator*(vector const& v) const -> T {
        T res{0};
        for (auto&& [a, b] : std::views::zip(*this, v)) {
            res += a * b;
        }
        return res;
    }

    // Piecewise multiply
    [[nodiscard]] constexpr auto piecewise_multiply(vector const& v2) const -> vector {
        vector r;
        for (auto&& [res, a, b] : std::views::zip(r, *this, v2)) {
            res = a * b;
        }
        return r;
    }

    [[nodiscard]] constexpr auto sum() const -> T {
        return std::ranges::fold_left(*this, T{0}, std::plus<>{});
    }

    // 2D
    [[nodiscard]] constexpr auto rotate(T angle) const -> vector
        requires(N == 2)
    {
        T cosa = std::cos(angle), sina = std::sin(angle);
        return vector{(*this)[0] * cosa - (*this)[1] * sina, (*this)[0] * sina + (*this)[1] * cosa};
    }

    [[nodiscard]] constexpr auto angle() const -> T
        requires(N == 2)
    {
        return std::atan2((*this)[1], (*this)[0]);
    }

    // 3D
    static constexpr auto from_angles(T psi, T theta) -> vector
        requires(N == 3)
    {
        return vector{std::cos(psi) * std::cos(theta), std::sin(psi) * std::cos(theta),
                      std::sin(theta)};
    }

    // Cross
    constexpr auto operator^(vector const& v) const -> vector
        requires(N == 3)
    {
        return vector{(*this)[1] * v[2] - (*this)[2] * v[1], (*this)[2] * v[0] - (*this)[0] * v[2],
                      (*this)[0] * v[1] - (*this)[1] * v[0]};
    }

    // Operators
    constexpr auto operator+(vector const& v) const -> vector {
        vector r;
        for (auto&& [res, a, b] : std::views::zip(r, *this, v)) {
            res = a + b;
        }
        return r;
    }

    constexpr auto operator-(vector const& v) const -> vector {
        vector r;
        for (auto&& [res, a, b] : std::views::zip(r, *this, v)) {
            res = a - b;
        }
        return r;
    }

    constexpr auto operator*(T s) const -> vector {
        vector r;
        for (auto&& [res, a] : std::views::zip(r, *this)) {
            res = a * s;
        }
        return r;
    }

    constexpr auto operator/(T s) const -> vector {
        vector r;
        for (auto&& [res, a] : std::views::zip(r, *this)) {
            res = a / s;
        }
        return r;
    }

    constexpr auto operator+=(vector const& v) -> vector& {
        for (auto&& [a, b] : std::views::zip(*this, v)) {
            a += b;
        }
        return *this;
    }

    constexpr auto operator*=(T s) -> vector& {
        for (auto& a : *this) {
            a *= s;
        }
        return *this;
    }

    constexpr auto operator/=(T s) -> vector& {
        for (auto& a : *this) {
            a /= s;
        }
        return *this;
    }
};

using vec_2i = vector<std::int32_t, 2>;
using vec_2u = vector<std::uint32_t, 2>;
using vec_2f = vector<std::float_t, 2>;
using vec_2d = vector<std::double_t, 2>;
using vec_2b = vector<std::uint8_t, 2>;
using vec_2s = vector<std::size_t, 2>;

using vec_3i = vector<std::int32_t, 3>;
using vec_3u = vector<std::uint32_t, 3>;
using vec_3f = vector<std::float_t, 3>;
using vec_3d = vector<std::double_t, 3>;
using vec_3b = vector<std::uint8_t, 3>;
using vec_3s = vector<std::size_t, 3>;

using vec_4i = vector<std::int32_t, 4>;
using vec_4u = vector<std::uint32_t, 4>;
using vec_4f = vector<std::float_t, 4>;
using vec_4d = vector<std::double_t, 4>;
using vec_4b = vector<std::uint8_t, 4>;
using vec_4s = vector<std::size_t, 4>;

}// namespace star

template <typename T, std::size_t N> struct std::formatter<star::vector<T, N>> {
    std::formatter<T> element_formatter;

    constexpr auto parse(std::format_parse_context& ctx) { return element_formatter.parse(ctx); }

    auto format(star::vector<T, N> const& v, std::format_context& ctx) const {
        auto out = ctx.out();
        *out++ = '(';
        for (std::size_t i = 0; i < N; ++i) {
            if (i > 0) {
                *out++ = ',';
                *out++ = ' ';
            }
            ctx.advance_to(out);
            out = element_formatter.format(v[i], ctx);
        }
        *out++ = ')';
        return out;
    }
};
