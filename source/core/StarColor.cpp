#include "StarColor.hpp"

#include "StarEncode.hpp"
#include "StarFormat.hpp"
#include "StarInterpolation.hpp"

import std;

namespace Star {

Color const Color::Red = Color::rgba(255, 73, 66, 255);
Color const Color::Orange = Color::rgba(255, 180, 47, 255);
Color const Color::Yellow = Color::rgba(255, 239, 30, 255);
Color const Color::Green = Color::rgba(79, 230, 70, 255);
Color const Color::Blue = Color::rgba(38, 96, 255, 255);
Color const Color::Indigo = Color::rgba(75, 0, 130, 255);
Color const Color::Violet = Color::rgba(160, 119, 255, 255);
Color const Color::Black = Color::rgba(0, 0, 0, 255);
Color const Color::White = Color::rgba(255, 255, 255, 255);
Color const Color::Magenta = Color::rgba(221, 92, 249, 255);
Color const Color::DarkMagenta = Color::rgba(142, 33, 144, 255);
Color const Color::Cyan = Color::rgba(0, 220, 233, 255);
Color const Color::DarkCyan = Color::rgba(0, 137, 165, 255);
Color const Color::CornFlowerBlue = Color::rgba(100, 149, 237, 255);
Color const Color::Gray = Color::rgba(160, 160, 160, 255);
Color const Color::LightGray = Color::rgba(192, 192, 192, 255);
Color const Color::DarkGray = Color::rgba(128, 128, 128, 255);
Color const Color::DarkGreen = Color::rgba(0, 128, 0, 255);
Color const Color::Pink = Color::rgba(255, 162, 187, 255);
Color const Color::Clear = Color::rgba(0, 0, 0, 0);

CaseInsensitiveStringMap<Color> const Color::NamedColors{
  {"red", Color::Red},
  {"orange", Color::Orange},
  {"yellow", Color::Yellow},
  {"green", Color::Green},
  {"blue", Color::Blue},
  {"indigo", Color::Indigo},
  {"violet", Color::Violet},
  {"black", Color::Black},
  {"white", Color::White},
  {"magenta", Color::Magenta},
  {"darkmagenta", Color::DarkMagenta},
  {"cyan", Color::Cyan},
  {"darkcyan", Color::DarkCyan},
  {"cornflowerblue", Color::CornFlowerBlue},
  {"gray", Color::Gray},
  {"lightgray", Color::LightGray},
  {"darkgray", Color::DarkGray},
  {"darkgreen", Color::DarkGreen},
  {"pink", Color::Pink},
  {"clear", Color::Clear}
};

auto Color::rgbf(const Vec3F& c) -> Color {
  return rgbaf(c[0], c[1], c[2], 1.0f);
}

auto Color::rgbaf(const Vec4F& c) -> Color {
  return rgbaf(c[0], c[1], c[2], c[3]);
}

auto Color::rgbf(float r, float g, float b) -> Color {
  return rgbaf(r, g, b, 1.0f);
}

auto Color::rgbaf(float r, float g, float b, float a) -> Color {
  Color c;
  c.m_data[0] = clamp(r, 0.0f, 1.0f);
  c.m_data[1] = clamp(g, 0.0f, 1.0f);
  c.m_data[2] = clamp(b, 0.0f, 1.0f);
  c.m_data[3] = clamp(a, 0.0f, 1.0f);
  return c;
}

auto Color::rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b) -> Color {
  return rgba(r, g, b, 255);
}

auto Color::rgba(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a) -> Color {
  Color c;
  c.m_data[0] = r / 255.0f;
  c.m_data[1] = g / 255.0f;
  c.m_data[2] = b / 255.0f;
  c.m_data[3] = a / 255.0f;
  return c;
}

auto Color::fromUint32(std::uint32_t v) -> Color {
  Color c;
  c.setAlpha(((std::uint8_t*)(&v))[3]);
  c.setRed(((std::uint8_t*)(&v))[2]);
  c.setGreen(((std::uint8_t*)(&v))[1]);
  c.setBlue(((std::uint8_t*)(&v))[0]);
  return c;
}

