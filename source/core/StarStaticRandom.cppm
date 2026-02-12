export module star.static_random;

import std;

export namespace star {

constexpr std::uint64_t fnv_offset_basis = 0xcbf29ce484222325ULL;
constexpr std::uint64_t fnv_prime = 0x100000001b3ULL;

constexpr void fnv_push(std::uint64_t& hash, std::span<std::byte const> data) noexcept {
    for (auto byte : data) {
        hash ^= static_cast<std::uint8_t>(byte);
        hash *= fnv_prime;
    }
}

template <typename T> constexpr void static_push_value(std::uint64_t& hash, T const& v) noexcept {
    if constexpr (std::same_as<T, std::string>) {
        fnv_push(hash, {reinterpret_cast<std::byte const*>(v.data()), v.size()});
    } else if constexpr (std::is_enum_v<T>) {
        auto val = static_cast<std::underlying_type_t<T>>(v);
        fnv_push(hash, std::as_bytes(std::span{&val, 1}));
    } else if constexpr (std::is_standard_layout_v<T> && std::is_trivial_v<T>) {
        fnv_push(hash, std::as_bytes(std::span{&v, 1}));
    } else {
        fnv_push(hash, std::as_bytes(std::span{reinterpret_cast<std::byte const*>(&v), sizeof(v)}));
    }
}

template <typename... args_t>
[[nodiscard]] constexpr auto static_random_u64(args_t const&... args) noexcept -> std::uint64_t {
    std::uint64_t hash = 1997293021376312589ULL;
    (static_push_value(hash, args), ...);
    return hash;
}

template <typename... args_t>
[[nodiscard]] constexpr auto static_random_u32(args_t const&... args) noexcept -> std::uint32_t {
    std::uint64_t h = static_random_u64(args...);
    return static_cast<std::uint32_t>(h ^ (h >> 32));//NOLINT
}

template <typename... args_t>
[[nodiscard]] constexpr auto static_random_i32(args_t const&... args) noexcept -> std::int32_t {
    return static_cast<std::int32_t>(static_random_u32(args...));
}

template <typename... args_t>
[[nodiscard]] constexpr auto static_random_i32_range(std::int32_t min, std::int32_t max,
                                                     args_t const&... args) noexcept
  -> std::int32_t {
    std::uint64_t range = static_cast<std::uint64_t>(max) - min + 1;
    std::uint64_t denom = std::numeric_limits<std::uint64_t>::max() / range;
    return static_cast<std::int32_t>(static_random_u64(args...) / denom + min);
}

template <typename... args_t>
[[nodiscard]] constexpr auto static_random_u32_range(std::uint32_t min, std::uint32_t max,
                                                     args_t const&... args) noexcept
  -> std::uint32_t {
    std::uint64_t range = static_cast<std::uint64_t>(max) - min + 1;
    std::uint64_t denom = std::numeric_limits<std::uint64_t>::max() / range;
    return static_cast<std::uint32_t>(static_random_u64(args...) / denom + min);
}

template <typename... args_t>
[[nodiscard]] constexpr auto static_random_i64(args_t const&... args) noexcept -> std::int64_t {
    return static_cast<std::int64_t>(static_random_u64(args...));
}

// Generates values in the range [0.0, 1.0]
template <typename... args_t>
[[nodiscard]] constexpr auto static_random_float(args_t const&... args) noexcept -> float {
    return static_cast<float>(static_random_u32(args...) & 0x7fffffff) / 2147483648.0F;
}

template <typename... args_t>
[[nodiscard]] constexpr auto static_random_float_range(float min, float max,
                                                       args_t const&... args) noexcept -> float {
    return static_random_float(args...) * (max - min) + min;
}

// Generates values in the range [0.0, 1.0]
template <typename... args_t>
[[nodiscard]] constexpr auto static_random_double(args_t const&... args) noexcept -> double {
    return static_cast<double>(static_random_u64(args...) & 0x7fffffffffffffff)
      / 9223372036854775808.0;
}

template <typename... args_t>
[[nodiscard]] constexpr auto static_random_double_range(double min, double max,
                                                        args_t const&... args) noexcept -> double {
    return static_random_double(args...) * (max - min) + min;
}

template <std::ranges::forward_range container_t, typename... args_t>
[[nodiscard]] constexpr auto static_random_from(container_t&& container, args_t const&... args)
  -> std::ranges::range_reference_t<container_t> {
    auto size = std::ranges::distance(container);
    auto i = std::ranges::begin(container);
    std::ranges::advance(i,
                         static_random_i32_range(0, static_cast<std::int32_t>(size) - 1, args...));
    return *i;
}

template <std::ranges::forward_range container_t, typename... args_t>
[[nodiscard]] constexpr auto static_random_value_from(container_t const& container,
                                                      args_t const&... args)
  -> std::ranges::range_value_t<container_t> {
    if (std::ranges::empty(container)) {
        return {};
    }
    auto size = std::ranges::distance(container);
    auto i = std::ranges::begin(container);
    std::ranges::advance(i,
                         static_random_i32_range(0, static_cast<std::int32_t>(size) - 1, args...));
    return *i;
}

template <typename F, typename result_t> class urbg {
  public:
    using result_type = result_t;

    explicit constexpr urbg(F&& func) : m_func(std::move(func)) {}

    [[nodiscard]] static constexpr auto min() noexcept -> result_t {
        return std::numeric_limits<result_t>::min();
    }
    [[nodiscard]] static constexpr auto max() noexcept -> result_t {
        return std::numeric_limits<result_t>::max();
    }

    constexpr auto operator()() -> result_t { return m_func(); }

  private:
    F m_func{};
};

template <typename result_t = std::uint32_t, typename F> constexpr auto make_urbg(F&& func) {
    return constexpr_urbg<std::decay_t<F>, result_t>(std::forward<F>(func));
}

template <std::ranges::random_access_range container_t, typename... args_t>
constexpr void static_random_shuffle(container_t& container, args_t const&... args) {
    auto size = std::ranges::distance(container);
    if (size <= 1) {
        return;
    }

    for (int i = 1, mix = 0; i < size; ++i) {
        int off = static_cast<int>(static_random_u32_range(0, i, ++mix, args...));
        if (off != i) {
            std::ranges::swap(container[i], container[off]);
        }
    }
}

}// namespace star
