export module star.rect;

import std;
import star.line;
import star.vector;

export namespace star {

template <typename T, std::size_t N> class box {
  public:
    using coord = star::vector<T, N>;
    using line = star::line<T, N>;
    using line_intersect_result = typename line::intersect_result;

    struct intersect_result {
        bool intersects = false;
        coord overlap{};
        bool glances = false;
    };

    [[nodiscard]] static constexpr auto null() noexcept -> box {
        return {coord::filled(std::numeric_limits<T>::max()),
                coord::filled(std::numeric_limits<T>::lowest())};
    }

    [[nodiscard]] static constexpr auto inf() noexcept -> box {
        return {coord::filled(std::numeric_limits<T>::lowest()),
                coord::filled(std::numeric_limits<T>::max())};
    }

    template <typename box2>
    [[nodiscard]] static constexpr auto integral(const box2& b) noexcept -> box {
        coord min_v, max_v;
        for (std::size_t i = 0; i < N; ++i) {
            min_v[i] = std::floor(b.min()[i]);
            max_v[i] = std::ceil(b.max()[i]);
        }
        return {min_v, max_v};
    }

    template <typename box2>
    [[nodiscard]] static constexpr auto round(const box2& b) noexcept -> box {
        coord min_v, max_v;
        for (std::size_t i = 0; i < N; ++i) {
            min_v[i] = std::round(b.min()[i]);
            max_v[i] = std::round(b.max()[i]);
        }
        return {min_v, max_v};
    }

    template <typename... tn>
    [[nodiscard]] static constexpr auto boundbox_of(const tn&... list) noexcept -> box {
        box b = null();
        (b.combine(list), ...);
        return b;
    }

    template <std::ranges::input_range R>
    [[nodiscard]] static constexpr auto boundbox_of_points(R&& range) noexcept -> box {
        box b = null();
        for (auto const& p : range) {
            b.combine(Coord(p));
        }
        return b;
    }

    [[nodiscard]] static constexpr auto with_size(const coord& min, const coord& size) noexcept
      -> box {
        return {min, min + size};
    }

    [[nodiscard]] static constexpr auto with_center(const coord& center, const coord& size) noexcept
      -> box {
        return {center - size / T(2), center + size / T(2)};
    }

    constexpr box() noexcept = default;
    constexpr box(const coord& min, const coord& max) noexcept : m_min(min), m_max(max) {}

    template <typename T2>
    explicit constexpr box(box<T2, N> const& b) noexcept
        : m_min(Coord(b.min())), m_max(Coord(b.max())) {}

    [[nodiscard]] constexpr auto is_null() const noexcept -> bool {
        return m_min == coord::filled(std::numeric_limits<T>::max())
          && m_max == coord::filled(std::numeric_limits<T>::lowest());
    }

    [[nodiscard]] constexpr auto is_negative() const noexcept -> bool {
        for (std::size_t i = 0; i < N; ++i) {
            if (m_max[i] < m_min[i]) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] constexpr auto is_empty() const noexcept -> bool {
        for (std::size_t i = 0; i < N; ++i) {
            if (m_max[i] <= m_min[i]) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] constexpr auto size() const noexcept -> coord { return m_max - m_min; }
    [[nodiscard]] constexpr auto size(std::size_t dim) const noexcept -> T {
        return m_max[dim] - m_min[dim];
    }

    [[nodiscard]] constexpr auto center() const noexcept -> coord { return (m_min + m_max) / T(2); }

    [[nodiscard]] constexpr auto volume() const noexcept -> T {
        T res = 1;
        auto s = size();
        for (std::size_t i = 0; i < N; ++i) {
            res *= s[i];
        }
        return res;
    }

    constexpr auto min(this auto&& self) noexcept -> auto&& { return self.m_min; }
    constexpr auto max(this auto&& self) noexcept -> auto&& { return self.m_max; }

    constexpr void set_min(const coord& c) noexcept { m_min = c; }
    constexpr void set_max(const coord& c) noexcept { m_max = c; }

    constexpr void combine(const box& other) noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            m_min[i] = std::min(m_min[i], other.m_min[i]);
            m_max[i] = std::max(m_max[i], other.m_max[i]);
        }
    }

    [[nodiscard]] constexpr auto combined(const box& other) const noexcept -> box {
        box b = *this;
        b.combine(other);
        return b;
    }

    constexpr void combine(const coord& p) noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            m_min[i] = std::min(m_min[i], p[i]);
            m_max[i] = std::max(m_max[i], p[i]);
        }
    }

    [[nodiscard]] constexpr auto combined(const coord& p) const noexcept -> box {
        box b = *this;
        b.combine(p);
        return b;
    }

    constexpr void limit(const box& other) noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            m_min[i] = std::max(m_min[i], other.m_min[i]);
            m_max[i] = std::min(m_max[i], other.m_max[i]);
        }
    }

    [[nodiscard]] constexpr auto limited(const box& other) const noexcept -> box {
        box b = *this;
        b.limit(other);
        return b;
    }

    constexpr void make_positive() noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            if (m_max[i] < m_min[i]) {
                std::swap(m_min[i], m_max[i]);
            }
        }
    }

    constexpr void translate(coord const& c) noexcept {
        m_min += c;
        m_max += c;
    }

    [[nodiscard]] constexpr auto translated(coord const& c) const noexcept -> box {
        box b = *this;
        b.translate(c);
        return b;
    }

    constexpr void scale(T factor) noexcept {
        coord c = center();
        m_min = c + (m_min - c) * factor;
        m_max = c + (m_max - c) * factor;
    }

    [[nodiscard]] constexpr auto scaled(T factor) const noexcept -> box {
        box b = *this;
        b.scale(factor);
        return b;
    }

    constexpr void pad(T amount) noexcept {
        m_min -= coord::filled(amount);
        m_max += coord::filled(amount);
    }

    [[nodiscard]] constexpr auto padded(T amount) const noexcept -> box {
        box b = *this;
        b.pad(amount);
        return b;
    }

    [[nodiscard]] constexpr auto overlap(box const& other) const noexcept -> box {
        box res = *this;
        res.limit(other);
        return res;
    }

    [[nodiscard]] constexpr auto intersection(box const& b) const noexcept -> intersect_result {
        intersect_result res;
        T min_overlap = std::numeric_limits<T>::max();
        std::size_t dim = 0;
        bool negative = false;

        for (std::size_t i = 0; i < N; ++i) {
            if (T o1 = m_max[i] - b.m_min[i]; o1 < min_overlap) {
                min_overlap = o1;
                dim = i;
                negative = true;
            }
            if (T o2 = b.m_max[i] - m_min[i]; o2 < min_overlap) {
                min_overlap = o2;
                dim = i;
                negative = false;
            }
        }

        res.intersects = (min_overlap > 0);
        res.overlap[dim] = negative ? -min_overlap : min_overlap;
        res.glances = near_zero(min_overlap);
        return res;
    }

    [[nodiscard]] constexpr auto intersects(box const& other,
                                            bool include_edges = true) const noexcept -> bool {
        for (std::size_t i = 0; i < N; ++i) {
            if (include_edges) {
                if (m_max[i] < other.m_min[i] || other.m_max[i] < m_min[i]) {
                    return false;
                }
            } else {
                if (m_max[i] <= other.m_min[i] || other.m_max[i] <= m_min[i]) {
                    return false;
                }
            }
        }
        return true;
    }

    [[nodiscard]] constexpr auto contains(coord const& p, bool include_edges = true) const noexcept
      -> bool {
        for (std::size_t i = 0; i < N; ++i) {
            if (include_edges) {
                if (p[i] < m_min[i] || p[i] > m_max[i]) {
                    return false;
                }
            } else {
                if (p[i] <= m_min[i] || p[i] >= m_max[i]) {
                    return false;
                }
            }
        }
        return true;
    }

    template <std::size_t P = N>
        requires(P == 2)
    constexpr box(T xmin, T ymin, T xmax, T ymax) noexcept : m_min(xmin, ymin), m_max(xmax, ymax) {}

    template <std::size_t P = N>
        requires(P == 2)
    [[nodiscard]] constexpr auto x_min() const noexcept -> T {
        return m_min[0];
    }
    template <std::size_t P = N>
        requires(P == 2)
    [[nodiscard]] constexpr auto x_max() const noexcept -> T {
        return m_max[0];
    }
    template <std::size_t P = N>
        requires(P == 2)
    [[nodiscard]] constexpr auto y_min() const noexcept -> T {
        return m_min[1];
    }
    template <std::size_t P = N>
        requires(P == 2)
    [[nodiscard]] constexpr auto y_max() const noexcept -> T {
        return m_max[1];
    }

    template <std::size_t P = N>
        requires(P == 2)
    [[nodiscard]] constexpr auto width() const noexcept -> T {
        return size(0);
    }
    template <std::size_t P = N>
        requires(P == 2)
    [[nodiscard]] constexpr auto height() const noexcept -> T {
        return size(1);
    }

    template <std::size_t P = N>
        requires(P == 2)
    [[nodiscard]] auto edges() const noexcept -> std::array<line, 4> {
        return {line{m_min, coord{m_min[0], m_max[1]}}, line{m_min, coord{m_max[0], m_min[1]}},
                line{coord{m_min[0], m_max[1]}, m_max}, line{coord{m_max[0], m_min[1]}, m_max}};
    }

    template <std::size_t P = N>
        requires(P == 2)
    [[nodiscard]] auto subtract(box const& rect) const -> std::vector<box> {
        std::vector<box> regions;
        auto ol = overlap(rect);
        if (ol.is_empty()) {
            regions.push_back(*this);
        } else {
            if (x_min() < ol.x_min()) {
                regions.emplace_back(x_min(), y_min(), ol.x_min(), y_max());
            }
            if (ol.x_max() < x_max()) {
                regions.emplace_back(ol.x_max(), y_min(), x_max(), y_max());
            }
            if (y_min() < ol.y_min()) {
                regions.emplace_back(rect.x_min(), y_min(), rect.x_max(), ol.y_min());
            }
            if (ol.y_max() < y_max()) {
                regions.emplace_back(rect.x_min(), ol.y_max(), rect.x_max(), y_max());
            }
        }
        return regions;
    }

  protected:
    coord m_min;
    coord m_max;
};

template <typename T> using rect = box<T, 2>;

using rect_i = rect<std::int32_t>;
using rect_u = rect<std::uint32_t>;
using rect_f = rect<std::float_t>;
using rect_d = rect<std::double_t>;

template <typename T, std::size_t N>
auto operator<<(std::ostream& os, box<T, N> const& b) -> std::ostream& {
    return os << std::format("box{{min: {}, max: {}}}", b.min(), b.max());
}

}// namespace star

template <typename T, std::size_t N> struct std::formatter<star::box<T, N>> : std::formatter<T> {
    auto format(star::box<T, N> const& b, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "box{{min: {}, max: {}}}", b.min(), b.max());
    }
};
