#pragma once

#include "StarBiMap.hpp"
#include "StarException.hpp"
#include "StarInterpolation.hpp"
#include "StarJson.hpp"
#include "StarRandom.hpp"

import std;

namespace Star {

using PerlinException = ExceptionDerived<"PerlinException">;

enum class PerlinType : std::uint8_t {
  Uninitialized,
  Perlin,
  Billow,
  RidgedMulti
};
extern EnumMap<PerlinType> const PerlinTypeNames;

constexpr int PerlinSampleSize = 512;

template <std::floating_point Float>
class Perlin {
public:
  // Default constructed perlin noise is uninitialized and cannot be queried.
  Perlin() = default;

  Perlin(unsigned octaves, Float freq, Float amp, Float bias, Float alpha, Float beta, std::uint64_t seed)
      : Perlin(PerlinType::Perlin, octaves, freq, amp, bias, alpha, beta, seed) {}

  Perlin(PerlinType type, unsigned octaves, Float freq, Float amp, Float bias, Float alpha, Float beta, std::uint64_t seed)
      : m_type(type), m_seed(seed), m_octaves(static_cast<int>(octaves)), m_frequency(freq), m_amplitude(amp), m_bias(bias), m_alpha(alpha), m_beta(beta) {
    init(seed);
  }

  Perlin(Json const& config, std::uint64_t seed)
      : m_type(PerlinTypeNames.getLeft(config.getString("type", "perlin"))),
        m_seed(seed),
        m_octaves(config.getInt("octaves", 1)),
        m_frequency(config.getFloat("frequency", 1.0)),
        m_amplitude(config.getFloat("amplitude", 1.0)),
        m_bias(config.getFloat("bias", 0.0)),
        m_alpha(config.getFloat("alpha", 2.0)),
        m_beta(config.getFloat("beta", 0.5)),
        m_offset(config.getFloat("offset", 1.0)),
        m_gain(config.getFloat("gain", 2.0)) {
    init(seed);
  }

  explicit Perlin(Json const& json) : Perlin(json, json.getUInt("seed", 0)) {}

  auto get(Float x) const -> Float {
    using enum PerlinType;
    switch (m_type) {
    case Perlin: return perlin(x);
    case RidgedMulti: return ridgedMulti(x);
    case Billow: return billow(x);
    default: throw PerlinException("Perlin not initialized");
    }
  }

  auto get(Float x, Float y) const -> Float {
    using enum PerlinType;
    switch (m_type) {
    case Perlin: return perlin(x, y);
    case RidgedMulti: return ridgedMulti(x, y);
    case Billow: return billow(x, y);
    default: throw PerlinException("Perlin not initialized");
    }
  }

  auto get(Float x, Float y, Float z) const -> Float {
    using enum PerlinType;
    switch (m_type) {
    case Perlin: return perlin(x, y, z);
    case RidgedMulti: return ridgedMulti(x, y, z);
    case Billow: return billow(x, y, z);
    default: throw PerlinException("Perlin not initialized");
    }
  }

  [[nodiscard]] auto type() const noexcept -> PerlinType { return m_type; }
  [[nodiscard]] auto octaves() const noexcept -> int { return m_octaves; }
  auto frequency() const noexcept -> Float { return m_frequency; }
  auto amplitude() const noexcept -> Float { return m_amplitude; }
  auto bias() const noexcept -> Float { return m_bias; }
  auto alpha() const noexcept -> Float { return m_alpha; }
  auto beta() const noexcept -> Float { return m_beta; }

  [[nodiscard]] auto toJson() const -> Json {
    return JsonObject{
      {"type", PerlinTypeNames.getRight(m_type)},
      {"octaves", m_octaves},
      {"frequency", m_frequency},
      {"amplitude", m_amplitude},
      {"bias", m_bias},
      {"alpha", m_alpha},
      {"beta", m_beta},
      {"seed", m_seed},
      {"offset", m_offset},
      {"gain", m_gain}};
  }

private:
  static constexpr auto s_curve(Float t) noexcept -> Float {
    return t * t * (3.0 - 2.0 * t);
  }

  static constexpr void setup(Float v, int& b0, int& b1, Float& r0, Float& r1) noexcept {
    int iv = static_cast<int>(std::floor(v));
    Float fv = v - iv;

    b0 = iv & (PerlinSampleSize - 1);
    b1 = (iv + 1) & (PerlinSampleSize - 1);
    r0 = fv;
    r1 = fv - 1.0;
  }

  static constexpr auto at2(const Float* q, Float rx, Float ry) noexcept -> Float {
    return rx * q[0] + ry * q[1];
  }

  static constexpr auto at3(const Float* q, Float rx, Float ry, Float rz) noexcept -> Float {
    return rx * q[0] + ry * q[1] + rz * q[2];
  }

  auto noise1(Float arg) const -> Float {
    int bx0, bx1;
    Float rx0, rx1;
    setup(arg, bx0, bx1, rx0, rx1);

    Float sx = s_curve(rx0);
    Float u = rx0 * m_g1[m_p[bx0]];
    Float v = rx1 * m_g1[m_p[bx1]];

    return Star::lerp(sx, u, v);
  }

