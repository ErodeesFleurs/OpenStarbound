export module star.matrix3;

import std;
import star.vector;

export namespace star {

template <typename T>
    requires std::is_arithmetic_v<T>
class matrix3 {
  public:
    using Vec3 = star::vector<T, 3>;
    using Vec2 = star::vector<T, 2>;
    using Rows = std::array<Vec3, 3>;

    static constexpr bool ContiguousStorage =
      sizeof(Vec3) == 3 * sizeof(T) && sizeof(Rows) == 3 * sizeof(Vec3);

    [[nodiscard]] static constexpr auto identity() -> matrix3 {
        return matrix3(1, 0, 0, 0, 1, 0, 0, 0, 1);
    }

    // Construct an affine 2d transform
    [[nodiscard]] static constexpr auto rotation(T angle, Vec2 const& point = Vec2()) -> matrix3 {
        T s = std::sin(angle);
        T c = std::cos(angle);
        return matrix3(c, -s, point[0] - c * point[0] + s * point[1], s, c,
                       point[1] - s * point[0] - c * point[1], 0, 0, 1);
    }
    [[nodiscard]] static constexpr auto translation(Vec2 const& point) -> matrix3 {
        return matrix3(1, 0, point[0], 0, 1, point[1], 0, 0, 1);
    }
    [[nodiscard]] static constexpr auto scaling(T scale, Vec2 const& point = Vec2()) -> matrix3 {
        return scaling(Vec2::filled(scale), point);
    }
    [[nodiscard]] static constexpr auto scaling(Vec2 const& scale, Vec2 const& point = Vec2())
      -> matrix3 {
        return matrix3(scale[0], 0, point[0] - point[0] * scale[0], 0, scale[1],
                       point[1] - point[1] * scale[1], 0, 0, 1);
    }

    constexpr matrix3() = default;

    constexpr matrix3(T r1c1, T r1c2, T r1c3, T r2c1, T r2c2, T r2c3, T r3c1, T r3c2, T r3c3)
        : m_rows(Vec3(r1c1, r1c2, r1c3), Vec3(r2c1, r2c2, r2c3), Vec3(r3c1, r3c2, r3c3)) {}

    constexpr matrix3(Vec3 const& r1, Vec3 const& r2, Vec3 const& r3) : m_rows{r1, r2, r3} {}

    explicit constexpr matrix3(T const* ptr) : m_rows{Vec3(ptr), Vec3(ptr + 3), Vec3(ptr + 6)} {}

    explicit constexpr matrix3(std::span<T const, 9> data)
        : m_rows{Vec3(data.data()), Vec3(data.data() + 3), Vec3(data.data() + 6)} {}

    template <typename T2> explicit constexpr matrix3(matrix3<T2> const& m) { *this = m; }

    template <typename T2> constexpr auto operator=(matrix3<T2> const& m) -> matrix3& {
        m_rows = m.rows();
        return *this;
    }

    template <typename Self>
    [[nodiscard]] constexpr auto operator[](this Self&& self, std::size_t i) -> auto&& {
        return std::forward<Self>(self).m_rows[i];
    }

    template <typename Self>
    [[nodiscard]] constexpr auto ptr(this Self&& self) -> auto*
        requires ContiguousStorage
    {
        return self.m_rows[0].ptr();
    }

    constexpr void copy(T* loc) const {
        std::ranges::copy(m_rows[0], loc);
        std::ranges::copy(m_rows[1], loc + 3);
        std::ranges::copy(m_rows[2], loc + 6);
    }

    [[nodiscard]] constexpr auto row(std::size_t i) const -> Vec3 { return m_rows[i]; }

    template <typename T2> constexpr void set_row(std::size_t i, std::array<T2, 3> const& v) {
        m_rows[i] = Vec3(v);
    }

    [[nodiscard]] constexpr auto col(std::size_t i) const -> Vec3 {
        return Vec3(m_rows[0][i], m_rows[1][i], m_rows[2][i]);
    }

    template <typename T2> constexpr void set_col(std::size_t i, std::array<T2, 3> const& v) {
        m_rows[0][i] = static_cast<T>(v[0]);
        m_rows[1][i] = static_cast<T>(v[1]);
        m_rows[2][i] = static_cast<T>(v[2]);
    }

