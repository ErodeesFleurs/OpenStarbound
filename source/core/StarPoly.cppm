export module star.poly;

import std;
import star.line;
import star.rect;
import star.vector;
import star.rect;
import star.matrix3;

namespace star {

template <typename DataType>
    requires std::is_arithmetic_v<DataType>
class polygon {
  public:
    using vertex = star::vector<DataType, 2>;
    using line = star::line<DataType, 2>;
    using rect = star::box<DataType, 2>;
    using vertex_list = std::vector<vertex>;

    struct intersect_result {
        bool intersects = false;
        vertex overlap{};
    };

    struct line_intersect_result {
        vertex point;
        DataType along;
        std::optional<std::size_t> intersected_side;
    };

    using iterator = typename vertex_list::iterator;
    using const_iterator = typename vertex_list::const_iterator;

    [[nodiscard]] static auto convex_hull(std::span<const vertex> points) -> polygon {
        if (points.empty()) {
            return {};
        }

        vertex_list sorted_points(points.begin(), points.end());
        std::ranges::sort(sorted_points);

        auto cross = [](vertex const& o, vertex const& a, vertex const& b) -> auto {
            return (a.x() - o.x()) * (b.y() - o.y()) - (a.y() - o.y()) * (b.x() - o.x());
        };

        vertex_list hull;
        hull.reserve(points.size() + 1);

        // Lower hull
        for (auto const& p : sorted_points) {
            while (hull.size() >= 2 && cross(hull[hull.size() - 2], hull.back(), p) <= 0) {
                hull.pop_back();
            }
            hull.push_back(p);
        }

        // Upper hull
        const auto lower_hull_size = hull.size();
        for (auto i = sorted_points.size() - 1; i-- > 0;) {
            auto const& p = sorted_points[i];
            while (hull.size() > lower_hull_size
                   && cross(hull[hull.size() - 2], hull.back(), p) <= 0) {
                hull.pop_back();
            }
            hull.push_back(p);
        }

        hull.pop_back();// Last point is same as first
        return polygon(std::move(hull));
    }
    [[nodiscard]] static auto clip(polygon const& input_poly, polygon const& convex_clip_poly)
      -> polygon {
        if (input_poly.is_null()) {
            return input_poly;
        }

        auto insideEdge = [](line const& edge, vertex const& p) -> auto {
            return ((edge.max() - edge.min()) ^ (p - edge.min())) > 0;
        };

        vertex_list output = input_poly.m_vertexes;
        for (std::size_t i = 0; i < convex_clip_poly.sides(); ++i) {
            if (output.empty()) {
                break;
            }

            line clip_edge = convex_clip_poly.side_at(i);
            vertex_list input = std::move(output);
            output.clear();

            vertex s = input.back();
            for (vertex const& e : input) {
                if (insideEdge(clip_edge, e)) {
                    if (!insideEdge(clip_edge, s)) {
                        output.push_back(clip_edge.intersection(line(s, e)).point);
                    }
                    output.push_back(e);
                } else if (insideEdge(clip_edge, s)) {
                    output.push_back(clip_edge.intersection(line(s, e)).point);
                }
                s = e;
            }
        }
        return polygon(std::move(output));
    }

    constexpr polygon() = default;

    template <typename T> explicit constexpr polygon(const box<T, 2>& rect) {
        m_vertexes = {vertex(rect.min()), vertex(rect.max().x(), rect.min().y()),
                      vertex(rect.max()), vertex(rect.min().x(), rect.max().y())};
    }

    template <typename T> explicit constexpr polygon(const polygon<T>& other) {
        m_vertexes.reserve(other.vertexes().size());
        for (auto const& v : other) {
            m_vertexes.emplace_back(static_cast<vertex>(v));
        }
    }

    explicit constexpr polygon(const vertex& coord) : m_vertexes{coord} {}
    explicit constexpr polygon(const vertex_list& vertexes) : m_vertexes(vertexes) {}
    explicit constexpr polygon(vertex_list&& vertexes) : m_vertexes(std::move(vertexes)) {}
    constexpr polygon(std::initializer_list<vertex> vertexes) : m_vertexes(vertexes) {}

    [[nodiscard]] constexpr auto is_null() const -> bool { return m_vertexes.empty(); }