auto Color::temperature(float temp) -> Color {
  // Magic numbers ahoy!
  Color c;
  c.setAlpha(255);

  temp = clamp<float>(temp, 1000, 40000);

  temp /= 100;

  double r, g, b;
  if (temp <= 66) {
    r = 255;
    g = clamp<double>(99.4708025861 * std::log(temp) - 161.1195681661, 0, 255);
    if (temp <= 19) {
      b = 0;
    } else {
      b = clamp<double>(138.5177312231 * std::log(temp - 10) - 305.0447927307, 0, 255);
    }
  } else {
    r = clamp<double>(329.698727446 * std::pow(temp - 60, -0.1332047592), 0, 255);
    g = clamp<double>(288.1221695283 * std::pow(temp - 60, -0.0755148492), 0, 255);
    b = 255;
  }

  c.setRedF((float)r / 255.0f);
  c.setGreenF((float)g / 255.0f);
  c.setBlueF((float)b / 255.0f);

  return c;
}

auto Color::rgb(Vec3B const& c) -> Color {
  return rgb(c[0], c[1], c[2]);
}

auto Color::rgba(Vec4B const& c) -> Color {
  return rgba(c[0], c[1], c[2], c[3]);
}

auto Color::hsv(float h, float s, float v) -> Color {
  return hsva(h, s, v, 1.0f);
}

auto Color::hsva(float h, float s, float v, float a) -> Color {
  h = clamp(h, 0.0f, 1.0f);
  s = clamp(s, 0.0f, 1.0f);
  v = clamp(v, 0.0f, 1.0f);
  a = clamp(a, 0.0f, 1.0f);

  Color retColor;
  if (s == 0.0f) {
    retColor.setRedF(v);
    retColor.setGreenF(v);
    retColor.setBlueF(v);
    retColor.setAlphaF(a);
  } else {
    float var_h, var_i, var_1, var_2, var_3, var_r, var_g, var_b;

    var_h = h * 6.0f;
    if (var_h == 6.0f)
      var_h = 0.0f; // H must be < 1

    var_i = std::floor(var_h);

    var_1 = v * (1.0f - s);
    var_2 = v * (1.0f - s * (var_h - var_i));
    var_3 = v * (1.0f - s * (1.0f - (var_h - var_i)));

    if (var_i == 0) {
      var_r = v;
      var_g = var_3;
      var_b = var_1;
    } else if (var_i == 1) {
      var_r = var_2;
      var_g = v;
      var_b = var_1;
    } else if (var_i == 2) {
      var_r = var_1;
      var_g = v;
      var_b = var_3;
    } else if (var_i == 3) {
      var_r = var_1;
      var_g = var_2;
      var_b = v;
    } else if (var_i == 4) {
      var_r = var_3;
      var_g = var_1;
      var_b = v;
    } else {
      var_r = v;
      var_g = var_1;
      var_b = var_2;
    }

    retColor.setRedF(var_r);
    retColor.setGreenF(var_g);
    retColor.setBlueF(var_b);
    retColor.setAlphaF(a);
  }
  return retColor;
}

auto Color::hsv(Vec3F const& c) -> Color {
  return Color::hsv(c[0], c[1], c[2]);
}

auto Color::hsva(Vec4F const& c) -> Color {
  return Color::hsva(c[0], c[1], c[2], c[3]);
}

auto Color::grayf(float g) -> Color {
  return Color::rgbf(g, g, g);
}

auto Color::gray(std::uint8_t g) -> Color {
  return Color::rgb(g, g, g);
}

Color::Color(StringView name) {
  if (name.utf8().starts_with('#'))
    *this = fromHex(name.utf8().substr(1));
  else {
    auto i = NamedColors.find(String(name));
    if (i != NamedColors.end())
      *this = i->second;
    else
      throw ColorException(strf("Named color {} not found", name), false);
  }
}

auto Color::redF() const -> float {
  return m_data[0];
}

