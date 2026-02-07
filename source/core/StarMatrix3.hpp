#pragma once

#include "StarVector.hpp"

import std;

namespace Star {

template <typename T>
class Matrix3 {
public:
  using Vec3 = Vector<T, 3>;
  using Vec2 = Vector<T, 2>;
  using Rows = Array<Vec3, 3>;

  // Only enable pointer access if we know that our internal rows are not
  // padded
  template <typename RT = void>
  using EnableIfContiguousStorage =
       std::enable_if_t<sizeof(Vec3) == 3 * sizeof(T) && sizeof(Rows) == 3 * sizeof(Vec3), RT>;

  static auto identity() -> Matrix3;

  // Construct an affine 2d transform
  static auto rotation(T angle, Vec2 const& point = Vec2()) -> Matrix3;
  static auto translation(Vec2 const& point) -> Matrix3;
  static auto scaling(T scale, Vec2 const& point = Vec2()) -> Matrix3;
  static auto scaling(Vec2 const& scale, Vec2 const& point = Vec2()) -> Matrix3;

  Matrix3();

  Matrix3(T r1c1, T r1c2, T r1c3, T r2c1, T r2c2, T r2c3, T r3c1, T r3c2, T r3c3);

  Matrix3(Vec3 const& r1, Vec3 const& r2, Vec3 const& r3);

  Matrix3(T const* ptr);
  template <typename T2>
  Matrix3(Matrix3<T2> const& m);

  template <typename T2>
  auto operator=(Matrix3<T2> const& m) -> Matrix3&;

  // Row-major indexing
  auto operator[](std::size_t const i) -> Vec3&;
  auto operator[](std::size_t const i) const -> Vec3 const&;

  // Gives pointer to row major storage
  auto ptr() -> EnableIfContiguousStorage<T*>;
  auto ptr() const -> EnableIfContiguousStorage<T const*>;

  // Copy to an existing array
  void copy(T* loc) const;

  auto row(std::size_t i) const -> Vec3;
  template <typename T2>
  void setRow(std::size_t i, Vector<T2, 3> const& v);

  auto col(std::size_t i) -> Vec3;
  template <typename T2>
  void setCol(std::size_t i, Vector<T2, 3> const& v);

  auto determinant() const -> T;
  auto trace() const -> Vec3;
  auto inverse() const -> Matrix3;
  auto isOrthogonal(T tolerance) const -> bool;

  void transpose();
  void orthogonalize();
  void invert();

  // Apply the given 2d affine transformation to this matrix in global
  // coordinates
  void rotate(T angle, Vec2 const& point = Vec2());
  void translate(Vec2 const& point);
  void scale(Vec2 const& scale, Vec2 const& point = Vec2());
  void scale(T scale, Vec2 const& point = Vec2());

  // Do an affine transformation of the given 2d vector.
  template <typename T2>
  auto transformVec2(Vector<T2, 2> const& v2) const -> Vector<T2, 2>;

  // The resulting angle of a transformation on any ray with this angle.
  [[nodiscard]] auto transformAngle(float angle) const -> float;

  auto operator==(Matrix3 const& m2) const -> bool;
  auto operator!=(Matrix3 const& m2) const -> bool;

  auto operator*=(T const& s) -> Matrix3&;
  auto operator/=(T const& s) -> Matrix3&;
  auto operator*(T const& s) const -> Matrix3;
  auto operator/(T const& s) const -> Matrix3;
  auto operator-() const -> Matrix3;

  template <typename T2>
  auto operator+=(Matrix3<T2> const& m2) -> Matrix3&;

  template <typename T2>
  auto operator-=(Matrix3<T2> const& m2) -> Matrix3&;

  template <typename T2>
  auto operator*=(Matrix3<T2> const& m2) -> Matrix3&;

  template <typename T2>
  auto operator+(Matrix3<T2> const& m2) const -> Matrix3;

  template <typename T2>
  auto operator-(Matrix3<T2> const& m2) const -> Matrix3;

  template <typename T2>
  auto operator*(Matrix3<T2> const& m2) const -> Matrix3;

  template <typename T2>
  auto operator*(Vector<T2, 3> const& v) const -> Vec3;

