#pragma once

#include "StarException.hpp"

import std;

namespace Star {

using MathException = ExceptionDerived<"MathException">;

namespace Constants {
  double constexpr pi = std::numbers::pi;
  double constexpr rad2deg = 57.2957795130823208768;
  double constexpr deg2rad = 1 / rad2deg;
  double constexpr sqrt2 = std::numbers::sqrt2;
  double constexpr log2e = std::numbers::log2e;
}


inline auto log2(float f) -> float {
  return std::log(f) * (float)Constants::log2e;
}

inline auto log2(double d) -> double {
  return std::log(d) * Constants::log2e;
}

// Count the number of '1' bits in the given unsigned integer
template <typename Int>
auto countSetBits(Int value) -> unsigned requires (std::is_integral_v<Int> && std::is_unsigned_v<Int>) {
  unsigned count = 0;
  while (value != 0) {
    value &= (value - 1);
    ++count;
  }
  return count;
}

template <typename T, typename T2>
auto
nearEqual(T x, T2 y, unsigned ulp) -> bool requires (!std::numeric_limits<T>::is_integer && !std::numeric_limits<T2>::is_integer && sizeof(T) >= sizeof(T2)) {
  auto epsilon = std::numeric_limits<T>::epsilon();
  return abs(x - y) <= epsilon * max(abs(x), (T)abs(y)) * ulp;
}

template <typename T, typename T2>
auto
nearEqual(T x, T2 y, unsigned ulp) -> bool requires (!std::numeric_limits<T>::is_integer && !std::numeric_limits<T2>::is_integer && sizeof(T) < sizeof(T2)) {
  return nearEqual(y, x, ulp);
}

template <typename T, typename T2>
auto
nearEqual(T x, T2 y, unsigned ulp) -> bool requires (std::numeric_limits<T>::is_integer && !std::numeric_limits<T2>::is_integer) {
  return nearEqual((double)x, y, ulp);
}

template <typename T, typename T2>
auto
nearEqual(T x, T2 y, unsigned ulp) -> bool requires (!std::numeric_limits<T>::is_integer && std::numeric_limits<T2>::is_integer) {
  return nearEqual(x, (double)y, ulp);
}

template <typename T, typename T2>
auto
nearEqual(T x, T2 y, unsigned) -> bool requires (std::numeric_limits<T>::is_integer && std::numeric_limits<T2>::is_integer) {
  return x == y;
}

template <typename T, typename T2>
auto nearEqual(T x, T2 y) -> bool {
  return nearEqual(x, y, 1);
}

template <typename T>
auto nearZero(T x, unsigned ulp = 2) -> bool requires (!std::numeric_limits<T>::is_integer) {
  return abs(x) <= std::numeric_limits<T>::min() * ulp;
}

template <typename T>
auto nearZero(T x) -> bool requires std::numeric_limits<T>::is_integer {
  return x == 0;
}

template <typename T>
constexpr auto lowest() -> T {
  return std::numeric_limits<T>::lowest();
}

template <typename T>
constexpr auto highest() -> T {
  return std::numeric_limits<T>::max();
}

template <typename T>
constexpr auto square(T const& x) -> T {
  return x * x;
}

template <typename T>
constexpr auto cube(T const& x) -> T {
  return x * x * x;
}

template <typename Float>
auto ipart(Float f) -> int {
  return (int)std::floor(f);
}

template <typename Float>
auto fpart(Float f) -> Float {
  return f - ipart(f);
}

template <typename Float>
auto rfpart(Float f) -> Float {
  return 1.0 - fpart(f);
}

template <typename T, typename T2>
auto clampMagnitude(T const& v, T2 const& mag) -> T {
  if (v > mag)
    return mag;
  else if (v < -mag)
    return -mag;
  else
    return v;
}

template <typename T>
auto clamp(T const val, T const min, T const max) -> T {
  return std::min(std::max(val, min), max);
}

template <typename T>
auto clampDynamic(T const val, T const a, T const b) -> T {
  return std::min(std::max(val, std::min(a, b)), std::max(a, b));
}

template <typename IntType, typename PowType>
auto intPow(IntType i, PowType p) -> IntType {

  if (p == 0)
    return 1;
  if (p == 1)
    return i;

  IntType tmp = intPow(i, p / 2);
  if ((p % 2) == 0)
    return tmp * tmp;
  else
    return i * tmp * tmp;
}

template <typename Int>
auto isPowerOf2(Int x) -> bool {
  if (x < 1)
    return false;
  return (x & (x - 1)) == 0;
}

inline auto ceilPowerOf2(std::uint64_t v) -> std::uint64_t {
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v |= v >> 32;
  v++;
  return v;
}

template <typename Float>
auto sigmoid(Float x) -> Float {
  return 1 / (1 + std::exp(-x));
}

// returns a % m such that the answer is always positive.
// For example, -1 mod 10 is 9.
template <typename IntType>
auto pmod(IntType a, IntType m) -> IntType {
  IntType r = a % m;
  return r < 0 ? r + m : r;
}

// Same as pmod but for float like values.
template <typename Float>
auto pfmod(Float a, Float m) -> Float {
  if (m == 0)
    return a;

  return a - m * std::floor(a / m);
}

// Finds the *smallest* distance (in absolute value terms) from b to a (a - b)
// in a non-euclidean wrapping number line.  Suppose size is 100, wrapDiff(10,
// 109) would return 1, because 509 is congruent to the point 9.  On the other
// hand, wrapDiff(10, 111) would return -1, because 111 is congruent to the
// point 11.
template <typename Type>
auto wrapDiff(Type a, Type b, Type size) -> Type {
  a = pmod(a, size);
  b = pmod(b, size);

  Type diff = a - b;
  if (diff > size / 2)
    diff -= size;
  else if (diff < -size / 2)
    diff += size;

  return diff;
}

// Sampe as wrapDiff but for float like values
template <typename Type>
auto wrapDiffF(Type a, Type b, Type size) -> Type {
  a = pfmod(a, size);
  b = pfmod(b, size);

  Type diff = a - b;
  if (diff > size / 2)
    diff -= size;
  else if (diff < -size / 2)
    diff += size;

  return diff;
}

// like std::pow, except ignores sign, and the return value will match the sign
// of the value passed in.  ppow(-2, 2) == -4
template <typename Float>
auto ppow(Float val, Float pow) -> Float {
  return copysign(std::pow(std::fabs(val), pow), val);
}

// Returns angle wrapped around to the range [-pi, pi).
template <typename Float>
auto constrainAngle(Float angle) -> Float {
  angle = fmod((Float)(angle + Constants::pi), (Float)(Constants::pi * 2));
  if (angle < 0)
    angle += Constants::pi * 2;
  return angle - Constants::pi;
}

// Returns the closest angle movement to go from the given angle to the target
// angle, in radians.
template <typename Float>
auto angleDiff(Float angle, Float targetAngle) -> Float {
  double diff = fmod((Float)(targetAngle - angle + Constants::pi), (Float)(Constants::pi * 2));
  if (diff < 0)
    diff += Constants::pi * 2;
  return diff - Constants::pi;
}

// Approach the given goal value from the current value, at a maximum rate of
// change.  Rate should always be a positive value. (T must be signed).
template <typename T>
auto approach(T goal, T current, T rate) -> T {
  if (goal < current) {
    return max(current - rate, goal);
  } else if (goal > current) {
    return min(current + rate, goal);
  } else {
    return current;
  }
}

// Same as approach, specialied for angles, and always approaches from the
// closest absolute direction.
template <typename T>
auto approachAngle(T goal, T current, T rate) -> T {
  return constrainAngle(current + clampMagnitude<T>(angleDiff(current, goal), rate));
}

// Used in color conversion from floating point to std::uint8_t
inline auto floatToByte(float val, bool doClamp = false) -> std::uint8_t {
  if (doClamp)
    val = clamp(val, 0.0f, 1.0f);
  return (std::uint8_t)(val * 255.0f);
}

// Used in color conversion from std::uint8_t to normalized float.
inline auto byteToFloat(std::uint8_t val) -> float {
  return val / 255.0f;
}

// Turn a randomized floating point value from [0.0, 1.0] to [-1.0, 1.0]
template <typename Float>
auto randn(Float val) -> Float {
  return val * 2 - 1;
}

// Increments a value between min and max inclusive, cycling around to min when
// it would be incremented beyond max.  If the value is outside of the range,
// the next increment will start at min.
template <typename Integer>
auto cycleIncrement(Integer val, Integer min, Integer max) -> Integer {
  if (val < min || val >= max)
    return min;
  else
    return val + 1;
}

}
