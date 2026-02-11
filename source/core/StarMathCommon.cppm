export module star.math_common;

import std;
import star.exception;

export namespace star {

using math_exception = exception_derived<"math_exception">;

// Bit Manipulation

// Count the number of '1' bits in the given unsigned integer
template <std::unsigned_integral Int>
[[nodiscard]] constexpr auto count_set_bits(Int value) noexcept -> unsigned {
    return static_cast<unsigned>(std::popcount(value));
}

template <std::integral Int> [[nodiscard]] constexpr auto is_power_of2(Int x) noexcept -> bool {
    if (x <= 0) {
        return false;
    }
    return std::has_single_bit(static_cast<std::make_unsigned_t<Int>>(x));
}

[[nodiscard]] constexpr auto ceil_power_of2(std::uint64_t v) noexcept -> std::uint64_t {
    if (v <= 1) {
        return 1;
    }
    return std::bit_ceil(v);
}

// Number Equality

template <typename T1, typename T2>
    requires(std::is_arithmetic_v<T1> && std::is_arithmetic_v<T2>)
[[nodiscard]] constexpr auto near_equal(T1 x, T2 y, unsigned ulp = 1) noexcept -> bool {
    if constexpr (std::integral<T1> && std::integral<T2>) {
        return x == y;
    } else {
        using Common = std::common_type_t<T1, T2, float>;
        auto cx = static_cast<Common>(x);
        auto cy = static_cast<Common>(y);
        auto epsilon = std::numeric_limits<Common>::epsilon();
        return std::abs(cx - cy)
          <= epsilon * std::max({std::abs(cx), std::abs(cy), Common(1)}) * ulp;
    }
}

template <std::floating_point T>
[[nodiscard]] constexpr auto near_zero(T x, unsigned ulp = 2) noexcept -> bool {
    return std::abs(x) <= std::numeric_limits<T>::min() * ulp;
}

// Misc Math Functions

template <typename T> [[nodiscard]] constexpr auto square(T const& x) noexcept -> T {
    return x * x;
}

template <typename T> [[nodiscard]] constexpr auto cube(T const& x) noexcept -> T {
    return x * x * x;
}

template <std::floating_point Float> [[nodiscard]] constexpr auto ipart(Float f) noexcept -> int {
    return static_cast<int>(std::floor(f));
}

template <std::floating_point Float> [[nodiscard]] constexpr auto fpart(Float f) noexcept -> Float {
    return f - std::floor(f);
}

template <typename T, typename T2>
[[nodiscard]] constexpr auto clamp_magnitude(T v, T2 mag) noexcept -> T {
    auto m = static_cast<T>(std::abs(mag));
    return std::clamp(v, -m, m);
}

template <typename T>
[[nodiscard]] constexpr auto clamp_dynamic(T val, T a, T b) noexcept
  -> T {// NOLINT(bugprone-easily-swappable-parameters)
    auto [min_v, max_v] = std::minmax(a, b);
    return std::clamp(val, min_v, max_v);
}

template <std::integral Int, std::integral Pow>
[[nodiscard]] constexpr auto int_pow(Int base, Pow exp) noexcept -> Int {
    Int res = 1;
    while (exp > 0) {
        if (exp & 1) {
            res *= base;
        }
        base *= base;
        exp >>= 1;
    }
    return res;
}

// Mod

template <std::integral T> [[nodiscard]] constexpr auto pmod(T a, T m) noexcept -> T {
    T r = a % m;
    return r < 0 ? r + m : r;
}

template <std::floating_point T> [[nodiscard]] constexpr auto pfmod(T a, T m) noexcept -> T {
    if (m == 0) {
        return a;
    }
    return a - m * std::floor(a / m);
}

// Ring Arithmetic

// Returns the smallest difference between a and b on a ring of the given size.
template <typename T> [[nodiscard]] constexpr auto wrap_diff(T a, T b, T size) noexcept -> T {
    T diff;
    if constexpr (std::integral<T>) {
        diff = pmod(a - b, size);
    } else {
        diff = pfmod(a - b, size);
    }

    if (diff > size / 2) {
        diff -= size;
    }
    return diff;
}

// Angle Utilities

template <std::floating_point T>
[[nodiscard]] constexpr auto constrain_angle(T angle) noexcept -> T {
    constexpr T pi = std::numbers::pi_v<T>;
    constexpr T two_pi = pi * 2;
    angle = pfmod(angle + pi, two_pi);
    return angle - pi;
}

template <std::floating_point T>
[[nodiscard]] constexpr auto angle_diff(T angle, T target) noexcept -> T {
    constexpr T pi = std::numbers::pi_v<T>;
    return constrain_angle(target - angle);
}

// Color Utilities

[[nodiscard]] constexpr auto float_to_byte(float val, bool doClamp = true) noexcept
  -> std::uint8_t {
    if (doClamp) {
        val = std::clamp(val, 0.0F, 1.0F);
    }
    return static_cast<std::uint8_t>(std::round(val * 255.0F));
}

[[nodiscard]] constexpr auto byte_to_float(std::uint8_t val) noexcept -> float {
    return static_cast<float>(val) / 255.0F;
}

// logical

template <std::integral T>
[[nodiscard]] constexpr auto cycle_increment(T val, T min_v, T max_v) noexcept -> T {
    if (val < min_v || val >= max_v) {
        return min_v;
    }
    return val + 1;
}

}// namespace star