  auto noise2(std::span<const Float, 2> vec) const -> Float {
    int bx0, bx1, by0, by1;
    Float rx0, rx1, ry0, ry1;

    setup(vec[0], bx0, bx1, rx0, rx1);
    setup(vec[1], by0, by1, ry0, ry1);

    int i = m_p[bx0];
    int j = m_p[bx1];

    int b00 = m_p[i + by0];
    int b10 = m_p[j + by0];
    int b01 = m_p[i + by1];
    int b11 = m_p[j + by1];

    Float sx = s_curve(rx0);
    Float sy = s_curve(ry0);

    Float u = at2(m_g2[b00].data(), rx0, ry0);
    Float v = at2(m_g2[b10].data(), rx1, ry0);
    Float a = Star::lerp(sx, u, v);

    u = at2(m_g2[b01].data(), rx0, ry1);
    v = at2(m_g2[b11].data(), rx1, ry1);
    Float b = Star::lerp(sx, u, v);

    return Star::lerp(sy, a, b);
  }

  auto noise3(std::span<const Float, 3> vec) const -> Float {
    int bx0, bx1, by0, by1, bz0, bz1;
    Float rx0, rx1, ry0, ry1, rz0, rz1;

    setup(vec[0], bx0, bx1, rx0, rx1);
    setup(vec[1], by0, by1, ry0, ry1);
    setup(vec[2], bz0, bz1, rz0, rz1);

    int i = m_p[bx0];
    int j = m_p[bx1];

    int b00 = m_p[i + by0];
    int b10 = m_p[j + by0];
    int b01 = m_p[i + by1];
    int b11 = m_p[j + by1];

    Float sx = s_curve(rx0);
    Float sy = s_curve(ry0);
    Float sz = s_curve(rz0);

    Float u = at3(m_g3[b00 + bz0].data(), rx0, ry0, rz0);
    Float v = at3(m_g3[b10 + bz0].data(), rx1, ry0, rz0);
    Float a = Star::lerp(sx, u, v);

    u = at3(m_g3[b01 + bz0].data(), rx0, ry1, rz0);
    Float b = Star::lerp(sx, u, at3(m_g3[b11 + bz0].data(), rx1, ry1, rz0));

    Float c = Star::lerp(sy, a, b);

    u = at3(m_g3[b00 + bz1].data(), rx0, ry0, rz1);
    v = at3(m_g3[b10 + bz1].data(), rx1, ry0, rz1);
    a = Star::lerp(sx, u, v);

    u = at3(m_g3[b01 + bz1].data(), rx0, ry1, rz1);
    Float d = Star::lerp(sy, a, Star::lerp(sx, u, at3(m_g3[b11 + bz1].data(), rx1, ry1, rz1)));

    return Star::lerp(sz, c, d);
  }

  static void normalize2(std::array<Float, 2>& v) noexcept {
    Float s = std::hypot(v[0], v[1]);
    if (s == 0.0f) {
      v = {1.0f, 0.0f};
    } else {
      v[0] /= s;
      v[1] /= s;
    }
  }

  static void normalize3(std::array<Float, 3>& v) noexcept {
    Float s = std::hypot(v[0], v[1], v[2]);
    if (s == 0.0f) {
      v = {1.0f, 0.0f, 0.0f};
    } else {
      v[0] /= s;
      v[1] /= s;
      v[2] /= s;
    }
  }

  void init(std::uint64_t seed) {
    RandomSource randomSource(seed);

    constexpr int size = PerlinSampleSize * 2 + 2;
    m_p.resize(size);
    m_g3.resize(size);
    m_g2.resize(size);
    m_g1.resize(size);

    for (int i = 0; i < PerlinSampleSize; i++) {
      m_p[i] = i;
      m_g1[i] = static_cast<Float>(randomSource.randInt(-PerlinSampleSize, PerlinSampleSize)) / PerlinSampleSize;

      for (int j = 0; j < 2; j++)
        m_g2[i][j] = static_cast<Float>(randomSource.randInt(-PerlinSampleSize, PerlinSampleSize)) / PerlinSampleSize;
      normalize2(m_g2[i]);

      for (int j = 0; j < 3; j++)
        m_g3[i][j] = static_cast<Float>(randomSource.randInt(-PerlinSampleSize, PerlinSampleSize)) / PerlinSampleSize;
      normalize3(m_g3[i]);
    }

    int i = PerlinSampleSize;
    while (--i) {
      int j = static_cast<int>(randomSource.randUInt(PerlinSampleSize - 1));
      std::swap(m_p[i], m_p[j]);
    }

    for (int i = 0; i < PerlinSampleSize + 2; i++) {
      m_p[PerlinSampleSize + i] = m_p[i];
      m_g1[PerlinSampleSize + i] = m_g1[i];
      m_g2[PerlinSampleSize + i] = m_g2[i];
      m_g3[PerlinSampleSize + i] = m_g3[i];
    }
  }

