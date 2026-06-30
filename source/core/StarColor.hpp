#pragma once

#include "StarStringView.hpp"
#include "StarVector.hpp"
#include "StarFormat.hpp"

namespace Star {

struct ColorExceptionTag { static constexpr char const* typeName = "ColorException"; };
using ColorException = TypedException<StarException, ColorExceptionTag>;

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
  [[nodiscard]] static Vec3F v3bToFloat(Vec3B const& b);
  [[nodiscard]] static Vec3B v3fToByte(Vec3F const& f, bool doClamp = true);
  [[nodiscard]] static Vec4F v4bToFloat(Vec4B const& b);
  [[nodiscard]] static Vec4B v4fToByte(Vec4F const& f, bool doClamp = true);

  [[nodiscard]] static Color rgbf(float r, float g, float b);
  [[nodiscard]] static Color rgbaf(float r, float g, float b, float a);
  [[nodiscard]] static Color rgbf(Vec3F const& c);
  [[nodiscard]] static Color rgbaf(Vec4F const& c);

  [[nodiscard]] static Color rgb(uint8_t r, uint8_t g, uint8_t b);
  [[nodiscard]] static Color rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
  [[nodiscard]] static Color rgb(Vec3B const& c);
  [[nodiscard]] static Color rgba(Vec4B const& c);

  [[nodiscard]] static Color hsv(float h, float s, float b);
  [[nodiscard]] static Color hsva(float h, float s, float b, float a);
  [[nodiscard]] static Color hsv(Vec3F const& c);
  [[nodiscard]] static Color hsva(Vec4F const& c);

  [[nodiscard]] static Color grayf(float g);
  [[nodiscard]] static Color gray(uint8_t g);

  // Only supports 8 bit color
  [[nodiscard]] static Color fromHex(StringView s);

  // #AARRGGBB
  [[nodiscard]] static Color fromUint32(uint32_t v);

  // Color from temperature in Kelvin
  [[nodiscard]] static Color temperature(float temp);

  [[nodiscard]] static Vec4B hueShiftVec4B(Vec4B color, float hue);
  [[nodiscard]] static Vec4B hexToVec4B(StringView s);
  // Black
  Color() = default;

  explicit Color(StringView name);

  [[nodiscard]] uint8_t red() const;
  [[nodiscard]] uint8_t green() const;
  [[nodiscard]] uint8_t blue() const;
  [[nodiscard]] uint8_t alpha() const;

  void setRed(uint8_t r);
  void setGreen(uint8_t g);
  void setBlue(uint8_t b);
  void setAlpha(uint8_t a);

  [[nodiscard]] float redF() const;
  [[nodiscard]] float greenF() const;
  [[nodiscard]] float blueF() const;
  [[nodiscard]] float alphaF() const;

  void setRedF(float r);
  void setGreenF(float b);
  void setBlueF(float g);
  void setAlphaF(float a);

  [[nodiscard]] bool isClear() const;

  // Returns a 4 byte value equal to #AARRGGBB
  [[nodiscard]] uint32_t toUint32() const;

  [[nodiscard]] Vec4B toRgba() const;
  [[nodiscard]] Vec3B toRgb() const;
  [[nodiscard]] Vec4F toRgbaF() const;
  [[nodiscard]] Vec3F toRgbF() const;

  [[nodiscard]] Vec4F const& data() const;

  [[nodiscard]] Vec4F toHsva() const;

  [[nodiscard]] float hue() const;
  [[nodiscard]] float saturation() const;
  [[nodiscard]] float value() const;

  void setHue(float hue);
  void setSaturation(float saturation);
  void setValue(float value);

  // Shift the current hue by the given value, with hue wrapping.
  void hueShift(float hue);

  // Reduce the color toward black by the given amount, from 0.0 to 1.0.
  void fade(float value);

  [[nodiscard]] String toHex() const;

  void convertToLinear();
  void convertToSRGB();

  [[nodiscard]] Color toLinear();
  [[nodiscard]] Color toSRGB();

  [[nodiscard]] Color contrasting();
  [[nodiscard]] Color complementary();

  // Mix two colors, giving the second color the given amount
  [[nodiscard]] Color mix(Color const& c, float amount = 0.5f) const;
  [[nodiscard]] Color multiply(float amount) const;

  [[nodiscard]] bool operator==(Color const& c) const;
  [[nodiscard]] bool operator!=(Color const& c) const;
  [[nodiscard]] Color operator+(Color const& c) const;
  [[nodiscard]] Color operator*(Color const& c) const;
  Color& operator+=(Color const& c);
  Color& operator*=(Color const& c);

  [[nodiscard]] static float toLinear(float in);
  [[nodiscard]] static float fromLinear(float in);
private:
  Vec4F m_data;
};

std::ostream& operator<<(std::ostream& os, Color const& c);

[[nodiscard]] inline Vec3F Color::v3bToFloat(Vec3B const& b) {
  return Vec3F(byteToFloat(b[0]), byteToFloat(b[1]), byteToFloat(b[2]));
}

[[nodiscard]] inline Vec3B Color::v3fToByte(Vec3F const& f, bool doClamp) {
  return Vec3B(floatToByte(f[0], doClamp), floatToByte(f[1], doClamp), floatToByte(f[2], doClamp));
}

[[nodiscard]] inline Vec4F Color::v4bToFloat(Vec4B const& b) {
  return Vec4F(byteToFloat(b[0]), byteToFloat(b[1]), byteToFloat(b[2]), byteToFloat(b[3]));
}

[[nodiscard]] inline Vec4B Color::v4fToByte(Vec4F const& f, bool doClamp) {
  return Vec4B(floatToByte(f[0], doClamp), floatToByte(f[1], doClamp), floatToByte(f[2], doClamp), floatToByte(f[3], doClamp));
}

}

template <> struct std::formatter<Star::Color> : Star::OstreamFormatter {};
