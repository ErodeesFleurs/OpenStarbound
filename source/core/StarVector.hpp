#pragma once

#include "StarOstreamFormatter.hpp"
#include "StarArray.hpp"
#include "StarMathCommon.hpp"
#include "StarAlgorithm.hpp"
#include "StarHash.hpp"

import std;

namespace Star {

template <typename T, std::size_t N>
class Vector : public Array<T, N> {
public:
  using Base = Array<T, N>;

  template <std::size_t P, typename T2 = void>
  using Enable2D =  std::enable_if_t<P == 2 && N == P, T2>;

  template <std::size_t P, typename T2 = void>
  using Enable3D =  std::enable_if_t<P == 3 && N == P, T2>;

  template <std::size_t P, typename T2 = void>
  using Enable4D =  std::enable_if_t<P == 4 && N == P, T2>;

  template <std::size_t P, typename T2 = void>
  using Enable2DOrHigher =  std::enable_if_t<P >= 2 && N == P, T2>;

  template <std::size_t P, typename T2 = void>
  using Enable3DOrHigher =  std::enable_if_t<P >= 3 && N == P, T2>;

  template <std::size_t P, typename T2 = void>
  using Enable4DOrHigher =  std::enable_if_t<P >= 4 && N == P, T2>;

  static auto filled(T const& t) -> Vector;

  template <typename T2>
  static auto floor(Vector<T2, N> const& v) -> Vector;

  template <typename T2>
  static auto ceil(Vector<T2, N> const& v) -> Vector;

  template <typename T2>
  static auto round(Vector<T2, N> const& v) -> Vector;

  template <typename Iterator>
  static auto copyFrom(Iterator p) -> Vector;

  // Is zero-initialized (from Array)
  Vector();

  explicit Vector(T const& e1);

  template <typename... TN>
  Vector(T const& e1, TN const&... rest);

  template <typename T2>
  explicit Vector(Array<T2, N> const& v);

  template <typename T2, typename T3>
  Vector(Array<T2, N - 1> const& u, T3 const& v);

  template <std::size_t N2>
  auto toSize() const -> Vector<T, N2>;
  [[nodiscard]] auto vec2() const -> Vector<T, 2>;
  [[nodiscard]] auto vec3() const -> Vector<T, 3>;
  [[nodiscard]] auto vec4() const -> Vector<T, 4>;

  [[nodiscard]] auto piecewiseMultiply(Vector const& v2) const -> Vector;
  [[nodiscard]] auto piecewiseDivide(Vector const& v2) const -> Vector;

  [[nodiscard]] auto piecewiseMin(Vector const& v2) const -> Vector;
  [[nodiscard]] auto piecewiseMax(Vector const& v2) const -> Vector;
  [[nodiscard]] auto piecewiseClamp(Vector const& min, Vector const& max) const -> Vector;

  [[nodiscard]] auto min() const -> T;
  [[nodiscard]] auto max() const -> T;

  [[nodiscard]] auto sum() const -> T;
  [[nodiscard]] auto product() const -> T;

  template <typename Function>
  auto combine(Vector const& v, Function f) const -> Vector;

  // Outputs angles in the range [0, pi]
  [[nodiscard]] auto angleBetween(Vector const& v) const -> T;

  // Angle between two normalized vectors.
  [[nodiscard]] auto angleBetweenNormalized(Vector const& v) const -> T;

  [[nodiscard]] auto magnitudeSquared() const -> T;
  [[nodiscard]] auto magnitude() const -> T;

  void normalize();
  [[nodiscard]] auto normalized() const -> Vector;

  [[nodiscard]] auto projectOnto(Vector const& v) const -> Vector;

  [[nodiscard]] auto projectOntoNormalized(Vector const& v) const -> Vector;

  void negate();

  // Reverses order of components of vector
  void reverse();

  [[nodiscard]] auto abs() const -> Vector;
  [[nodiscard]] auto floor() const -> Vector;
  [[nodiscard]] auto ceil() const -> Vector;
  [[nodiscard]] auto round() const -> Vector;

  void fill(T const& v);
  void clamp(T const& min, T const& max);

  template <typename Function>
  void transform(Function&& function);