  auto perlin(Float x) const -> Float {
    Float sum = 0, scale = 1;
    Float p = x * m_frequency;
    for (int i = 0; i < m_octaves; i++) {
      sum += noise1(p) / scale;
      scale *= m_alpha;
      p *= m_beta;
    }
    return sum * m_amplitude + m_bias;
  }

  auto perlin(Float x, Float y) const -> Float {
    Float sum = 0, scale = 1;
    std::array<Float, 2> p{x * m_frequency, y * m_frequency};
    for (int i = 0; i < m_octaves; i++) {
      sum += noise2(p) / scale;
      scale *= m_alpha;
      p[0] *= m_beta;
      p[1] *= m_beta;
    }
    return sum * m_amplitude + m_bias;
  }

  auto perlin(Float x, Float y, Float z) const -> Float {
    Float sum = 0, scale = 1;
    std::array<Float, 3> p{x * m_frequency, y * m_frequency, z * m_frequency};
    for (int i = 0; i < m_octaves; i++) {
      sum += noise3(p) / scale;
      scale *= m_alpha;
      p[0] *= m_beta;
      p[1] *= m_beta;
      p[2] *= m_beta;
    }
    return sum * m_amplitude + m_bias;
  }

  auto ridgedMulti(Float x) const -> Float {
    Float sum = 0, scale = 1, weight = 1.0;
    x *= m_frequency;
    for (int i = 0; i < m_octaves; ++i) {
      Float val = m_offset - std::abs(noise1(x));
      val *= val * weight;
      weight = std::clamp<Float>(val * m_gain, 0.0, 1.0);
      sum += val / scale;
      scale *= m_alpha;
      x *= m_beta;
    }
    return ((sum * 1.25) - 1.0) * m_amplitude + m_bias;
  }

  auto ridgedMulti(Float x, Float y) const -> Float {
    Float sum = 0, scale = 1, weight = 1.0;
    std::array<Float, 2> p{x * m_frequency, y * m_frequency};
    for (int i = 0; i < m_octaves; ++i) {
      Float val = m_offset - std::abs(noise2(p));
      val *= val * weight;
      weight = std::clamp<Float>(val * m_gain, 0.0, 1.0);
      sum += val / scale;
      scale *= m_alpha;
      p[0] *= m_beta;
      p[1] *= m_beta;
    }
    return ((sum * 1.25) - 1.0) * m_amplitude + m_bias;
  }

  auto ridgedMulti(Float x, Float y, Float z) const -> Float {
    Float sum = 0, scale = 1, weight = 1.0;
    std::array<Float, 3> p{x * m_frequency, y * m_frequency, z * m_frequency};
    for (int i = 0; i < m_octaves; ++i) {
      Float val = m_offset - std::abs(noise3(p));
      val *= val * weight;
      weight = std::clamp<Float>(val * m_gain, 0.0, 1.0);
      sum += val / scale;
      scale *= m_alpha;
      p[0] *= m_beta;
      p[1] *= m_beta;
      p[2] *= m_beta;
    }
    return ((sum * 1.25) - 1.0) * m_amplitude + m_bias;
  }

  auto billow(Float x) const -> Float {
    Float sum = 0, scale = 1;
    Float p = x * m_frequency;
    for (int i = 0; i < m_octaves; i++) {
      Float val = 2.0 * std::abs(noise1(p)) - 1.0;
      sum += val / scale;
      scale *= m_alpha;
      p *= m_beta;
    }
    return (sum + 0.5) * m_amplitude + m_bias;
  }

  auto billow(Float x, Float y) const -> Float {
    Float sum = 0, scale = 1;
    std::array<Float, 2> p{x * m_frequency, y * m_frequency};
    for (int i = 0; i < m_octaves; i++) {
      Float val = 2.0 * std::abs(noise2(p)) - 1.0;
      sum += val / scale;
      scale *= m_alpha;
      p[0] *= m_beta;
      p[1] *= m_beta;
    }
    return (sum + 0.5) * m_amplitude + m_bias;
  }

  auto billow(Float x, Float y, Float z) const -> Float {
    Float sum = 0, scale = 1;
    std::array<Float, 3> p{x * m_frequency, y * m_frequency, z * m_frequency};
    for (int i = 0; i < m_octaves; i++) {
      Float val = 2.0 * std::abs(noise3(p)) - 1.0;
      sum += val / scale;
      scale *= m_alpha;
      p[0] *= m_beta;
      p[1] *= m_beta;
      p[2] *= m_beta;
    }
    return (sum + 0.5) * m_amplitude + m_bias;
  }

  PerlinType m_type = PerlinType::Uninitialized;
  std::uint64_t m_seed = 0;

  int m_octaves = 1;
  Float m_frequency = 1.0;
  Float m_amplitude = 1.0;
  Float m_bias = 0.0;
  Float m_alpha = 2.0;
  Float m_beta = 0.5;

  Float m_offset = 1.0;
  Float m_gain = 2.0;

  std::vector<int> m_p;
  std::vector<std::array<Float, 3>> m_g3;
  std::vector<std::array<Float, 2>> m_g2;
  std::vector<Float> m_g1;
};

using PerlinF = Perlin<float>;
using PerlinD = Perlin<double>;

}// namespace Star
