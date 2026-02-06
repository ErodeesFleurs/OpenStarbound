#pragma once

#include "StarException.hpp"
#include "StarStringView.hpp"
#include "StarVector.hpp"

import std;

namespace Star {

using ColorException = ExceptionDerived<"ColorException">;

class Color {
public:
  static Color const Red;
  static Color const Orange;
  static Color const Yellow;
  static Color const Green;
  static Color const Blue;
  static Color const Indigo;
  static Color const Violet;
  static Color const Black;
  static Color const White;
  static Color const Magenta;
  static Color const DarkMagenta;
  static Color const Cyan;
  static Color const DarkCyan;
  static Color const CornFlowerBlue;
  static Color const Gray;
  static Color const LightGray;
  static Color const DarkGray;
  static Color const DarkGreen;
  static Color const Pink;
  static Color const Clear;

  static CaseInsensitiveStringMap<Color> const NamedColors;

  // Some useful conversion methods for dealing with Vec3 / Vec4 as colors
  static auto v3bToFloat(Vec3B const& b) -> Vec3F;
  static auto v3fToByte(Vec3F const& f, bool doClamp = true) -> Vec3B;
  static auto v4bToFloat(Vec4B const& b) -> Vec4F;
  static auto v4fToByte(Vec4F const& f, bool doClamp = true) -> Vec4B;

  static auto rgbf(float r, float g, float b) -> Color;
  static auto rgbaf(float r, float g, float b, float a) -> Color;
  static auto rgbf(Vec3F const& c) -> Color;
  static auto rgbaf(Vec4F const& c) -> Color;

  static auto rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b) -> Color;
  static auto rgba(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a) -> Color;
  static auto rgb(Vec3B const& c) -> Color;
  static auto rgba(Vec4B const& c) -> Color;

  static auto hsv(float h, float s, float b) -> Color;
  static auto hsva(float h, float s, float b, float a) -> Color;
  static auto hsv(Vec3F const& c) -> Color;
  static auto hsva(Vec4F const& c) -> Color;

  static auto grayf(float g) -> Color;
  static auto gray(std::uint8_t g) -> Color;

  // Only supports 8 bit color
  static auto fromHex(StringView s) -> Color;

  // #AARRGGBB
  static auto fromUint32(std::uint32_t v) -> Color;

  // Color from temperature in Kelvin
  static auto temperature(float temp) -> Color;

  static auto hueShiftVec4B(Vec4B color, float hue) -> Vec4B;
  static auto hexToVec4B(StringView s) -> Vec4B;
  // Black
  Color() = default;

  explicit Color(StringView name);

  [[nodiscard]] auto red() const -> std::uint8_t;
  [[nodiscard]] auto green() const -> std::uint8_t;
  [[nodiscard]] auto blue() const -> std::uint8_t;
  [[nodiscard]] auto alpha() const -> std::uint8_t;

  void setRed(std::uint8_t r);
  void setGreen(std::uint8_t g);
  void setBlue(std::uint8_t b);
  void setAlpha(std::uint8_t a);

  [[nodiscard]] auto redF() const -> float;
  [[nodiscard]] auto greenF() const -> float;
  [[nodiscard]] auto blueF() const -> float;
  [[nodiscard]] auto alphaF() const -> float;

  void setRedF(float r);
  void setGreenF(float b);
  void setBlueF(float g);
  void setAlphaF(float a);

  [[nodiscard]] auto isClear() const -> bool;

  // Returns a 4 byte value equal to #AARRGGBB
  [[nodiscard]] auto toUint32() const -> std::uint32_t;

  [[nodiscard]] auto toRgba() const -> Vec4B;
  [[nodiscard]] auto toRgb() const -> Vec3B;
  [[nodiscard]] auto toRgbaF() const -> Vec4F;
  [[nodiscard]] auto toRgbF() const -> Vec3F;

  [[nodiscard]] auto data() const -> Vec4F const&;

  [[nodiscard]] auto toHsva() const -> Vec4F;

  [[nodiscard]] auto hue() const -> float;
  [[nodiscard]] auto saturation() const -> float;
  [[nodiscard]] auto value() const -> float;

  void setHue(float hue);
  void setSaturation(float saturation);
  void setValue(float value);

  // Shift the current hue by the given value, with hue wrapping.
  void hueShift(float hue);

  // Reduce the color toward black by the given amount, from 0.0 to 1.0.
  void fade(float value);

  [[nodiscard]] auto toHex() const -> String;

  void convertToLinear();
  void convertToSRGB();

  auto toLinear() -> Color;
  auto toSRGB() -> Color;

  auto contrasting() -> Color;
  auto complementary() -> Color;

  // Mix two colors, giving the second color the given amount
  [[nodiscard]] auto mix(Color const& c, float amount = 0.5f) const -> Color;
  [[nodiscard]] auto multiply(float amount) const -> Color;

  auto operator==(Color const& c) const -> bool;
  auto operator!=(Color const& c) const -> bool;
  auto operator+(Color const& c) const -> Color;
  auto operator*(Color const& c) const -> Color;
  auto operator+=(Color const& c) -> Color&;
  auto operator*=(Color const& c) -> Color&;

  static auto toLinear(float in) -> float;
  static auto fromLinear(float in) -> float;
private:
  Vec4F m_data;
};

auto operator<<(std::ostream& os, Color const& c) -> std::ostream&;

inline auto Color::v3bToFloat(Vec3B const& b) -> Vec3F {
  return {byteToFloat(b[0]), byteToFloat(b[1]), byteToFloat(b[2])};
}

inline auto Color::v3fToByte(Vec3F const& f, bool doClamp) -> Vec3B {
  return {floatToByte(f[0], doClamp), floatToByte(f[1], doClamp), floatToByte(f[2], doClamp)};
}

inline auto Color::v4bToFloat(Vec4B const& b) -> Vec4F {
  return {byteToFloat(b[0]), byteToFloat(b[1]), byteToFloat(b[2]), byteToFloat(b[3])};
}

inline auto Color::v4fToByte(Vec4F const& f, bool doClamp) -> Vec4B {
  return {floatToByte(f[0], doClamp), floatToByte(f[1], doClamp), floatToByte(f[2], doClamp), floatToByte(f[3], doClamp)};
}

}

template <> struct std::formatter<Star::Color> : Star::ostream_formatter {};
