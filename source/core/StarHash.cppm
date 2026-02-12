module;

export module star.hash;

import std;

export namespace star {

// To avoid having to specialize std::hash in the std namespace, which is
// slightly annoying, Star type wrappers use Star::hash, which just defaults to
// std::hash.  Star::hash also enables template specialization with a dummy
// Enable parameter.
template <typename T, typename Enable = void> struct hash : std::hash<T> {};

template <typename T> inline constexpr void hash_combine(std::size_t& seed, const T& v) {
    static constexpr std::size_t LeftShift = 6;
    static constexpr std::size_t RightShift = 2;

    constexpr std::size_t constant =
      (sizeof(std::size_t) == 8) ? 0x9e3779b97f4a7c15ULL : 0x9e3779b9ULL;
    std::size_t h = star::hash<T>{}(v);
    seed ^= h + constant + std::rotl(seed, LeftShift) + std::rotr(seed, RightShift);
}

// Paul Larson hashing algorithm, very very *cheap* hashing function.
class PLHasher {
  public:
    static constexpr std::size_t ShiftAmount = 5;

    constexpr explicit PLHasher(std::size_t initial = 0) : m_hash(initial) {}

    template <typename T>
        requires std::is_arithmetic_v<T> || std::is_enum_v<T>
    constexpr void put(T b) {
        m_hash = (m_hash << ShiftAmount) + m_hash + static_cast<std::size_t>(b);
    }

    [[nodiscard]] constexpr auto hash() const -> std::size_t { return m_hash; }

  private:
    std::size_t m_hash;
};

template <typename T1, typename T2> struct hash<std::pair<T1, T2>> {
    constexpr auto operator()(std::pair<T1, T2> const& p) const -> std::size_t {
        std::size_t h = hash<T1>{}(p.first);
        hash_combine(h, p.second);
        return h;
    }
};

template <typename... TTypes> struct hash<std::tuple<TTypes...>> {
    constexpr auto operator()(std::tuple<TTypes...> const& value) const -> std::size_t {
        return std::apply(
          [](auto const&... args) -> auto {
              std::size_t h = 0;
              (hash_combine(h, args), ...);
              return h;
          },
          value);
    }
};

template <typename T> struct hash<std::optional<T>> {
    constexpr auto operator()(std::optional<T> const& a) const -> std::size_t {
        return a ? hash<T>{}(*a) : 0;
    }
};

template <typename T> struct hash<T, std::enable_if_t<std::is_enum_v<T>>> {
    constexpr auto operator()(T e) const -> std::size_t {
        using Underlying = std::underlying_type_t<T>;
        return std::hash<Underlying>{}(static_cast<Underlying>(e));
    }
};

template <std::ranges::input_range R> struct hash<R> {
    auto operator()(R const& range) const -> std::size_t {
        std::size_t h = 0;
        for (auto const& item : range) {
            hash_combine(h, item);
        }
        return h;
    }
};

template <typename First, typename... Rest>
constexpr auto hash_of(First const& first, Rest const&... rest) -> std::size_t {
    std::size_t h = star::hash<First>{}(first);
    (hash_combine(h, rest), ...);
    return h;
}

// hash for compare
struct case_insensitive_string_hash {
    auto operator()(const std::string& s) const -> std::size_t {
        std::size_t h = 0;
        for (char c : s) {
            hash_combine(h, std::tolower(static_cast<unsigned char>(c)));
        }
        return h;
    }
};

struct case_insensitive_string_compare {
    auto operator()(const std::string& lhs, const std::string& rhs) const -> bool {
        return std::ranges::equal(lhs, rhs, [](char a, char b) -> bool {
            return std::tolower(static_cast<unsigned char>(a))
              == std::tolower(static_cast<unsigned char>(b));
        });
    }
};

}// namespace star
