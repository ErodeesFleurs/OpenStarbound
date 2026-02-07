#pragma once

#include "StarBiMap.hpp"
#include "StarException.hpp"
#include "StarInterpolation.hpp"
#include "StarJson.hpp"
#include "StarRandom.hpp"

import std;

namespace Star {

using PerlinException = ExceptionDerived<"PerlinException">;

enum class PerlinType {
  Uninitialized,
  Perlin,
  Billow,
  RidgedMulti
};
extern EnumMap<PerlinType> const PerlinTypeNames;

int const PerlinSampleSize = 512;

template <typename Float>
class Perlin {
public:
  // Default constructed perlin noise is uninitialized and cannot be queried.
  Perlin();

  Perlin(unsigned octaves, Float freq, Float amp, Float bias, Float alpha, Float beta, std::uint64_t seed);
  Perlin(PerlinType type, unsigned octaves, Float freq, Float amp, Float bias, Float alpha, Float beta, std::uint64_t seed);
  Perlin(Json const& config, std::uint64_t seed);
  explicit Perlin(Json const& json);

  Perlin(Perlin const& perlin);
  Perlin(Perlin&& perlin);

  auto operator=(Perlin const& perlin) -> Perlin&;
  auto operator=(Perlin&& perlin) -> Perlin&;

  auto get(Float x) const -> Float;
  auto get(Float x, Float y) const -> Float;
  Float get(Float x, Float y, Float z) const;

  [[nodiscard]] auto type() const -> PerlinType;

  [[nodiscard]] auto octaves() const -> unsigned;
  auto frequency() const -> Float;
  auto amplitude() const -> Float;
  auto bias() const -> Float;
  auto alpha() const -> Float;
  auto beta() const -> Float;

  [[nodiscard]] auto toJson() const -> Json;

private:
  static auto s_curve(Float t) -> Float;
  static void setup(Float v, int& b0, int& b1, Float& r0, Float& r1);

  static auto at2(Float* q, Float rx, Float ry) -> Float;
  static auto at3(Float* q, Float rx, Float ry, Float rz) -> Float;

  auto noise1(Float arg) const -> Float;
  auto noise2(std::array<Float, 2>& vec) const -> Float;
  auto noise3(std::array<Float, 3>& vec) const -> Float;

  void normalize2(std::array<Float, 2>& v) const;
  void normalize3(std::array<Float, 3>& v) const;

  void init(std::uint64_t seed);

  auto perlin(Float x) const -> Float;
  auto perlin(Float x, Float y) const -> Float;
  auto perlin(Float x, Float y, Float z) const -> Float;

  auto ridgedMulti(Float x) const -> Float;
  auto ridgedMulti(Float x, Float y) const -> Float;
  auto ridgedMulti(Float x, Float y, Float z) const -> Float;

  auto billow(Float x) const -> Float;
  auto billow(Float x, Float y) const -> Float;
  auto billow(Float x, Float y, Float z) const -> Float;

  PerlinType m_type;
  std::uint64_t m_seed;

  int m_octaves;
  Float m_frequency;
  Float m_amplitude;
  Float m_bias;
  Float m_alpha;
  Float m_beta;

  // Only used for RidgedMulti
  Float m_offset;
  Float m_gain;