auto Color::greenF() const -> float {
  return m_data[1];
}

auto Color::blueF() const -> float {
  return m_data[2];
}

auto Color::alphaF() const -> float {
  return m_data[3];
}

auto Color::isClear() const -> bool {
  return m_data[3] == 0;
}

auto Color::red() const -> std::uint8_t {
  return std::uint8_t(std::round(m_data[0] * 255));
}

auto Color::green() const -> std::uint8_t {
  return std::uint8_t(std::round(m_data[1] * 255));
}

auto Color::blue() const -> std::uint8_t {
  return std::uint8_t(std::round(m_data[2] * 255));
}

auto Color::alpha() const -> std::uint8_t {
  return std::uint8_t(m_data[3] * 255);
}

void Color::setRedF(float r) {
  m_data[0] = clamp(r, 0.0f, 1.0f);
}

void Color::setGreenF(float g) {
  m_data[1] = clamp(g, 0.0f, 1.0f);
}

void Color::setBlueF(float b) {
  m_data[2] = clamp(b, 0.0f, 1.0f);
}

void Color::setAlphaF(float a) {
  m_data[3] = clamp(a, 0.0f, 1.0f);
}

void Color::setRed(std::uint8_t r) {
  m_data[0] = r / 255.0f;
}

void Color::setGreen(std::uint8_t g) {
  m_data[1] = g / 255.0f;
}

void Color::setBlue(std::uint8_t b) {
  m_data[2] = b / 255.0f;
}

void Color::setAlpha(std::uint8_t a) {
  m_data[3] = a / 255.0f;
}

auto Color::toUint32() const -> std::uint32_t {
  std::uint32_t val;
  ((std::uint8_t*)(&val))[3] = alpha();
  ((std::uint8_t*)(&val))[2] = red();
  ((std::uint8_t*)(&val))[1] = green();
  ((std::uint8_t*)(&val))[0] = blue();
  return val;
}

auto Color::fromHex(StringView s) -> Color {
  return Color::rgba(hexToVec4B(s));
}

auto Color::toRgba() const -> Vec4B {
  return {red(), green(), blue(), alpha()};
}

auto Color::toRgb() const -> Vec3B {
  return {red(), green(), blue()};
}

auto Color::toRgbaF() const -> Vec4F {
  return {redF(), greenF(), blueF(), alphaF()};
}

auto Color::toRgbF() const -> Vec3F {
  return {redF(), greenF(), blueF()};
}

auto Color::toHsva() const -> Vec4F {
  float h, s, v;

  float var_r = redF();
  float var_g = greenF();
  float var_b = blueF();

  // Min. value of RGB
  float var_min = std::min({var_r, var_g, var_b});

  // Max. value of RGB
  float var_max = std::max({var_r, var_g, var_b});

  // Delta RGB value
  float del_max = var_max - var_min;

  v = var_max;

  if (del_max == 0.0f) { // This is a gray, no chroma...
    h = 0.0f;
    s = 0.0f;
  } else { // Chromatic data
    s = del_max / var_max;

    float del_r = (((var_max - var_r) / 6.0f) + (del_max / 2.0f)) / del_max;
    float del_g = (((var_max - var_g) / 6.0f) + (del_max / 2.0f)) / del_max;
    float del_b = (((var_max - var_b) / 6.0f) + (del_max / 2.0f)) / del_max;

    if (var_r == var_max)
      h = del_b - del_g;
    else if (var_g == var_max)
      h = (1.0f / 3.0f) + del_r - del_b;
    else
      /*if (var_b == var_max)*/ h = (2.0f / 3.0f) + del_g - del_r;

    if (h < 0.0f)
      h += 1.0f;
    if (h >= 1.0f)
      h -= 1.0f;
  }

  return {h, s, v, alphaF()};
}

auto Color::hue() const -> float {
  return toHsva()[0];
}