    [[nodiscard]] constexpr auto is_convex() const -> bool {
        if (m_vertexes.size() < 3) {
            return true;
        }

        bool sign = false;
        for (std::size_t i = 0; i < m_vertexes.size(); ++i) {
            vertex const& v1 = m_vertexes[i];
            vertex const& v2 = m_vertexes[(i + 1) % m_vertexes.size()];
            vertex const& v3 = m_vertexes[(i + 2) % m_vertexes.size()];
            auto cp = (v2 - v1) ^ (v3 - v2);
            if (i == 0) {
                sign = cp > 0;
            } else if ((cp > 0) != sign) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] constexpr auto convex_area() const -> DataType {
        if (m_vertexes.size() < 3) {
            return 0;
        }
        DataType area = 0;
        for (std::size_t i = 0; i < m_vertexes.size(); ++i) {
            auto const& v1 = m_vertexes[i];
            auto const& v2 = m_vertexes[(i + 1) % m_vertexes.size()];
            area += (v1.x() * v2.y() - v2.x() * v1.y());
        }
        return std::abs(area) * 0.5;
    }

    constexpr void deduplicate(DataType max_distance) {
        if (m_vertexes.size() < 2) {
            return;
        }
        const auto distSq = max_distance * max_distance;
        vertex_list unique;
        unique.reserve(m_vertexes.size());
        unique.push_back(m_vertexes[0]);
        for (std::size_t i = 1; i < m_vertexes.size(); ++i) {
            if ((m_vertexes[i] - unique.back()).magnitude_squared() > distSq) {
                unique.push_back(m_vertexes[i]);
            }
        }
        if (unique.size() > 1 && (unique.front() - unique.back()).magnitude_squared() <= distSq) {
            unique.pop_back();
        }
        m_vertexes = std::move(unique);
    }

    constexpr void add(vertex const& v) { m_vertexes.push_back(v); }
    constexpr void remove(std::size_t i) {
        if (i < m_vertexes.size()) {
            m_vertexes.erase(m_vertexes.begin() + i);
        }
    }
    constexpr void clear() { m_vertexes.clear(); }

    [[nodiscard]] constexpr auto vertexes() const -> vertex_list const& { return m_vertexes; }
    [[nodiscard]] constexpr auto vertexes() -> vertex_list& { return m_vertexes; }
    [[nodiscard]] constexpr auto sides() const -> std::size_t { return m_vertexes.size(); }

    [[nodiscard]] constexpr auto side(std::size_t i) const -> line {
        return side_at(i % m_vertexes.size());
    }

    [[nodiscard]] constexpr auto distance(vertex const& p) const -> DataType {
        if (contains(p)) {
            return 0;
        }
        DataType min_dist = std::numeric_limits<DataType>::max();
        for (std::size_t i = 0; i < m_vertexes.size(); ++i) {
            min_dist = std::min(min_dist, side_at(i).distanceTo(p));
        }
        return min_dist;
    }

    constexpr void translate(vertex const& v) {
        for (auto& vert : m_vertexes) {
            vert += v;
        }
    }

    constexpr void set_center(vertex const& c) { translate(c - center()); }

    constexpr void rotate(DataType angle, vertex const& origin = {}) {
        for (auto& v : m_vertexes) {
            v = (v - origin).rotate(angle) + origin;
        }
    }

    constexpr void scale(vertex const& s, vertex const& origin = {}) {
        for (auto& v : m_vertexes) {
            v = (v - origin).piecewise_multiply(s) + origin;
        }
    }

    constexpr void flip_horizontal(DataType x = 0) {
        scale({-1, 1}, {x, 0});
        std::ranges::reverse(m_vertexes);
    }

    constexpr void flip_vertical(DataType y = 0) {
        scale({1, -1}, {0, y});
        std::ranges::reverse(m_vertexes);
    }

    template <typename T> constexpr void transform(matrix3<T> const& mat) {
        for (auto& v : m_vertexes) {
            v = mat.transformVec2(v);
        }
    }

    [[nodiscard]] constexpr auto operator[](std::size_t i) const -> vertex const& {
        return m_vertexes[i];
    }
    [[nodiscard]] constexpr auto operator[](std::size_t i) -> vertex& { return m_vertexes[i]; }

    constexpr auto operator==(polygon const&) const -> bool = default;

    [[nodiscard]] constexpr auto begin() { return m_vertexes.begin(); }
    [[nodiscard]] constexpr auto begin() const { return m_vertexes.begin(); }
    [[nodiscard]] constexpr auto end() { return m_vertexes.end(); }
    [[nodiscard]] constexpr auto end() const { return m_vertexes.end(); }

    [[nodiscard]] constexpr auto get_vertex(std::size_t i) const -> vertex const& {
        return m_vertexes[i % m_vertexes.size()];
    }

    [[nodiscard]] constexpr auto normal(std::size_t i) const -> vertex {
        vertex d = side(i).diff();
        return d == vertex{} ? vertex{} : d.rot90().normalized();
    }

    [[nodiscard]] constexpr auto center() const -> vertex {
        if (m_vertexes.empty()) {
            return {};
        }
        return std::ranges::fold_left(m_vertexes, vertex{}, std::plus<>{})
          / static_cast<DataType>(m_vertexes.size());
    }

    [[nodiscard]] constexpr auto bottom_center() const -> vertex {
        if (m_vertexes.empty()) {
            return {};
        }
        auto bounds = bound_box();
        vertex c = center();
        DataType halfWidth = bounds.width() / 2.0;
        if (bounds.width() > bounds.height()) {
            return c;
        }
        return {c.x(), bounds.min().y() + halfWidth};
    }

    [[nodiscard]] constexpr auto bound_box() const -> rect {
        if (m_vertexes.empty()) {
            return rect::null();
        }
        rect bounds(m_vertexes[0], m_vertexes[0]);
        for (auto const& v : m_vertexes | std::views::drop(1)) {
            bounds.combine(v);
        }
        return bounds;
    }

    [[nodiscard]] constexpr auto winding_number(vertex const& p) const -> int {
        int wn = 0;
        for (std::size_t i = 0; i < m_vertexes.size(); ++i) {
            auto const& v1 = m_vertexes[i];
            auto const& v2 = m_vertexes[(i + 1) % m_vertexes.size()];
            if (v1.y() <= p.y()) {
                if (v2.y() > p.y() && ((v2 - v1) ^ (p - v1)) > 0) {
                    ++wn;
                }
            } else {
                if (v2.y() <= p.y() && ((v2 - v1) ^ (p - v1)) < 0) {
                    --wn;
                }
            }
        }
        return wn;
    }

    [[nodiscard]] constexpr auto contains(vertex const& p) const -> bool {
        return winding_number(p) != 0;
    }

    [[nodiscard]] auto sat_intersection(polygon const& other) const -> intersect_result {
        DataType shortest_overlap = std::numeric_limits<DataType>::max();
        vertex final_axis{};

        auto test_axes = [&](polygon const& a, polygon const& b, bool flip) -> auto {
            for (std::size_t i = 0; i < a.sides(); ++i) {
                vertex axis = a.side(i).diff().rot90().normalized();
                if (flip) {
                    axis = -axis;
                }

                auto [minA, maxA] = a.project(axis);
                auto [minB, maxB] = b.project(axis);

                DataType overlap = std::min(maxA, maxB) - std::max(minA, minB);
                if (overlap <= 0) {
                    return false;
                }

                if (overlap < shortest_overlap) {
                    shortest_overlap = overlap;
                    final_axis = axis;
                }
            }
            return true;
        };

        if (!test_axes(*this, other, true) || !test_axes(other, *this, false)) {
            return {false, {}};
        }

        return {true, final_axis * shortest_overlap};
    }

    [[nodiscard]] auto directional_sat_intersection(polygon const& other, vertex const& dir,
                                                    bool choose_sign) const -> intersect_result {
        DataType shortest_overlap = std::numeric_limits<DataType>::max();
        vertex final_sep_dir{};

        auto test_directional = [&](polygon const& a, polygon const& b) -> auto {
            for (std::size_t i = 0; i < a.sides() + b.sides(); ++i) {
                vertex axis = (i < a.sides()) ? a.side(i).diff().rot90().normalized()
                                              : b.side(i - a.sides()).diff().rot90().normalized();
                auto [minA, maxA] = a.project(axis);
                auto [minB, maxB] = b.project(axis);
                DataType overlap = std::min(maxA, maxB) - std::max(minA, minB);
                if (overlap <= 0) {
                    return false;
                }

                DataType axisDot = dir * axis;
                if (std::abs(axisDot) < 1e-6) {
                    continue;
                }

                DataType proj_overlap = overlap / axisDot;
                if (choose_sign) {
                    if (std::abs(proj_overlap) < shortest_overlap) {
                        shortest_overlap = std::abs(proj_overlap);
                        final_sep_dir = dir * (proj_overlap > 0 ? 1 : -1);
                    }
                } else if (proj_overlap >= 0 && proj_overlap < shortest_overlap) {
                    shortest_overlap = proj_overlap;
                    final_sep_dir = dir;
                }
            }
            return true;
        };

        if (!test_directional(*this, other)) {
            return {false, {}};
        }
        return {true, final_sep_dir * shortest_overlap};
    }

    [[nodiscard]] auto line_intersection(line const& l) const
      -> std::optional<line_intersect_result> {
        if (contains(l.min())) {
            return line_intersect_result{l.min(), 0, {}};
        }
        std::optional<line_intersect_result> nearest;
        for (std::size_t i = 0; i < m_vertexes.size(); ++i) {
            auto isect = l.intersection(side_at(i));
            if (isect.intersects && (!nearest || isect.t < nearest->along)) {
                nearest = line_intersect_result{isect.point, isect.t, i};
            }
        }
        return nearest;
    }

    [[nodiscard]] auto intersects(polygon const& other) const -> bool {
        return satIntersection(other).intersects;
    }
    [[nodiscard]] auto intersects(line const& l) const -> bool {
        if (contains(l.min()) || contains(l.max())) {
            return true;
        }
        return std::ranges::any_of(std::views::iota(0UZ, m_vertexes.size()),
                                   [&](std::size_t i) -> auto { return l.intersects(side_at(i)); });
    }

  private:
    [[nodiscard]] constexpr auto side_at(std::size_t i) const -> line {
        return line(m_vertexes[i], m_vertexes[(i + 1) % m_vertexes.size()]);
    }

    [[nodiscard]] constexpr auto project(vertex const& axis) const
      -> std::pair<DataType, DataType> {
        DataType min = std::numeric_limits<DataType>::max();
        DataType max = std::numeric_limits<DataType>::lowest();
        for (auto const& v : m_vertexes) {
            DataType p = v * axis;
            min = std::min(min, p);
            max = std::max(max, p);
        }
        return {min, max};
    }

    vertex_list m_vertexes;
};

using poly_i = polygon<std::int32_t>;
using poly_f = polygon<std::float_t>;
using poly_d = polygon<std::double_t>;

}// namespace star