  template <typename T2>
  auto operator*(Vector<T2, 2> const& v) const -> Vec2;
private:
  Rows m_rows;
};

using Mat3F = Matrix3<float>;
using Mat3D = Matrix3<double>;

template <typename T>
auto Matrix3<T>::identity() -> Matrix3<T> {
  return Matrix3(1, 0, 0, 0, 1, 0, 0, 0, 1);
}

template <typename T>
auto Matrix3<T>::rotation(T angle, Vec2 const& point) -> Matrix3<T> {
  T s = sin(angle);
  T c = cos(angle);
  return Matrix3(c, -s, point[0] - c * point[0] + s * point[1], s, c, point[1] - s * point[0] - c * point[1], 0, 0, 1);
}

template <typename T>
auto Matrix3<T>::translation(Vec2 const& point) -> Matrix3<T> {
  return Matrix3(1, 0, point[0], 0, 1, point[1], 0, 0, 1);
}

template <typename T>
auto Matrix3<T>::scaling(T scale, Vec2 const& point) -> Matrix3<T> {
  return scaling(Vec2::filled(scale), point);
}

template <typename T>
auto Matrix3<T>::scaling(Vec2 const& scale, Vec2 const& point) -> Matrix3<T> {
  return Matrix3(scale[0], 0, point[0] - point[0] * scale[0], 0, scale[1], point[1] - point[1] * scale[1], 0, 0, 1);
}

template <typename T>
Matrix3<T>::Matrix3() = default;

template <typename T>
Matrix3<T>::Matrix3(T r1c1, T r1c2, T r1c3, T r2c1, T r2c2, T r2c3, T r3c1, T r3c2, T r3c3)
  : m_rows(Vec3(r1c1, r1c2, r1c3), Vec3(r2c1, r2c2, r2c3), Vec3(r3c1, r3c2, r3c3)) {}

template <typename T>
Matrix3<T>::Matrix3(const Vec3& r1, const Vec3& r2, const Vec3& r3)
  : m_rows{r1, r2, r3} {}

template <typename T>
Matrix3<T>::Matrix3(T const* ptr)
  : m_rows{Vec3(ptr), Vec3(ptr + 3), Vec3(ptr + 6)} {}

template <typename T>
template <typename T2>
Matrix3<T>::Matrix3(const Matrix3<T2>& m) {
  *this = m;
}

template <typename T>
template <typename T2>
auto Matrix3<T>::operator=(const Matrix3<T2>& m) -> Matrix3<T>& {
  m_rows = m.m_rows;
  return *this;
}

template <typename T>
auto Matrix3<T>::operator[](const std::size_t i) -> Vec3 & {
  return m_rows[i];
}

template <typename T>
auto Matrix3<T>::operator[](const std::size_t i) const -> Vec3 const & {
  return m_rows[i];
}

template <typename T>
auto Matrix3<T>::ptr() -> EnableIfContiguousStorage<T*> {
  return m_rows[0].ptr();
}

template <typename T>
auto Matrix3<T>::ptr() const -> EnableIfContiguousStorage<T const*> {
  return m_rows[0].ptr();
}

template <typename T>
void Matrix3<T>::copy(T* loc) const {
  m_rows[0].copyFrom(loc);
  m_rows[1].copyFrom(loc + 3);
  m_rows[2].copyFrom(loc + 6);
}

template <typename T>
auto Matrix3<T>::row(std::size_t i) const -> Vec3 {
  return operator[](i);
}

template <typename T>
template <typename T2>
void Matrix3<T>::setRow(std::size_t i, const Vector<T2, 3>& v) {
  operator[](i) = Vec3(v);
}

template <typename T>
auto Matrix3<T>::col(std::size_t i) -> Vec3 {
  return Vec3(m_rows[0][i], m_rows[1][i], m_rows[2][i]);
}

template <typename T>
template <typename T2>
void Matrix3<T>::setCol(std::size_t i, const Vector<T2, 3>& v) {
  m_rows[0][i] = T(v[0]);
  m_rows[1][i] = T(v[1]);
  m_rows[2][i] = T(v[2]);
}

template <typename T>
auto Matrix3<T>::determinant() const -> T {
  return m_rows[0][0] * m_rows[1][1] * m_rows[2][2] - m_rows[0][0] * m_rows[2][1] * m_rows[1][2]
      + m_rows[1][0] * m_rows[2][1] * m_rows[0][2] - m_rows[1][0] * m_rows[0][1] * m_rows[2][2]
      + m_rows[2][0] * m_rows[0][1] * m_rows[1][2] - m_rows[2][0] * m_rows[1][1] * m_rows[0][2];
}

template <typename T>
void Matrix3<T>::transpose() {
  std::swap(m_rows[1][0], m_rows[0][1]);
  std::swap(m_rows[2][0], m_rows[0][2]);
  std::swap(m_rows[2][1], m_rows[1][2]);
}

template <typename T>
void Matrix3<T>::invert() {
  T d = determinant();

  m_rows[0][0] = (m_rows[1][1] * m_rows[2][2] - m_rows[1][2] * m_rows[2][1]) / d;
  m_rows[0][1] = -(m_rows[0][1] * m_rows[2][2] - m_rows[0][2] * m_rows[2][1]) / d;
  m_rows[0][2] = (m_rows[0][1] * m_rows[1][2] - m_rows[0][2] * m_rows[1][1]) / d;
  m_rows[1][0] = -(m_rows[1][0] * m_rows[2][2] - m_rows[1][2] * m_rows[2][0]) / d;
  m_rows[1][1] = (m_rows[0][0] * m_rows[2][2] - m_rows[0][2] * m_rows[2][0]) / d;
  m_rows[1][2] = -(m_rows[0][0] * m_rows[1][2] - m_rows[0][2] * m_rows[1][0]) / d;
  m_rows[2][0] = (m_rows[1][0] * m_rows[2][1] - m_rows[1][1] * m_rows[2][0]) / d;
  m_rows[2][1] = -(m_rows[0][0] * m_rows[2][1] - m_rows[0][1] * m_rows[2][0]) / d;
  m_rows[2][2] = (m_rows[0][0] * m_rows[1][1] - m_rows[0][1] * m_rows[1][0]) / d;
}

template <typename T>
auto Matrix3<T>::inverse() const -> Matrix3<T> {
  auto m = *this;
  m.invert();
  return m;
}

template <typename T>
void Matrix3<T>::orthogonalize() {
  m_rows[0].normalize();
  T dot = m_rows[0] * m_rows[1];
  m_rows[1][0] -= m_rows[0][0] * dot;
  m_rows[1][1] -= m_rows[0][1] * dot;
  m_rows[1][2] -= m_rows[0][2] * dot;
  m_rows[1].normalize();

  dot = m_rows[1] * m_rows[2];
  m_rows[2][0] -= m_rows[1][0] * dot;
  m_rows[2][1] -= m_rows[1][1] * dot;
  m_rows[2][2] -= m_rows[1][2] * dot;
  m_rows[2].normalize();
}

template <typename T>
auto Matrix3<T>::isOrthogonal(T tolerance) const -> bool {
  T det = determinant();
  return std::fabs(det - 1) < tolerance || std::fabs(det + 1) < tolerance;
}

template <typename T>
void Matrix3<T>::rotate(T angle, Vec2 const& point) {
  *this = rotation(angle, point) * *this;
}

template <typename T>
void Matrix3<T>::translate(Vec2 const& point) {
  *this = translation(point) * *this;
}

template <typename T>
void Matrix3<T>::scale(Vec2 const& scale, Vec2 const& point) {
  *this = scaling(scale, point) * *this;
}

template <typename T>
void Matrix3<T>::scale(T scale, Vec2 const& point) {
  *this = scaling(scale, point) * *this;
}

template <typename T>
template <typename T2>
auto Matrix3<T>::transformVec2(Vector<T2, 2> const& point) const -> Vector<T2, 2> {
  return (*this) * point;
}

template <typename T>
auto Matrix3<T>::transformAngle(float angle) const -> float {
  Vec2 a = Vec2::withAngle(angle, 1.0f);
  Matrix3 m = *this;
  m[0][2] = 0;
  m[1][2] = 0;
  return m.transformVec2(a).angle();
}

template <typename T>
auto Matrix3<T>::operator==(Matrix3 const& m2) const -> bool {
  return tie(m_rows[0], m_rows[1], m_rows[2]) == tie(m2.m_rows[0], m2.m_rows[1], m2.m_rows[2]);
}

template <typename T>
auto Matrix3<T>::operator!=(Matrix3 const& m2) const -> bool {
  return tie(m_rows[0], m_rows[1], m_rows[2]) != tie(m2.m_rows[0], m2.m_rows[1], m2.m_rows[2]);
}

template <typename T>
auto Matrix3<T>::operator*=(const T& s) -> Matrix3<T>& {
  m_rows[0] *= s;
  m_rows[1] *= s;
  m_rows[2] *= s;
  return *this;
}

template <typename T>
auto Matrix3<T>::operator/=(const T& s) -> Matrix3<T>& {
  m_rows[0] /= s;
  m_rows[1] /= s;
  m_rows[2] /= s;
  return *this;
}

template <typename T>
auto Matrix3<T>::trace() const -> Vec3 {
  return Vec3(m_rows[0][0], m_rows[1][1], m_rows[2][2]);
}

template <typename T>
auto Matrix3<T>::operator-() const -> Matrix3<T> {
  return Matrix3(-m_rows[0], -m_rows[1], -m_rows[2]);
}

template <typename T>
template <typename T2>
auto Matrix3<T>::operator+=(const Matrix3<T2>& m) -> Matrix3<T>& {
  m_rows[0] += m[0];
  m_rows[1] += m[1];
  m_rows[2] += m[2];
  return *this;
}

template <typename T>
template <typename T2>
auto Matrix3<T>::operator-=(const Matrix3<T2>& m) -> Matrix3<T>& {
  m_rows[0] -= m[0];
  m_rows[1] -= m[1];
  m_rows[2] -= m[2];
  return *this;
}

template <typename T>
template <typename T2>
auto Matrix3<T>::operator*=(Matrix3<T2> const& m2) -> Matrix3<T>& {
  *this = *this * m2;
  return *this;
}

template <typename T>
template <typename T2>
auto Matrix3<T>::operator+(const Matrix3<T2>& m2) const -> Matrix3<T> {
  return Matrix3<T>(m_rows[0] + m2[0], m_rows[1] + m2[1], m_rows[2] + m2[2]);
}

template <typename T>
template <typename T2>
auto Matrix3<T>::operator-(const Matrix3<T2>& m2) const -> Matrix3<T> {
  return Matrix3<T>(m_rows[0] - m2[0], m_rows[1] - m2[1], m_rows[2] - m2[2]);
}

template <typename T>
template <typename T2>
auto Matrix3<T>::operator*(const Matrix3<T2>& m2) const -> Matrix3<T> {
  return Matrix3<T>(m_rows[0][0] * m2[0][0] + m_rows[0][1] * m2[1][0] + m_rows[0][2] * m2[2][0],
      m_rows[0][0] * m2[0][1] + m_rows[0][1] * m2[1][1] + m_rows[0][2] * m2[2][1],
      m_rows[0][0] * m2[0][2] + m_rows[0][1] * m2[1][2] + m_rows[0][2] * m2[2][2],
      m_rows[1][0] * m2[0][0] + m_rows[1][1] * m2[1][0] + m_rows[1][2] * m2[2][0],
      m_rows[1][0] * m2[0][1] + m_rows[1][1] * m2[1][1] + m_rows[1][2] * m2[2][1],
      m_rows[1][0] * m2[0][2] + m_rows[1][1] * m2[1][2] + m_rows[1][2] * m2[2][2],
      m_rows[2][0] * m2[0][0] + m_rows[2][1] * m2[1][0] + m_rows[2][2] * m2[2][0],
      m_rows[2][0] * m2[0][1] + m_rows[2][1] * m2[1][1] + m_rows[2][2] * m2[2][1],
      m_rows[2][0] * m2[0][2] + m_rows[2][1] * m2[1][2] + m_rows[2][2] * m2[2][2]);
}

template <typename T>
template <typename T2>
auto Matrix3<T>::operator*(const Vector<T2, 3>& u) const -> Vec3 {
  return Vec3(m_rows[0][0] * u[0] + m_rows[0][1] * u[1] + m_rows[0][2] * u[2],
      m_rows[1][0] * u[0] + m_rows[1][1] * u[1] + m_rows[1][2] * u[2],
      m_rows[2][0] * u[0] + m_rows[2][1] * u[1] + m_rows[2][2] * u[2]);
}

template <typename T>
template <typename T2>
auto Matrix3<T>::operator*(const Vector<T2, 2>& u) const -> Vec2 {
  return Vec2(m_rows[0][0] * u[0] + m_rows[0][1] * u[1] + m_rows[0][2],
    m_rows[1][0] * u[0] + m_rows[1][1] * u[1] + m_rows[1][2]);
}

template <typename T>
auto Matrix3<T>::operator/(const T& s) const -> Matrix3<T> {
  return Matrix3<T>(m_rows[0] / s, m_rows[1] / s, m_rows[2] / s);
}

template <typename T>
auto Matrix3<T>::operator*(const T& s) const -> Matrix3<T> {
  return Matrix3<T>(m_rows[0] * s, m_rows[1] * s, m_rows[2] * s);
}

template <typename T>
auto determinant(const Matrix3<T>& m) -> T {
  return m.determinant();
}

template <typename T>
auto transpose(Matrix3<T> m) -> Matrix3<T> {
  return m.transpose();
}

template <typename T>
auto ortho(Matrix3<T> mat) -> Matrix3<T> {
  return mat.orthogonalize();
}

template <typename T>
auto operator*(T s, const Matrix3<T>& m) -> Matrix3<T> {
  return m * s;
}

template <typename T>
auto operator<<(std::ostream& os, Matrix3<T> m) -> std::ostream& {
  os << m[0][0] << ' ' << m[0][1] << ' ' << m[0][2] << std::endl;
  os << m[1][0] << ' ' << m[1][1] << ' ' << m[1][2] << std::endl;
  os << m[2][0] << ' ' << m[2][1] << ' ' << m[2][2];
  return os;
}

}

template <typename T>
struct std::formatter<Star::Matrix3<T>> : Star::ostream_formatter {};