auto Color::saturation() const -> float {
  // Min. value of RGB
  float var_min = std::min({m_data[0], m_data[1], m_data[2]});

  // Max. value of RGB
  float var_max = std::max({m_data[0], m_data[1], m_data[2]});

  // Delta RGB value
  float del_max = var_max - var_min;

  if (del_max == 0.0f) { // This is a gray, no chroma...
    return 0.0f;
  } else
    return del_max / var_max;
}

auto Color::value() const -> float {
  return std::max({m_data[0], m_data[1], m_data[2]});
}

void Color::setHue(float h) {
  auto hsva = toHsva();
  *this = Color::hsva(clamp(h, 0.0f, 1.0f), hsva[1], hsva[2], alphaF());
}

void Color::setSaturation(float s) {
  auto hsva = toHsva();
  *this = Color::hsva(hsva[0], clamp(s, 0.0f, 1.0f), hsva[2], alphaF());
}

void Color::setValue(float v) {
  auto hsva = toHsva();
  *this = Color::hsva(hsva[0], hsva[1], clamp(v, 0.0f, 1.0f), alphaF());
}

void Color::hueShift(float h) {
  setHue(pfmod(hue() + h, 1.0f));
}

void Color::fade(float value) {
  m_data *= (1.0f - value);
  m_data.clamp(0.0f, 1.0f);
}

auto Color::operator==(const Color& c) const -> bool {
  return m_data == c.m_data;
}

auto Color::operator!=(const Color& c) const -> bool {
  return m_data != c.m_data;
}

auto operator<<(std::ostream& os, const Color& c) -> std::ostream& {
  os << c.toRgbaF();
  return os;
}

auto Color::toLinear(float in) -> float {
  const float a = 0.055f;
  if (in <= 0.04045f)
    return in / 12.92f;
  return std::powf((in + a) / (1.0f + a), 2.4f);
}

auto Color::fromLinear(float in) -> float {
  const float a = 0.055f;
  if (in <= 0.0031308f)
    return 12.92f * in;
  return (1.0f + a) * std::powf(in, 1.0f / 2.4f) - a;
}

auto Color::toHex() const -> String {
  auto rgba = toRgba();
  return hexEncode((char*)rgba.ptr(), rgba[3] == 255 ? 3 : 4);
}

void Color::convertToLinear() {
  setRedF(toLinear(redF()));
  setGreenF(toLinear(greenF()));
  setBlueF(toLinear(blueF()));
}

void Color::convertToSRGB() {
  setRedF(fromLinear(redF()));
  setGreenF(fromLinear(greenF()));
  setBlueF(fromLinear(blueF()));
}

auto Color::toLinear() -> Color {
  Color c = *this;
  c.convertToLinear();
  return c;
}

auto Color::toSRGB() -> Color {
  Color c = *this;
  c.convertToSRGB();
  return c;
}

auto Color::contrasting() -> Color {
  Color c = *this;
  c.setHue(c.hue() + 120);
  return c;
}

auto Color::complementary() -> Color {
  Color c = *this;
  c.setHue(c.hue() + 180);
  return c;
}

auto Color::mix(Color const& c, float amount) const -> Color {
  return Color::rgbaf(lerp(clamp(amount, 0.0f, 1.0f), toRgbaF(), c.toRgbaF()));
}

auto Color::multiply(float amount) const -> Color {
  return Color::rgbaf(m_data * amount);
}

auto Color::operator+(Color const& c) const -> Color {
  return Color::rgbaf(m_data + c.toRgbaF());
}

auto Color::operator*(Color const& c) const -> Color {
  return Color::rgbaf(m_data.piecewiseMultiply(c.toRgbaF()));
}

auto Color::operator+=(Color const& c) -> Color& {
  return * this = *this + c;
}

auto Color::operator*=(Color const& c) -> Color& {
  return * this = *this * c;
}

