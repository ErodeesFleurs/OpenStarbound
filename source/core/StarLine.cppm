export module star.line;

import std;
import star.hash;
import star.math_common;
import star.matrix3;
import star.vector;

export namespace star {

template <typename T, std::size_t N> class line {
  public:
    using vector_type = star::vector<T, N>;

    struct intersect_result {
        // Whether or not the two objects intersect
        bool intersects = false;
        // Where the intersection is (minimum value if intersection occurs in more
        // than one point.)
        vector_type point{};
        // T value where intersection occurs, 0 is min, 1 is max
        T t = std::numeric_limits<T>::max();
        // Whether or not the two lines, if they were infinite lines, are the exact
        // same line
        bool coincides = false;
        // Whether or not the intersection is a glancing one, meaning the other
        // line isn't actually skewered, it's just barely touching Coincidental
        // lines are always glancing intersections.
        bool glances = false;
    };

    constexpr line() = default;

    template <typename T2>
    constexpr explicit line(line<T2, N> const& line)
        : m_min(vector_type(line.min())), m_max(vector_type(line.max())) {}

    constexpr line(vector_type const& a, vector_type const& b) : m_min(a), m_max(b) {}

    [[nodiscard]] constexpr auto direction() const -> vector_type { return diff().normalized(); }
    [[nodiscard]] constexpr auto length() const -> T { return diff().magnitude(); }
    [[nodiscard]] constexpr auto angle() const -> T { return diff().angle(); }
    [[nodiscard]] constexpr auto eval(T t) const -> vector_type { return m_min + diff() * t; }
    [[nodiscard]] constexpr auto diff() const -> vector_type { return m_max - m_min; }
    [[nodiscard]] constexpr auto center() const -> vector_type { return (m_min + m_max) / T(2); }
    [[nodiscard]] constexpr auto midpoint() const -> vector_type { return center(); }

    constexpr void setCenter(vector_type c) { translate(c - center()); }

    [[nodiscard]] constexpr auto min() & -> vector_type& { return m_min; }
    [[nodiscard]] constexpr auto max() & -> vector_type& { return m_max; }
    [[nodiscard]] constexpr auto min() const& -> vector_type const& { return m_min; }
    [[nodiscard]] constexpr auto max() const& -> vector_type const& { return m_max; }

    constexpr auto makePositive() -> bool {
        for (std::size_t i = 0; i < N; ++i) {
            if (m_min[i] < m_max[i]) {
                return false;
            }
            if (m_min[i] > m_max[i]) {
                std::swap(m_min, m_max);
                return true;
            }
        }
        return false;
    }

    constexpr void reverse() { std::swap(m_min, m_max); }
    [[nodiscard]] constexpr auto reversed() const -> line { return Line(m_max, m_min); }

    constexpr void translate(vector_type const& trans) {
        m_min += trans;
        m_max += trans;
    }

    [[nodiscard]] constexpr auto translated(vector_type const& trans) const -> line {
        return Line(m_min + trans, m_max + trans);
    }

    constexpr void scale(vector_type const& s, vector_type const& c = vector_type()) {
        m_min = vmult(m_min - c, s) + c;
        m_max = vmult(m_max - c, s) + c;
    }

    constexpr void scale(T s, vector_type const& c = vector_type()) {
        scale(vector_type::filled(s), c);
    }

    auto operator<=>(line const&) const = default;

    // Line2

    [[nodiscard]] constexpr auto intersection(line const& line2, bool infinite = false) const
      -> intersect_result
        requires(N == 2)
    {
        const vector_type& a = m_min;
        const vector_type& b = m_max;
        const vector_type& c = line2.m_min;
        const vector_type& d = line2.m_max;

        const vector_type ab = diff();
        const vector_type cd = line2.diff();

        const T denom = ab ^ cd;
        const T xNumer = (a ^ b) * cd[0] - (c ^ d) * ab[0];
        const T yNumer = (a ^ b) * cd[1] - (c ^ d) * ab[1];

        intersect_result isect;
        if (near_zero(denom)) {
            if (near_zero(xNumer) && near_zero(yNumer)) {
                isect.intersects = infinite || (a >= c && a <= d) || (c >= a && c <= b);
                if (isect.intersects) {
                    if (infinite) {
                        isect.point = vector_type::filled(-std::numeric_limits<T>::max());
                    } else {
                        isect.point = (a < c) ? c : a;
                    }
                }
                if (a < c) {
                    isect.t = !near_zero(ab[0]) ? (c[0] - a[0]) / ab[0] : (c[1] - a[1]) / ab[1];
                } else if (a > d) {
                    isect.t = !near_zero(ab[0]) ? (d[0] - a[0]) / ab[0] : (d[1] - a[1]) / ab[1];
                } else {
                    isect.t = 0;
                }
                isect.coincides = true;
                isect.glances = isect.intersects;
            } else {
                isect.intersects = false;
                isect.t = std::numeric_limits<T>::max();
                isect.point = vector_type{};
                isect.coincides = false;
                isect.glances = false;
            }
        } else {
            const T ta = ((c - a) ^ cd) / denom;
            const T tb = ((c - a) ^ ab) / denom;

            isect.intersects = infinite || (ta >= T(0) && ta <= T(1) && tb >= T(0) && tb <= T(1));
            isect.t = ta;
            isect.point = eval(ta);
            isect.coincides = false;
            isect.glances = !infinite && isect.intersects
              && (near_zero(ta) || near_equal(ta, T(1)) || near_zero(tb) || near_equal(tb, T(1)));
        }
        return isect;
    }

    [[nodiscard]] constexpr auto intersects(line const& l2, bool infinite = false) const -> bool
        requires(N == 2)
    {
        return intersection(l2, infinite).intersects;
    }

    // Returns t value for closest point on the line.  t value is *not* clamped
    // from 0.0 to 1.0
    [[nodiscard]] constexpr auto lineProjection(vector_type const& point) const -> T
        requires(N == 2)
    {
        const vector_type d = diff();
        return ((point[0] - m_min[0]) * d[0] + (point[1] - m_min[1]) * d[1]) / d.magnitudeSquared();
    }

    [[nodiscard]] constexpr auto distanceTo(vector_type const& point, bool infinite = false) const
      -> T
        requires(N == 2)
    {
        T t = lineProjection(point);
        if (!infinite) {
            t = std::clamp<T>(t, T(0), T(1));
        }
        return vmag(point - eval(t));
    }

    constexpr void rotate(T angle, vector_type const& rotationCenter = vector_type())
        requires(N == 2)
    {
        auto rotMatrix = matrix3<T>::rotation(angle, rotationCenter);
        m_min = rotMatrix.transformVec2(m_min);
        m_max = rotMatrix.transformVec2(m_max);
    }

    template <typename T2>
    constexpr void transform(matrix3<T2> const& xform)
        requires(N == 2)
    {
        m_min = xform.transformVec2(m_min);
        m_max = xform.transformVec2(m_max);
    }

    template <typename T2>
    [[nodiscard]] constexpr auto transformed(matrix3<T2> const& xform) const -> line
        requires(N == 2)
    {
        return Line(xform.transformVec2(m_min), xform.transformVec2(m_max));
    }

    constexpr void flipHorizontal(T horizontalPos)
        requires(N == 2)
    {
        m_min[0] = T(2) * horizontalPos - m_min[0];
        m_max[0] = T(2) * horizontalPos - m_max[0];
    }

    constexpr void flipVertical(T verticalPos)
        requires(N == 2)
    {
        m_min[1] = T(2) * verticalPos - m_min[1];
        m_max[1] = T(2) * verticalPos - m_max[1];
    }

  private:
    vector_type m_min;
    vector_type m_max;
};

using line_2f = line<std::float_t, 2>;
using line_2d = line<std::double_t, 2>;
using line_2i = line<std::int32_t, 2>;

template <typename T, std::size_t N>
auto operator<<(std::ostream& os, line<T, N> const& l) -> std::ostream& {
    return os << std::format("[{}, {}]", l.min(), l.max());
}

template <typename T, std::size_t N> struct hash<line<T, N>> {
    auto operator()(line<T, N> const& line) const -> std::size_t {
        std::size_t hashval = 0;
        hash_combine(hashval, vector_hasher(line.min()));
        hash_combine(hashval, vector_hasher(line.max()));
        return hashval;
    }
    star::hash<typename line<T, N>::vector_type> vector_hasher;
};

}// namespace star

template <typename T, std::size_t N>
struct std::formatter<star::line<T, N>> : std::formatter<std::string_view> {
    template <typename FormatContext>
    auto format(star::line<T, N> const& line, FormatContext& ctx) const {
        return std::format_to(ctx.out(), "[{}, {}]", line.min(), line.max());
    }
};