    [[nodiscard]] constexpr auto determinant() const -> T {
        return m_rows[0][0] * (m_rows[1][1] * m_rows[2][2] - m_rows[1][2] * m_rows[2][1])
          - m_rows[0][1] * (m_rows[1][0] * m_rows[2][2] - m_rows[1][2] * m_rows[2][0])
          + m_rows[0][2] * (m_rows[1][0] * m_rows[2][1] - m_rows[1][1] * m_rows[2][0]);
    }

    [[nodiscard]] constexpr auto trace() const -> Vec3 {
        return Vec3(m_rows[0][0], m_rows[1][1], m_rows[2][2]);
    }

    constexpr void transpose() {
        std::swap(m_rows[1][0], m_rows[0][1]);
        std::swap(m_rows[2][0], m_rows[0][2]);
        std::swap(m_rows[2][1], m_rows[1][2]);
    }

    constexpr void invert() {
        T d = determinant();
        matrix3 original = *this;

        m_rows[0][0] = (original[1][1] * original[2][2] - original[1][2] * original[2][1]) / d;
        m_rows[0][1] = -(original[0][1] * original[2][2] - original[0][2] * original[2][1]) / d;
        m_rows[0][2] = (original[0][1] * original[1][2] - original[0][2] * original[1][1]) / d;
        m_rows[1][0] = -(original[1][0] * original[2][2] - original[1][2] * original[2][0]) / d;
        m_rows[1][1] = (original[0][0] * original[2][2] - original[0][2] * original[2][0]) / d;
        m_rows[1][2] = -(original[0][0] * original[1][2] - original[0][2] * original[1][0]) / d;
        m_rows[2][0] = (original[1][0] * original[2][1] - original[1][1] * original[2][0]) / d;
        m_rows[2][1] = -(original[0][0] * original[2][1] - original[0][1] * original[2][0]) / d;
        m_rows[2][2] = (original[0][0] * original[1][1] - original[0][1] * original[1][0]) / d;
    }

    [[nodiscard]] constexpr auto inverse() const -> matrix3 {
        auto m = *this;
        m.invert();
        return m;
    }

    constexpr void orthogonalize() {
        m_rows[0].normalize();
        T dot01 = m_rows[0] * m_rows[1];
        m_rows[1] -= m_rows[0] * dot01;
        m_rows[1].normalize();

        T dot12 = m_rows[1] * m_rows[2];
        T dot02 = m_rows[0] * m_rows[2];
        m_rows[2] -= (m_rows[1] * dot12 + m_rows[0] * dot02);
        m_rows[2].normalize();
    }

    [[nodiscard]] constexpr auto is_orthogonal(T tolerance) const -> bool {
        T det = determinant();
        return std::abs(det - 1) < tolerance || std::abs(det + 1) < tolerance;
    }

    constexpr void rotate(T angle, Vec2 const& point = Vec2()) {
        *this = rotation(angle, point) * *this;
    }

    constexpr void translate(Vec2 const& point) { *this = translation(point) * *this; }

    constexpr void scale(Vec2 const& scale, Vec2 const& point = Vec2()) {
        *this = scaling(scale, point) * *this;
    }

    constexpr void scale(T scale, Vec2 const& point = Vec2()) {
        *this = scaling(scale, point) * *this;
    }

    template <typename T2>
    [[nodiscard]] constexpr auto transform_vec2(std::array<T2, 2> const& point) const
      -> std::array<T2, 2> {
        return (*this) * point;
    }

    [[nodiscard]] constexpr auto transform_angle(float angle) const -> float {
        auto a = Vec2::withAngle(angle, 1.0F);
        matrix3 m = *this;
        m[0][2] = 0;
        m[1][2] = 0;
        return m.transform_vec2(a).angle();
    }

    auto operator==(matrix3 const&) const -> bool = default;
    auto operator<=>(matrix3 const&) const = default;

    constexpr auto operator*=(T const& s) -> matrix3& {
        for (auto& row : m_rows) {
            row *= s;
        }
        return *this;
    }

    constexpr auto operator/=(T const& s) -> matrix3& {
        for (auto& row : m_rows) {
            row /= s;
        }
        return *this;
    }