auto Color::hueShiftVec4B(Vec4B color, float hue) -> Vec4B {
  float h, s, v;

  float var_r = color[0] / 255.0f;
  float var_g = color[1] / 255.0f;
  float var_b = color[2] / 255.0f;

  // Min. value of RGB
  float var_min = std::min({var_r, var_g, var_b});

  // Max. value of RGB
  float var_max = std::max({var_r, var_g, var_b});

  // Delta RGB value
  float del_max = var_max - var_min;

  v = var_max;

  if (del_max == 0.0f) { // This is a gray, no chroma...
    h = 0.0f;
    s = 0.0f;
  } else { // Chromatic data
    s = del_max / var_max;

    float vd = 1.0f / 6.0f;
    float dmh = del_max * 0.5f;
    float dmi = 1.0f / del_max;
    float del_r = (((var_max - var_r) * vd) + dmh) * dmi;
    float del_g = (((var_max - var_g) * vd) + dmh) * dmi;
    float del_b = (((var_max - var_b) * vd) + dmh) * dmi;

    if (var_r == var_max)
      h = del_b - del_g;
    else if (var_g == var_max)
      h = (1.0f / 3.0f) + del_r - del_b;
    else
      h = (2.0f / 3.0f) + del_g - del_r;

    if (h < 0.0f)
      h += 1.0f;
    if (h >= 1.0f)
      h -= 1.0f;
  }

  h += hue;

  if (h >= 1.0f)
    h -= 1.0f;

  if (s == 0.0f) {
    auto c = std::uint8_t(std::round(v * 255));
    return {c, c, c, color[3]};
  } else {
    float var_h, var_i, var_1, var_2, var_3, var_r, var_g, var_b;

    var_h = h * 6.0f;
    if (var_h == 6.0f)
      var_h = 0.0f; // H must be < 1

    var_i = std::floor(var_h);

    var_1 = v * (1.0f - s);
    var_2 = v * (1.0f - s * (var_h - var_i));
    var_3 = v * (1.0f - s * (1.0f - (var_h - var_i)));

    if (var_i == 0) {
      var_r = v;
      var_g = var_3;
      var_b = var_1;
    } else if (var_i == 1) {
      var_r = var_2;
      var_g = v;
      var_b = var_1;
    } else if (var_i == 2) {
      var_r = var_1;
      var_g = v;
      var_b = var_3;
    } else if (var_i == 3) {
      var_r = var_1;
      var_g = var_2;
      var_b = v;
    } else if (var_i == 4) {
      var_r = var_3;
      var_g = var_1;
      var_b = v;
    } else {
      var_r = v;
      var_g = var_1;
      var_b = var_2;
    }

    return {std::uint8_t(std::round(var_r * 255)), std::uint8_t(std::round(var_g * 255)), std::uint8_t(std::round(var_b * 255)), color[3]};
  }
}

auto Color::hexToVec4B(StringView s) -> Vec4B {
  Array<std::uint8_t, 4> cbytes;

  if (s.utf8Size() == 3) {
    nibbleDecode(s.utf8Ptr(), 3, (char*)cbytes.data(), 4);
    cbytes[0] = (cbytes[0] << 4) | cbytes[0];
    cbytes[1] = (cbytes[1] << 4) | cbytes[1];
    cbytes[2] = (cbytes[2] << 4) | cbytes[2];
    cbytes[3] = 255;
  } else if (s.utf8Size() == 4) {
    nibbleDecode(s.utf8Ptr(), 4, (char*)cbytes.data(), 4);
    cbytes[0] = (cbytes[0] << 4) | cbytes[0];
    cbytes[1] = (cbytes[1] << 4) | cbytes[1];
    cbytes[2] = (cbytes[2] << 4) | cbytes[2];
    cbytes[3] = (cbytes[3] << 4) | cbytes[3];
  } else if (s.utf8Size() == 6) {
    hexDecode(s.utf8Ptr(), 6, (char*)cbytes.data(), 4);
    cbytes[3] = 255;
  } else if (s.utf8Size() == 8) {
    hexDecode(s.utf8Ptr(), 8, (char*)cbytes.data(), 4);
  } else {
    throw ColorException(strf("Improper size {} for hex string '{}' in Color::hexToVec4B", s.utf8Size(), s), false);
  }

  return Vec4B(std::move(cbytes));
}

}