  template <typename Function>
  auto transformed(Function&& function) const -> Vector<decltype(std::declval<Function>()(std::declval<T>())), N>;

  auto operator-() const -> Vector;

  auto operator+(Vector const& v) const -> Vector;
  auto operator-(Vector const& v) const -> Vector;
  auto operator*(Vector const& v) const -> T;
  auto operator*(T s) const -> Vector;
  auto operator/(T s) const -> Vector;
  auto operator+=(Vector const& v) -> Vector&;
  auto operator-=(Vector const& v) -> Vector&;
  auto operator*=(T s) -> Vector&;
  auto operator/=(T s) -> Vector&;

  // Vector2

  // Return vector rotated to given angle
  template <std::size_t P = N>
  static auto withAngle(T angle, T magnitude = 1) -> Enable2D<P, Vector>;

  template <std::size_t P = N>
  static auto angleBetween2(Vector const& u, Vector const& v) -> Enable2D<P, T>;
  template <std::size_t P = N>
  static auto angleFormedBy2(Vector const& a, Vector const& b, Vector const& c) -> Enable2D<P, T>;
  template <std::size_t P = N>
  static auto angleFormedBy2(Vector const& a, Vector const& b, Vector const& c, std::function<Vector(Vector, Vector)> const& diff) -> Enable2D<P, T>;

  template <std::size_t P = N>
  auto rotate(T angle) const -> Enable2D<P, Vector>;

  // Faster than rotate(Constants::pi/2).
  template <std::size_t P = N>
  auto rot90() const -> Enable2D<P, Vector>;

  // Angle of vector on 2d plane, in the range [-pi, pi]
  template <std::size_t P = N>
  auto angle() const -> Enable2D<P, T>;

  // Returns polar coordinates of this cartesian vector
  template <std::size_t P = N>
  auto toPolar() const -> Enable2D<P, Vector>;

  // Returns cartesian coordinates of this polar vector
  template <std::size_t P = N>
  auto toCartesian() const -> Enable2D<P, Vector>;

  template <std::size_t P = N>
  [[nodiscard]] auto x() const -> Enable2DOrHigher<P, T> const&;
  template <std::size_t P = N>
  [[nodiscard]] auto y() const -> Enable2DOrHigher<P, T> const&;

  template <std::size_t P = N>
  auto setX(T const& t) -> Enable2DOrHigher<P>;
  template <std::size_t P = N>
  auto setY(T const& t) -> Enable2DOrHigher<P>;

  // Vector3

  template <std::size_t P = N>
  static auto fromAngles(T psi, T theta) -> Enable3D<P, Vector>;
  template <std::size_t P = N>
  static auto fromAnglesEnu(T psi, T theta) -> Enable3D<P, Vector>;
  template <std::size_t P = N>
  static auto tripleScalarProduct(Vector const& u, Vector const& v, Vector const& w) -> Enable3D<P, T>;
  template <std::size_t P = N>
  static auto angle(Vector const& v1, Vector const& v2) -> Enable3D<P, T>;

  template <std::size_t P = N>
  auto psi() const -> Enable3D<P, T>;
  template <std::size_t P = N>
  auto theta() const -> Enable3D<P, T>;
  template <std::size_t P = N>
  auto eulers() const -> Enable3D<P, Vector<T, 2>>;

  template <std::size_t P = N>
  auto psiEnu() const -> Enable3D<P, T>;
  template <std::size_t P = N>
  auto thetaEnu() const -> Enable3D<P, T>;

  template <std::size_t P = N>
  auto nedToEnu() const -> Enable3D<P, Vector>;
  template <std::size_t P = N>
  auto enuToNed() const -> Enable3D<P, Vector>;

  template <std::size_t P = N>
  [[nodiscard]] auto z() const -> Enable3DOrHigher<P, T> const&;

  template <std::size_t P = N>
  auto setZ(T const& t) -> Enable3DOrHigher<P>;

  // Vector4

  template <std::size_t P = N>
  auto w() const -> Enable4DOrHigher<P, T> const&;

  template <std::size_t P = N>
  auto setW(T const& t) -> Enable4DOrHigher<P>;