    [[nodiscard]] constexpr auto operator*(T const& s) const -> matrix3 {
        return matrix3(*this) *= s;
    }
    [[nodiscard]] constexpr auto operator/(T const& s) const -> matrix3 {
        return matrix3(*this) /= s;
    }
    [[nodiscard]] constexpr auto operator-() const -> matrix3 {
        return matrix3(-m_rows[0], -m_rows[1], -m_rows[2]);
    }

    template <typename T2> constexpr auto operator+=(matrix3<T2> const& m2) -> matrix3& {
        for (std::size_t i = 0; i < 3; ++i) {
            m_rows[i] += m2[i];
        }
        return *this;
    }

    template <typename T2> constexpr auto operator-=(matrix3<T2> const& m2) -> matrix3& {
        for (std::size_t i = 0; i < 3; ++i) {
            m_rows[i] -= m2[i];
        }
        return *this;
    }

    template <typename T2> constexpr auto operator*=(matrix3<T2> const& m2) -> matrix3& {
        return *this = *this * m2;
    }

    template <typename T2>
    [[nodiscard]] constexpr auto operator+(matrix3<T2> const& m2) const -> matrix3 {
        return matrix3(*this) += m2;
    }

    template <typename T2>
    [[nodiscard]] constexpr auto operator-(matrix3<T2> const& m2) const -> matrix3 {
        return matrix3(*this) -= m2;
    }

    template <typename T2>
    [[nodiscard]] constexpr auto operator*(matrix3<T2> const& m2) const -> matrix3 {
        return matrix3(m_rows[0][0] * m2[0][0] + m_rows[0][1] * m2[1][0] + m_rows[0][2] * m2[2][0],
                       m_rows[0][0] * m2[0][1] + m_rows[0][1] * m2[1][1] + m_rows[0][2] * m2[2][1],
                       m_rows[0][0] * m2[0][2] + m_rows[0][1] * m2[1][2] + m_rows[0][2] * m2[2][2],
                       m_rows[1][0] * m2[0][0] + m_rows[1][1] * m2[1][0] + m_rows[1][2] * m2[2][0],
                       m_rows[1][0] * m2[0][1] + m_rows[1][1] * m2[1][1] + m_rows[1][2] * m2[2][1],
                       m_rows[1][0] * m2[0][2] + m_rows[1][1] * m2[1][2] + m_rows[1][2] * m2[2][2],
                       m_rows[2][0] * m2[0][0] + m_rows[2][1] * m2[1][0] + m_rows[2][2] * m2[2][0],
                       m_rows[2][0] * m2[0][1] + m_rows[2][1] * m2[1][1] + m_rows[2][2] * m2[2][1],
                       m_rows[2][0] * m2[0][2] + m_rows[2][1] * m2[1][2] + m_rows[2][2] * m2[2][2]);
    }

    template <typename T2>
    [[nodiscard]] constexpr auto operator*(std::array<T2, 3> const& u) const -> Vec3 {
        return Vec3(m_rows[0][0] * u[0] + m_rows[0][1] * u[1] + m_rows[0][2] * u[2],
                    m_rows[1][0] * u[0] + m_rows[1][1] * u[1] + m_rows[1][2] * u[2],
                    m_rows[2][0] * u[0] + m_rows[2][1] * u[1] + m_rows[2][2] * u[2]);
    }

    template <typename T2>
    [[nodiscard]] constexpr auto operator*(std::array<T2, 2> const& u) const -> Vec2 {
        return Vec2(m_rows[0][0] * u[0] + m_rows[0][1] * u[1] + m_rows[0][2],
                    m_rows[1][0] * u[0] + m_rows[1][1] * u[1] + m_rows[1][2]);
    }

    constexpr auto rows() const -> Rows const& { return m_rows; }

  private:
    Rows m_rows;
};

using Mat3F = matrix3<float>;
using Mat3D = matrix3<double>;

}// namespace star

template <typename T> struct std::formatter<star::matrix3<T>> {
    auto format(star::matrix3<T> const& m, std::format_context& ctx) const -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "[[{}, {}, {}], [{}, {}, {}], [{}, {}, {}]]", m[0][0],
                              m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[2][0], m[2][1],
                              m[2][2]);
    }
};