  std::unique_ptr<int[]> p;
  std::unique_ptr<Float[][3]> g3;
  std::unique_ptr<Float[][2]> g2;
  std::unique_ptr<Float[]> g1;
};

using PerlinF = Perlin<float>;
using PerlinD = Perlin<double>;

template <typename Float>
auto Perlin<Float>::s_curve(Float t) -> Float {
  return t * t * (3.0 - 2.0 * t);
}

template <typename Float>
void Perlin<Float>::setup(Float v, int& b0, int& b1, Float& r0, Float& r1) {
  int iv = floor(v);
  Float fv = v - iv;

  b0 = iv & (PerlinSampleSize - 1);
  b1 = (iv + 1) & (PerlinSampleSize - 1);
  r0 = fv;
  r1 = fv - 1.0;
}

template <typename Float>
auto Perlin<Float>::at2(Float* q, Float rx, Float ry) -> Float {
  return rx * q[0] + ry * q[1];
}

template <typename Float>
auto Perlin<Float>::at3(Float* q, Float rx, Float ry, Float rz) -> Float {
  return rx * q[0] + ry * q[1] + rz * q[2];
}

template <typename Float>
Perlin<Float>::Perlin() {
  m_type = PerlinType::Uninitialized;
  m_alpha = 0;
  m_amplitude = 0;
  m_frequency = 0;
  m_seed = 0;
  m_gain = 0;
  m_beta = 0;
  m_offset = 0;
  m_bias = 0;
  m_octaves = 0;
}

template <typename Float>
Perlin<Float>::Perlin(unsigned octaves, Float freq, Float amp, Float bias, Float alpha, Float beta, std::uint64_t seed) {
  m_type = PerlinType::Perlin;
  m_seed = seed;

  m_octaves = octaves;
  m_frequency = freq;
  m_amplitude = amp;
  m_bias = bias;
  m_alpha = alpha;
  m_beta = beta;

  // TODO: These ought to be configurable
  m_offset = 1.0;
  m_gain = 2.0;

  init(m_seed);
}

template <typename Float>
Perlin<Float>::Perlin(PerlinType type, unsigned octaves, Float freq, Float amp, Float bias, Float alpha, Float beta, std::uint64_t seed) {
  m_type = type;
  m_seed = seed;

  m_octaves = octaves;
  m_frequency = freq;
  m_amplitude = amp;
  m_bias = bias;
  m_alpha = alpha;
  m_beta = beta;

  // TODO: These ought to be configurable
  m_offset = 1.0;
  m_gain = 2.0;

  init(m_seed);
}

template <typename Float>
Perlin<Float>::Perlin(Json const& config, std::uint64_t seed)
    : Perlin(config.set("seed", seed)) {}

template <typename Float>
Perlin<Float>::Perlin(Json const& json) {
  m_seed = json.getUInt("seed");
  m_octaves = json.getInt("octaves", 1);
  m_frequency = json.getDouble("frequency", 1.0);
  m_amplitude = json.getDouble("amplitude", 1.0);
  m_bias = json.getDouble("bias", 0.0);
  m_alpha = json.getDouble("alpha", 2.0);
  m_beta = json.getDouble("beta", 2.0);

  m_offset = json.getDouble("offset", 1.0);
  m_gain = json.getDouble("gain", 2.0);

  m_type = PerlinTypeNames.getLeft(json.getString("type"));

  init(m_seed);
}

template <typename Float>
Perlin<Float>::Perlin(Perlin const& perlin) {
  *this = perlin;
}

template <typename Float>
Perlin<Float>::Perlin(Perlin&& perlin) {
  *this = std::move(perlin);
}

template <typename Float>
auto Perlin<Float>::operator=(Perlin const& perlin) -> Perlin<Float>& {
  if (perlin.m_type == PerlinType::Uninitialized) {
    m_type = PerlinType::Uninitialized;
    p.reset();
    g3.reset();
    g2.reset();
    g1.reset();

  } else if (this != &perlin) {
    m_type = perlin.m_type;
    m_seed = perlin.m_seed;
    m_octaves = perlin.m_octaves;
    m_frequency = perlin.m_frequency;
    m_amplitude = perlin.m_amplitude;
    m_bias = perlin.m_bias;
    m_alpha = perlin.m_alpha;
    m_beta = perlin.m_beta;
    m_offset = perlin.m_offset;
    m_gain = perlin.m_gain;

    p.reset(new int[PerlinSampleSize + PerlinSampleSize + 2]);
    g3.reset(new Float[PerlinSampleSize + PerlinSampleSize + 2][3]);
    g2.reset(new Float[PerlinSampleSize + PerlinSampleSize + 2][2]);
    g1.reset(new Float[PerlinSampleSize + PerlinSampleSize + 2]);

    std::memcpy(p.get(), perlin.p.get(), (PerlinSampleSize + PerlinSampleSize + 2) * sizeof(int));
    std::memcpy(g3.get(), perlin.g3.get(), (PerlinSampleSize + PerlinSampleSize + 2) * sizeof(Float) * 3);
    std::memcpy(g2.get(), perlin.g2.get(), (PerlinSampleSize + PerlinSampleSize + 2) * sizeof(Float) * 2);
    std::memcpy(g1.get(), perlin.g1.get(), (PerlinSampleSize + PerlinSampleSize + 2) * sizeof(Float));
  }

  return *this;
}

template <typename Float>
auto Perlin<Float>::operator=(Perlin&& perlin) -> Perlin<Float>& {
  m_type = perlin.m_type;
  m_seed = perlin.m_seed;
  m_octaves = perlin.m_octaves;
  m_frequency = perlin.m_frequency;
  m_amplitude = perlin.m_amplitude;
  m_bias = perlin.m_bias;
  m_alpha = perlin.m_alpha;
  m_beta = perlin.m_beta;
  m_offset = perlin.m_offset;
  m_gain = perlin.m_gain;

  p = std::move(perlin.p);
  g3 = std::move(perlin.g3);
  g2 = std::move(perlin.g2);
  g1 = std::move(perlin.g1);

  return *this;
}

template <typename Float>
auto Perlin<Float>::get(Float x) const -> Float {
  switch (m_type) {
  case PerlinType::Perlin:
    return perlin(x);
  case PerlinType::Billow:
    return billow(x);
  case PerlinType::RidgedMulti:
    return ridgedMulti(x);
  default:
    throw PerlinException("::get called on uninitialized Perlin");
  }
}

template <typename Float>
auto Perlin<Float>::get(Float x, Float y) const -> Float {
  switch (m_type) {
  case PerlinType::Perlin:
    return perlin(x, y);
  case PerlinType::Billow:
    return billow(x, y);
  case PerlinType::RidgedMulti:
    return ridgedMulti(x, y);
  default:
    throw PerlinException("::get called on uninitialized Perlin");
  }
}

template <typename Float>
auto Perlin<Float>::get(Float x, Float y, Float z) const -> Float {
  switch (m_type) {
  case PerlinType::Perlin:
    return perlin(x, y, z);
  case PerlinType::Billow:
    return billow(x, y, z);
  case PerlinType::RidgedMulti:
    return ridgedMulti(x, y, z);
  default:
    throw PerlinException("::get called on uninitialized Perlin");
  }
}

template <typename Float>
auto Perlin<Float>::type() const -> PerlinType {
  return m_type;
}

template <typename Float>
auto Perlin<Float>::octaves() const -> unsigned {
  return m_octaves;
}

template <typename Float>
auto Perlin<Float>::frequency() const -> Float {
  return m_frequency;
}

template <typename Float>
auto Perlin<Float>::amplitude() const -> Float {
  return m_amplitude;
}

template <typename Float>
auto Perlin<Float>::bias() const -> Float {
  return m_bias;
}

template <typename Float>
auto Perlin<Float>::alpha() const -> Float {
  return m_alpha;
}

template <typename Float>
auto Perlin<Float>::beta() const -> Float {
  return m_beta;
}

template <typename Float>
auto Perlin<Float>::toJson() const -> Json {
  return JsonObject{
    {"seed", m_seed},
    {"octaves", m_octaves},
    {"frequency", m_frequency},
    {"amplitude", m_amplitude},
    {"bias", m_bias},
    {"alpha", m_alpha},
    {"beta", m_beta},
    {"offset", m_offset},
    {"gain", m_gain},
    {"type", PerlinTypeNames.getRight(m_type)}};
}

template <typename Float>
inline auto Perlin<Float>::noise1(Float arg) const -> Float {
  int bx0, bx1;
  Float rx0, rx1, sx, u, v;

  setup(arg, bx0, bx1, rx0, rx1);

  sx = s_curve(rx0);
  u = rx0 * g1[p[bx0]];
  v = rx1 * g1[p[bx1]];

  return (lerp(sx, u, v));
}

template <typename Float>
inline auto Perlin<Float>::noise2(std::array<Float, 2>& vec) const -> Float {
  int bx0, bx1, by0, by1, b00, b10, b01, b11;
  Float rx0, rx1, ry0, ry1, sx, sy, a, b, u, v;
  int i, j;

  setup(vec[0], bx0, bx1, rx0, rx1);
  setup(vec[1], by0, by1, ry0, ry1);

  i = p[bx0];
  j = p[bx1];

  b00 = p[i + by0];
  b10 = p[j + by0];
  b01 = p[i + by1];
  b11 = p[j + by1];

  sx = s_curve(rx0);
  sy = s_curve(ry0);

  u = at2(g2[b00], rx0, ry0);
  v = at2(g2[b10], rx1, ry0);
  a = lerp(sx, u, v);

  u = at2(g2[b01], rx0, ry1);
  v = at2(g2[b11], rx1, ry1);
  b = lerp(sx, u, v);

  return lerp(sy, a, b);
}

template <typename Float>
inline auto Perlin<Float>::noise3(std::array<Float, 3>& vec) const -> Float {
  int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
  Float rx0, rx1, ry0, ry1, rz0, rz1, sx, sy, sz, a, b, c, d, u, v;
  int i, j;

  setup(vec[0], bx0, bx1, rx0, rx1);
  setup(vec[1], by0, by1, ry0, ry1);
  setup(vec[2], bz0, bz1, rz0, rz1);

  i = p[bx0];
  j = p[bx1];

  b00 = p[i + by0];
  b10 = p[j + by0];
  b01 = p[i + by1];
  b11 = p[j + by1];

  sx = s_curve(rx0);
  sy = s_curve(ry0);
  sz = s_curve(rz0);

  u = at3(g3[b00 + bz0], rx0, ry0, rz0);
  v = at3(g3[b10 + bz0], rx1, ry0, rz0);
  a = lerp(sx, u, v);

  u = at3(g3[b01 + bz0], rx0, ry1, rz0);
  v = at3(g3[b11 + bz0], rx1, ry1, rz0);
  b = lerp(sx, u, v);

  c = lerp(sy, a, b);

  u = at3(g3[b00 + bz1], rx0, ry0, rz1);
  v = at3(g3[b10 + bz1], rx1, ry0, rz1);
  a = lerp(sx, u, v);

  u = at3(g3[b01 + bz1], rx0, ry1, rz1);
  v = at3(g3[b11 + bz1], rx1, ry1, rz1);
  b = lerp(sx, u, v);

  d = lerp(sy, a, b);

  return lerp(sz, c, d);
}

template <typename Float>
void Perlin<Float>::normalize2(std::array<Float, 2>& v) const {
  Float s;

  s = sqrt(v[0] * v[0] + v[1] * v[1]);
  if (s == 0.0f) {
    v[0] = 1.0f;
    v[1] = 0.0f;
  } else {
    v[0] = v[0] / s;
    v[1] = v[1] / s;
  }
}

template <typename Float>
void Perlin<Float>::normalize3(std::array<Float, 3>& v) const {
  Float s;

  s = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
  if (s == 0.0f) {
    v[0] = 1.0f;
    v[1] = 0.0f;
    v[2] = 0.0f;
  } else {
    v[0] = v[0] / s;
    v[1] = v[1] / s;
    v[2] = v[2] / s;
  }
}

template <typename Float>
void Perlin<Float>::init(std::uint64_t seed) {
  RandomSource randomSource(seed);

  p.reset(new int[PerlinSampleSize + PerlinSampleSize + 2]);
  g3.reset(new Float[PerlinSampleSize + PerlinSampleSize + 2][3]);
  g2.reset(new Float[PerlinSampleSize + PerlinSampleSize + 2][2]);
  g1.reset(new Float[PerlinSampleSize + PerlinSampleSize + 2]);

  int i, j, k;

  for (i = 0; i < PerlinSampleSize; i++) {
    p[i] = i;
    g1[i] = (Float)(randomSource.randInt(-PerlinSampleSize, PerlinSampleSize)) / PerlinSampleSize;

    for (j = 0; j < 2; j++)
      g2[i][j] = (Float)(randomSource.randInt(-PerlinSampleSize, PerlinSampleSize)) / PerlinSampleSize;
    normalize2(g2[i]);

    for (j = 0; j < 3; j++)
      g3[i][j] = (Float)(randomSource.randInt(-PerlinSampleSize, PerlinSampleSize)) / PerlinSampleSize;
    normalize3(g3[i]);
  }

  while (--i) {
    k = p[i];
    p[i] = p[j = randomSource.randUInt(PerlinSampleSize - 1)];
    p[j] = k;
  }

  for (i = 0; i < PerlinSampleSize + 2; i++) {
    p[PerlinSampleSize + i] = p[i];
    g1[PerlinSampleSize + i] = g1[i];
    for (j = 0; j < 2; j++)
      g2[PerlinSampleSize + i][j] = g2[i][j];
    for (j = 0; j < 3; j++)
      g3[PerlinSampleSize + i][j] = g3[i][j];
  }
}

template <typename Float>
inline auto Perlin<Float>::perlin(Float x) const -> Float {
  int i;
  Float val, sum = 0, scale = 1;
  Float p;

  p = x * m_frequency;
  for (i = 0; i < m_octaves; i++) {
    val = noise1(p);
    sum += val / scale;
    scale *= m_alpha;
    p *= m_beta;
  }
  return sum * m_amplitude + m_bias;
}

template <typename Float>
inline auto Perlin<Float>::perlin(Float x, Float y) const -> Float {
  int i;
  Float val, sum = 0;
  Float scale = 1;
  std::array<Float, 2> p;

  p[0] = x * m_frequency;
  p[1] = y * m_frequency;
  for (i = 0; i < m_octaves; i++) {
    val = noise2(p);
    sum += val / scale;
    scale *= m_alpha;
    p[0] *= m_beta;
    p[1] *= m_beta;
  }
  return sum * m_amplitude + m_bias;
}

template <typename Float>
inline auto Perlin<Float>::perlin(Float x, Float y, Float z) const -> Float {
  int i;
  Float val, sum = 0, scale = 1;
  ;
  std::array<Float, 3> p;

  p[0] = x * m_frequency;
  p[1] = y * m_frequency;
  p[2] = z * m_frequency;
  for (i = 0; i < m_octaves; i++) {
    val = noise3(p);
    sum += val / scale;
    scale *= m_alpha;
    p[0] *= m_beta;
    p[1] *= m_beta;
    p[2] *= m_beta;
  }

  return sum * m_amplitude + m_bias;
}

template <typename Float>
inline auto Perlin<Float>::ridgedMulti(Float x) const -> Float {
  Float val, sum = 0;
  Float scale = 1;
  Float weight = 1.0;

  x *= m_frequency;
  for (int i = 0; i < m_octaves; ++i) {
    val = noise1(x);

    val = m_offset - fabs(val);
    val *= val;
    val *= weight;

    weight = clamp<Float>(val * m_gain, 0.0, 1.0);

    sum += val / scale;
    scale *= m_alpha;
    x *= m_beta;
  }

  return ((sum * 1.25) - 1.0) * m_amplitude + m_bias;
}

template <typename Float>
inline auto Perlin<Float>::ridgedMulti(Float x, Float y) const -> Float {
  Float val, sum = 0, scale = 1;
  std::array<Float, 2> p;
  Float weight = 1.0;

  p[0] = x * m_frequency;
  p[1] = y * m_frequency;
  for (int i = 0; i < m_octaves; ++i) {
    val = noise2(p);

    val = m_offset - fabs(val);
    val *= val;
    val *= weight;

    weight = clamp<Float>(val * m_gain, 0.0, 1.0);

    sum += val / scale;
    scale *= m_alpha;
    p[0] *= m_beta;
    p[1] *= m_beta;
  }

  return ((sum * 1.25) - 1.0) * m_amplitude + m_bias;
}

template <typename Float>
inline Float Perlin<Float>::ridgedMulti(Float x, Float y, Float z) const {
  Float val, sum = 0, scale = 1;
  std::array<Float, 3> p;
  Float weight = 1.0;

  p[0] = x * m_frequency;
  p[1] = y * m_frequency;
  p[2] = z * m_frequency;
  for (int i = 0; i < m_octaves; ++i) {
    val = noise3(p);

    val = m_offset - fabs(val);
    val *= val;
    val *= weight;

    weight = clamp<Float>(val * m_gain, 0.0, 1.0);

    sum += val / scale;
    scale *= m_alpha;
    p[0] *= m_beta;
    p[1] *= m_beta;
    p[2] *= m_beta;
  }

  return ((sum * 1.25) - 1.0) * m_amplitude + m_bias;
}

template <typename Float>
inline auto Perlin<Float>::billow(Float x) const -> Float {
  Float val, sum = 0;
  Float p, scale = 1;

  p = x * m_frequency;
  for (int i = 0; i < m_octaves; i++) {
    val = noise1(p);
    val = 2.0 * fabs(val) - 1.0;

    sum += val / scale;
    scale *= m_alpha;
    p *= m_beta;
  }
  return (sum + 0.5) * m_amplitude + m_bias;
}

template <typename Float>
inline auto Perlin<Float>::billow(Float x, Float y) const -> Float {
  Float val, sum = 0, scale = 1;
  std::array<Float, 2> p;

  p[0] = x * m_frequency;
  p[1] = y * m_frequency;
  for (int i = 0; i < m_octaves; i++) {
    val = noise2(p);
    val = 2.0 * fabs(val) - 1.0;

    sum += val / scale;
    scale *= m_alpha;
    p[0] *= m_beta;
    p[1] *= m_beta;
  }
  return (sum + 0.5) * m_amplitude + m_bias;
}

template <typename Float>
inline auto Perlin<Float>::billow(Float x, Float y, Float z) const -> Float {
  Float val, sum = 0, scale = 1;
  std::array<Float, 3> p;

  p[0] = x * m_frequency;
  p[1] = y * m_frequency;
  p[2] = z * m_frequency;
  for (int i = 0; i < m_octaves; i++) {
    val = noise3(p);
    val = 2.0 * fabs(val) - 1.0;

    sum += val / scale;
    scale *= m_alpha;
    p[0] *= m_beta;
    p[1] *= m_beta;
    p[2] *= m_beta;
  }

  return (sum + 0.5) * m_amplitude + m_bias;
}

}// namespace Star