  using Base::size;
  using Base::empty;
};

using Vec2I = Vector<int, 2>;
using Vec2U = Vector<unsigned, 2>;
using Vec2F = Vector<float, 2>;
using Vec2D = Vector<double, 2>;
using Vec2B = Vector<std::uint8_t, 2>;
using Vec2S = Vector<std::size_t, 2>;

using Vec3I = Vector<int, 3>;
using Vec3U = Vector<unsigned, 3>;
using Vec3F = Vector<float, 3>;
using Vec3D = Vector<double, 3>;
using Vec3B = Vector<std::uint8_t, 3>;
using Vec3S = Vector<std::size_t, 3>;

using Vec4I = Vector<int, 4>;
using Vec4U = Vector<unsigned, 4>;
using Vec4F = Vector<float, 4>;
using Vec4D = Vector<double, 4>;
using Vec4B = Vector<std::uint8_t, 4>;
using Vec4S = Vector<std::size_t, 4>;

template <typename T, std::size_t N>
auto operator<<(std::ostream& os, Vector<T, N> const& v) -> std::ostream&;

template <typename T, std::size_t N>
auto operator*(T s, Vector<T, N> v) -> Vector<T, N>;

template <typename T, std::size_t N>
auto vnorm(Vector<T, N> v) -> Vector<T, N>;

template <typename T, std::size_t N>
auto vmag(Vector<T, N> const& v) -> T;

template <typename T, std::size_t N>
auto vmagSquared(Vector<T, N> const& v) -> T;

template <typename T, std::size_t N>
auto vmin(Vector<T, N> const& a, Vector<T, N> const& b) -> Vector<T, N>;

template <typename T, std::size_t N>
auto vmax(Vector<T, N> const& a, Vector<T, N> const& b) -> Vector<T, N>;

template <typename T, std::size_t N>
auto vclamp(Vector<T, N> const& a, Vector<T, N> const& min, Vector<T, N> const& max) -> Vector<T, N>;

template <typename VectorType>
auto vmult(VectorType const& a, VectorType const& b) -> VectorType;

template <typename VectorType>
auto vdiv(VectorType const& a, VectorType const& b) -> VectorType;

// Returns the cross product
template <typename T>
auto operator^(Vector<T, 3> v1, Vector<T, 3> v2) -> Vector<T, 3>;

// Returns the cross product / determinant
template <typename T>
auto operator^(Vector<T, 2> const& v1, Vector<T, 2> const& v2) -> T;

template <typename T, std::size_t N>
struct hash<Vector<T, N>> : hash<Array<T, N>> {};

template <typename T, std::size_t N>
auto Vector<T, N>::filled(T const& t) -> Vector<T, N> {
  Vector v;
  for (std::size_t i = 0; i < N; ++i)
    v[i] = t;
  return v;
}

template <typename T, std::size_t N>
template <typename T2>
auto Vector<T, N>::floor(Vector<T2, N> const& v) -> Vector<T, N> {
  Vector vec;
  for (std::size_t i = 0; i < N; ++i)
    vec[i] = std::floor(v[i]);
  return vec;
}

template <typename T, std::size_t N>
template <typename T2>
auto Vector<T, N>::ceil(Vector<T2, N> const& v) -> Vector<T, N> {
  Vector vec;
  for (std::size_t i = 0; i < N; ++i)
    vec[i] = std::ceil(v[i]);
  return vec;
}

template <typename T, std::size_t N>
template <typename T2>
auto Vector<T, N>::round(Vector<T2, N> const& v) -> Vector<T, N> {
  Vector vec;
  for (std::size_t i = 0; i < N; ++i)
    vec[i] = std::round(v[i]);
  return vec;
}

template <typename T, std::size_t N>
template <typename Iterator>
auto Vector<T, N>::copyFrom(Iterator p) -> Vector<T, N> {
  Vector v;
  for (std::size_t i = 0; i < N; ++i)
    v[i] = *(p++);
  return v;
}

template <typename T, std::size_t N>
Vector<T, N>::Vector() = default;

template <typename T, std::size_t N>
Vector<T, N>::Vector(T const& e1)
  : Base(e1) {}

template <typename T, std::size_t N>
template <typename... TN>
Vector<T, N>::Vector(T const& e1, TN const&... rest)
  : Base(e1, rest...) {}

template <typename T, std::size_t N>
template <typename T2>
Vector<T, N>::Vector(Array<T2, N> const& v)
  : Base(v) {}

template <typename T, std::size_t N>
template <typename T2, typename T3>
Vector<T, N>::Vector(Array<T2, N - 1> const& u, T3 const& v) {
  for (std::size_t i = 0; i < N - 1; ++i) {
    Base::operator[](i) = u[i];
  }
  Base::operator[](N - 1) = v;
}

template <typename T, std::size_t N>
template <std::size_t N2>
auto Vector<T, N>::toSize() const -> Vector<T, N2> {
  Vector<T, N2> r;
  constexpr std::size_t ns = std::min(N2, N);
  for (std::size_t i = 0; i < ns; ++i)
    r[i] = (*this)[i];
  return r;
}

template <typename T, std::size_t N>
auto Vector<T, N>::vec2() const -> Vector<T, 2> {
  return toSize<2>();
}

template <typename T, std::size_t N>
auto Vector<T, N>::vec3() const -> Vector<T, 3> {
  return toSize<3>();
}

template <typename T, std::size_t N>
auto Vector<T, N>::vec4() const -> Vector<T, 4> {
  return toSize<4>();
}

template <typename T, std::size_t N>
auto Vector<T, N>::piecewiseMultiply(Vector const& v2) const -> Vector<T, N> {
  return combine(v2, std::multiplies<T>());
}

template <typename T, std::size_t N>
auto Vector<T, N>::piecewiseDivide(Vector const& v2) const -> Vector<T, N> {
  return combine(v2, std::divides<T>());
}

template <typename T, std::size_t N>
auto Vector<T, N>::piecewiseMin(Vector const& v2) const -> Vector<T, N> {
  Vector r;
  for (std::size_t i = 0; i < N; ++i)
    r[i] = std::min((*this)[i], v2[i]);
  return r;
}

template <typename T, std::size_t N>
auto Vector<T, N>::piecewiseMax(Vector const& v2) const -> Vector<T, N> {
  Vector r;
  for (std::size_t i = 0; i < N; ++i)
    r[i] = std::max((*this)[i], v2[i]);
  return r;
}

template <typename T, std::size_t N>
auto Vector<T, N>::piecewiseClamp(Vector const& min, Vector const& max) const -> Vector<T, N> {
  Vector r;
  for (std::size_t i = 0; i < N; ++i)
    r[i] = std::max(std::min((*this)[i], max[i]), min[i]);
  return r;
}

template <typename T, std::size_t N>
auto Vector<T, N>::min() const -> T {
  T s = (*this)[0];
  for (std::size_t i = 1; i < N; ++i)
    s = std::min(s, (*this)[i]);
  return s;
}

template <typename T, std::size_t N>
auto Vector<T, N>::max() const -> T {
  T s = (*this)[0];
  for (std::size_t i = 1; i < N; ++i)
    s = std::max(s, (*this)[i]);
  return s;
}

template <typename T, std::size_t N>
auto Vector<T, N>::sum() const -> T {
  T s = (*this)[0];
  for (std::size_t i = 1; i < N; ++i)
    s += (*this)[i];
  return s;
}

template <typename T, std::size_t N>
auto Vector<T, N>::product() const -> T {
  T p = (*this)[0];
  for (std::size_t i = 1; i < N; ++i)
    p *= (*this)[i];
  return p;
}

template <typename T, std::size_t N>
template <typename Function>
auto Vector<T, N>::combine(Vector const& v, Function f) const -> Vector<T, N> {
  Vector r;
  for (std::size_t i = 0; i < N; ++i)
    r[i] = f((*this)[i], v[i]);
  return r;
}

template <typename T, std::size_t N>
auto Vector<T, N>::angleBetween(Vector const& v) const -> T {
  return std::acos(this->normalized() * v.normalized());
}

template <typename T, std::size_t N>
auto Vector<T, N>::angleBetweenNormalized(Vector const& v) const -> T {
  return std::acos(*this * v);
}

template <typename T, std::size_t N>
auto Vector<T, N>::magnitudeSquared() const -> T {
  T m = 0;
  for (std::size_t i = 0; i < N; ++i)
    m += square((*this)[i]);
  return m;
}

template <typename T, std::size_t N>
auto Vector<T, N>::magnitude() const -> T {
  return std::sqrt(magnitudeSquared());
}

template <typename T, std::size_t N>
void Vector<T, N>::normalize() {
  T m = magnitude();
  if (m != 0)
    *this = (*this) / m;
}

template <typename T, std::size_t N>
auto Vector<T, N>::normalized() const -> Vector<T, N> {
  T m = magnitude();
  if (m != 0)
    return (*this) / m;
  else
    return *this;
}

template <typename T, std::size_t N>
auto Vector<T, N>::projectOnto(Vector const& v) const -> Vector<T, N> {
  T m = v.magnitudeSquared();
  if (m != 0)
    return projectOntoNormalized(v) / m;
  else
    return Vector();
}

template <typename T, std::size_t N>
auto Vector<T, N>::projectOntoNormalized(Vector const& v) const -> Vector<T, N> {
  return ((*this) * v) * v;
}

template <typename T, std::size_t N>
auto Vector<T, N>::operator-() const -> Vector<T, N> {
  auto v = *this;
  v.negate();
  return v;
}

template <typename T, std::size_t N>
void Vector<T, N>::negate() {
  for (std::size_t i = 0; i < N; ++i)
    (*this)[i] = -(*this)[i];
}

template <typename T, std::size_t N>
auto Vector<T, N>::abs() const -> Vector<T, N> {
  Vector v;
  for (std::size_t i = 0; i < N; ++i)
    v[i] = std::fabs((*this)[i]);
  return v;
}

template <typename T, std::size_t N>
auto Vector<T, N>::floor() const -> Vector<T, N> {
  return floor(*this);
}

template <typename T, std::size_t N>
auto Vector<T, N>::ceil() const -> Vector<T, N> {
  return ceil(*this);
}

template <typename T, std::size_t N>
auto Vector<T, N>::round() const -> Vector<T, N> {
  return round(*this);
}

template <typename T, std::size_t N>
void Vector<T, N>::reverse() {
  std::reverse(Base::begin(), Base::end());
}

template <typename T, std::size_t N>
void Vector<T, N>::fill(T const& v) {
  Base::fill(v);
}

template <typename T, std::size_t N>
void Vector<T, N>::clamp(T const& min, T const& max) {
  for (std::size_t i = 0; i < N; ++i)
    (*this)[i] = std::max(min, std::min(max, (*this)[i]));
}

template <typename T, std::size_t N>
template <typename Function>
void Vector<T, N>::transform(Function&& function) {
  for (auto& e : *this)
    e = function(e);
}

template <typename T, std::size_t N>
template <typename Function>
auto Vector<T, N>::transformed(Function&& function) const -> Vector<decltype(std::declval<Function>()(std::declval<T>())), N> {
  return Star::transform<Vector<decltype(std::declval<Function>()(std::declval<T>())), N>>(*this, function);
}

template <typename T, std::size_t N>
auto Vector<T, N>::operator+(Vector const& v) const -> Vector<T, N> {
  Vector r;
  for (std::size_t i = 0; i < N; ++i)
    r[i] = (*this)[i] + v[i];
  return r;
}

template <typename T, std::size_t N>
auto Vector<T, N>::operator-(Vector const& v) const -> Vector<T, N> {
  Vector r;
  for (std::size_t i = 0; i < N; ++i)
    r[i] = (*this)[i] - v[i];
  return r;
}

template <typename T, std::size_t N>
auto Vector<T, N>::operator*(Vector const& v) const -> T {
  T sum = 0;
  for (std::size_t i = 0; i < N; ++i)
    sum += (*this)[i] * v[i];
  return sum;
}

template <typename T, std::size_t N>
auto Vector<T, N>::operator*(T s) const -> Vector<T, N> {
  Vector r;
  for (std::size_t i = 0; i < N; ++i)
    r[i] = (*this)[i] * s;
  return r;
}

template <typename T, std::size_t N>
auto Vector<T, N>::operator/(T s) const -> Vector<T, N> {
  Vector r;
  for (std::size_t i = 0; i < N; ++i)
    r[i] = (*this)[i] / s;
  return r;
}

template <typename T, std::size_t N>
auto Vector<T, N>::operator+=(Vector const& v) -> Vector<T, N>& {
  return (*this = *this + v);
}

template <typename T, std::size_t N>
auto Vector<T, N>::operator-=(Vector const& v) -> Vector<T, N>& {
  return (*this = *this - v);
}

template <typename T, std::size_t N>
auto Vector<T, N>::operator*=(T s) -> Vector<T, N>& {
  return (*this = *this * s);
}

template <typename T, std::size_t N>
auto Vector<T, N>::operator/=(T s) -> Vector<T, N>& {
  return (*this = *this / s);
}

// Vector2

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::withAngle(T angle, T magnitude) -> Enable2D<P, Vector<T, N>> {
  return Vector(std::cos(angle) * magnitude, std::sin(angle) * magnitude);
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::angleBetween2(Vector const& v1, Vector const& v2) -> Enable2D<P, T> {
  // TODO: Inefficient
  return v2.angle() - v1.angle();
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::angleFormedBy2(Vector const& a, Vector const& b, Vector const& c) -> Enable2D<P, T> {
  return angleBetween2(b - a, b - c);
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::angleFormedBy2(
    Vector const& a, Vector const& b, Vector const& c, std::function<Vector(Vector, Vector)> const& diff)
    -> Enable2D<P, T> {
  return angleBetween2(diff(b, a), diff(b, c));
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::angle() const -> Enable2D<P, T> {
  return atan2(Base::operator[](1), Base::operator[](0));
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::rotate(T a) const -> Enable2D<P, Vector<T, N>> {
  // TODO: Need a Matrix2
  T cosa = std::cos(a);
  T sina = std::sin(a);
  return Vector(
      Base::operator[](0) * cosa - Base::operator[](1) * sina, Base::operator[](0) * sina + Base::operator[](1) * cosa);
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::rot90() const -> Enable2D<P, Vector<T, N>> {
  return Vector(-y(), x());
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::toPolar() const -> Enable2D<P, Vector<T, N>> {
  return Vector(angle(), Base::magnitude());
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::toCartesian() const -> Enable2D<P, Vector<T, N>> {
  return vec2d(sin((*this)[0]) * (*this)[1], cos((*this)[0]) * (*this)[1]);
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::x() const -> Enable2DOrHigher<P, T> const & {
  return Base::operator[](0);
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::y() const -> Enable2DOrHigher<P, T> const & {
  return Base::operator[](1);
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::setX(T const& t) -> Enable2DOrHigher<P> {
  Base::operator[](0) = t;
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::setY(T const& t) -> Enable2DOrHigher<P> {
  Base::operator[](1) = t;
}

// Vector3

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::tripleScalarProduct(Vector const& a, Vector const& b, Vector const& c) -> Enable3D<P, T> {
  return a * (b ^ c);
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::theta() const -> Enable3D<P, T> {
  Vector<T, N> vn = std::norm(*this);
  T tmp = std::fabs(vn.z());
  if (tmp > 0.99999) {
    return tmp > 0.0 ? T(-Constants::pi / 2) : T(Constants::pi / 2);
  } else {
    return std::asin(-vn.z());
  }
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::psi() const -> Enable3D<P, T> {
  Vector<T, N> vn = std::norm(*this);
  T tmp = T(std::fabs(vn.z()));
  if (tmp > 0.99999) {
    return 0.0;
  } else {
    return T(std::atan2(vn.y(), vn.x()));
  }
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::thetaEnu() const -> Enable3D<P, T> {
  Vector<T, N> vn = std::norm(*this);
  T tmp = std::fabs(vn.z());
  if (tmp > 0.99999) {
    return tmp > 0.0 ? -Constants::pi / 2 : Constants::pi / 2;
  } else {
    return asin(vn.z());
  }
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::psiEnu() const -> Enable3D<P, T> {
  Vector<T, N> vn = std::norm(*this);
  T tmp = std::fabs(vn.z());
  if (tmp > 0.99999) {
    return 0.0;
  } else {
    return std::atan2(vn.x(), vn.y());
  }
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::eulers() const -> Enable3D<P, Vector<T, 2>> {
  T psi, theta;
  Vector<T, N> vn = std::norm(*this);
  T tmp = std::fabs(vn.z());
  if (tmp > 0.99999) {
    psi = 0.0;
    theta = tmp > 0.0 ? -Constants::pi / 2 : Constants::pi / 2;
  } else {
    psi = std::atan2(vn.y(), vn.x());
    theta = std::asin(-vn.z());
  }
  return Vector<T, 2>(psi, theta);
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::fromAngles(T psi, T theta) -> Enable3D<P, Vector<T, N>> {
  Vec3F nv;
  T cosTheta = T(std::cos(theta));

  nv.x() = T(std::cos(psi));
  nv.y() = T(std::sin(psi));
  nv.x() *= cosTheta;
  nv.y() *= cosTheta;
  nv.z() = T(-std::sin(theta));
  return nv;
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::fromAnglesEnu(T psi, T theta) -> Enable3D<P, Vector<T, N>> {
  Vector nv = fromAngles(psi, theta);
  return Vector(nv.y(), nv.x(), -nv.z());
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::angle(Vector const& v1, Vector const& v2) -> Enable3D<P, T> {
  return std::acos(std::min(norm(v1) * norm(v2), 1.0));
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::nedToEnu() const -> Enable3D<P, Vector<T, N>> {
  return Vector(y(), x(), -z());
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::enuToNed() const -> Enable3D<P, Vector<T, N>> {
  return Vector(y(), x(), -z());
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::z() const -> Enable3DOrHigher<P, T> const & {
  return Base::operator[](2);
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::setZ(T const& t) -> Enable3DOrHigher<P> {
  Base::operator[](2) = t;
}

// Vector4

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::w() const -> Enable4DOrHigher<P, T> const & {
  return Base::operator[](3);
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Vector<T, N>::setW(T const& t) -> Enable4DOrHigher<P> {
  Base::operator[](3) = t;
}

// Free Functions

template <typename T, std::size_t N>
auto operator<<(std::ostream& os, Vector<T, N> const& v) -> std::ostream& {
  os << '(';
  for (std::size_t i = 0; i < N; ++i) {
    os << v[i];
    if (i != N - 1)
      os << ", ";
  }
  os << ')';
  return os;
}

template <typename T, std::size_t N>
auto operator*(T s, Vector<T, N> v) -> Vector<T, N> {
  return v * s;
}

template <typename T, std::size_t N>
auto vnorm(Vector<T, N> v) -> Vector<T, N> {
  return v.normalized();
}

template <typename T, std::size_t N>
auto vmag(Vector<T, N> const& v) -> T {
  return v.magnitude();
}

template <typename T, std::size_t N>
auto vmagSquared(Vector<T, N> const& v) -> T {
  return v.magnitudeSquared();
}

template <typename T, std::size_t N>
auto vmin(Vector<T, N> const& a, Vector<T, N> const& b) -> Vector<T, N> {
  return a.piecewiseMin(b);
}

template <typename T, std::size_t N>
auto vmax(Vector<T, N> const& a, Vector<T, N> const& b) -> Vector<T, N> {
  return a.piecewiseMax(b);
}

template <typename T, std::size_t N>
auto vclamp(Vector<T, N> const& a, Vector<T, N> const& min, Vector<T, N> const& max) -> Vector<T, N> {
  return a.piecewiseClamp(min, max);
}

template <typename VectorType>
auto vmult(VectorType const& a, VectorType const& b) -> VectorType {
  return a.piecewiseMultiply(b);
}

template <typename VectorType>
auto vdiv(VectorType const& a, VectorType const& b) -> VectorType {
  return a.piecewiseDivide(b);
}

template <typename T>
auto operator^(Vector<T, 3> v1, Vector<T, 3> v2) -> Vector<T, 3> {
  return Vector<T, 3>(v1[1] * v2[2] - v1[2] * v2[1], v1[2] * v2[0] - v1[0] * v2[2], v1[0] * v2[1] - v1[1] * v2[0]);
}

template <typename T>
auto operator^(Vector<T, 2> const& v1, Vector<T, 2> const& v2) -> T {
  return v1[0] * v2[1] - v1[1] * v2[0];
}

}

template <typename T, std::size_t N>
struct std::formatter<Star::Vector<T, N>> : Star::ostream_formatter {};
