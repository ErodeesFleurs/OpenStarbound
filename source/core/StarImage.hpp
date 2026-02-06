#pragma once

#include "StarException.hpp"
#include "StarVector.hpp"
#include "StarIODevice.hpp"
#include "StarConfig.hpp"

import std;

namespace Star {

enum class PixelFormat : std::uint8_t {
  RGB24,
  RGBA32,
  BGR24,
  BGRA32,
  RGB_F,
  RGBA_F
};

auto bitsPerPixel(PixelFormat pf) -> std::uint8_t;
auto bytesPerPixel(PixelFormat pf) -> std::uint8_t;

using ImageException = ExceptionDerived<"ImageException">;

// Holds an image of PixelFormat in row major order, with no padding, with (0,
// 0) defined to be the *lower left* corner.
class Image {
public:
  static auto readPng(Ptr<IODevice> device) -> Image;
  static auto isPng(Ptr<IODevice> device) -> bool;
  // Returns the size and pixel format that would be constructed from the given
  // png file, without actually loading it.
  static auto readPngMetadata(Ptr<IODevice> device) -> std::tuple<Vec2U, PixelFormat>;

  static auto filled(Vec2U size, Vec4B color, PixelFormat pf = PixelFormat::RGBA32) -> Image;

  // Creates a zero size image
  Image(PixelFormat pf = PixelFormat::RGBA32);
  Image(Vec2U size, PixelFormat pf = PixelFormat::RGBA32);
  Image(unsigned width, unsigned height, PixelFormat pf = PixelFormat::RGBA32);
  ~Image();

  Image(Image const& image);
  Image(Image&& image);

  auto operator=(Image const& image) -> Image&;
  auto operator=(Image&& image) -> Image&;

  [[nodiscard]] auto bitsPerPixel() const -> std::uint8_t;
  [[nodiscard]] auto bytesPerPixel() const -> std::uint8_t;

  [[nodiscard]] auto width() const -> unsigned;
  [[nodiscard]] auto height() const -> unsigned;
  [[nodiscard]] auto size() const -> Vec2U;
  // width or height is 0
  [[nodiscard]] auto empty() const -> bool;

  [[nodiscard]] auto pixelFormat() const -> PixelFormat;

  // If the image is empty, the data ptr will be null
  [[nodiscard]] auto data() const -> std::uint8_t const*;
  auto data() -> std::uint8_t*;

  // Reallocate the image with the given width, height, and pixel format.  The
  // contents of the image are always zeroed after a call to reset.
  void reset(Vec2U size, std::optional<PixelFormat> pf = {});
  void reset(unsigned width, unsigned height, std::optional<PixelFormat> pf = {});

  // Fill the image with a given color
  void fill(Vec3B const& c);
  void fill(Vec4B const& c);

  // Fill a rectangle with a given color
  void fillRect(Vec2U const& pos, Vec2U const& size, Vec3B const& c);
  void fillRect(Vec2U const& pos, Vec2U const& size, Vec4B const& c);

  // Color parameters / return values here are in whatever the internal format
  // is.  Fourth byte, if missing or not provided, is assumed to be 255.  If
  // the position is out of range, then throws an exception.
  void set(Vec2U const& pos, Vec4B const& c);
  void set(Vec2U const& pos, Vec3B const& c);
  [[nodiscard]] auto get(Vec2U const& pos) const -> Vec4B;

  // Same as set / get, except color parameters / return values here are always
  // RGB[A], and converts if necessary.
  void setrgb(Vec2U const& pos, Vec4B const& c);
  void setrgb(Vec2U const& pos, Vec3B const& c);
  [[nodiscard]] auto getrgb(Vec2U const& pos) const -> Vec4B;

  // Get pixel value, but if pos is out of the normal pixel range, it is
  // clamped back into the valid pixel range.  Returns (0, 0, 0, 0) if image is
  // empty.
  [[nodiscard]] auto clamp(Vec2I const& pos) const -> Vec4B;
  [[nodiscard]] auto clamprgb(Vec2I const& pos) const -> Vec4B;

  // x / y versions of set / get, for compatibility
  void set(unsigned x, unsigned y, Vec4B const& c);
  void set(unsigned x, unsigned y, Vec3B const& c);
  [[nodiscard]] auto get(unsigned x, unsigned y) const -> Vec4B;
  void setrgb(unsigned x, unsigned y, Vec4B const& c);
  void setrgb(unsigned x, unsigned y, Vec3B const& c);
  [[nodiscard]] auto getrgb(unsigned x, unsigned y) const -> Vec4B;
  [[nodiscard]] auto clamp(int x, int y) const -> Vec4B;
  [[nodiscard]] auto clamprgb(int x, int y) const -> Vec4B;

  // Must be 32 bitsPerPixel, no format conversion or bounds checking takes
  // place.  Very fast inline versions.
  void set32(Vec2U const& pos, Vec4B const& c);
  void set32(unsigned x, unsigned y, Vec4B const& c);
  [[nodiscard]] auto get32(unsigned x, unsigned y) const -> Vec4B;

  // Must be 24 bitsPerPixel, no format conversion or bounds checking takes
  // place.  Very fast inline versions.
  void set24(Vec2U const& pos, Vec3B const& c);
  void set24(unsigned x, unsigned y, Vec3B const& c);
  [[nodiscard]] auto get24(unsigned x, unsigned y) const -> Vec3B;

  // Called as callback(unsigned x, unsigned y, Vec4B const& pixel)
  template <typename CallbackType>
  void forEachPixel(CallbackType&& callback) const;

  // Called as callback(unsigned x, unsigned y, Vec4B& pixel)
  template <typename CallbackType>
  void forEachPixel(CallbackType&& callback);

  // Pixel rectangle, lower left position and size of rectangle.
  [[nodiscard]] auto subImage(Vec2U const& pos, Vec2U const& size) const -> Image;

  // Copy given image into this one at pos
  void copyInto(Vec2U const& pos, Image const& image);
  // Draw given image over this one at pos (with alpha composition)
  void drawInto(Vec2U const& pos, Image const& image);

  // Convert this image into the given pixel format
  [[nodiscard]] auto convert(PixelFormat pixelFormat) const -> Image;

  void writePng(Ptr<IODevice> device) const;

private:
  std::uint8_t* m_data;
  unsigned m_width;
  unsigned m_height;
  PixelFormat m_pixelFormat;
};

inline auto bitsPerPixel(PixelFormat pf) -> std::uint8_t {
  switch (pf) {
    case PixelFormat::RGB24:
      return 24;
    case PixelFormat::RGBA32:
      return 32;
    case PixelFormat::BGR24:
      return 24;
    case PixelFormat::BGRA32:
      return 32;
    case PixelFormat::RGB_F:
      return 96;
    default:
      return 128;
  }
}

inline auto bytesPerPixel(PixelFormat pf) -> std::uint8_t {
  switch (pf) {
    case PixelFormat::RGB24:
      return 3;
    case PixelFormat::RGBA32:
      return 4;
    case PixelFormat::BGR24:
      return 3;
    case PixelFormat::BGRA32:
      return 4;
    case PixelFormat::RGB_F:
      return 12;
    default:
      return 16;
  }
}

inline auto Image::bitsPerPixel() const -> std::uint8_t {
  return Star::bitsPerPixel(m_pixelFormat);
}

inline auto Image::bytesPerPixel() const -> std::uint8_t {
  return Star::bytesPerPixel(m_pixelFormat);
}

inline auto Image::width() const -> unsigned {
  return m_width;
}

inline auto Image::height() const -> unsigned {
  return m_height;
}

inline auto Image::empty() const -> bool {
  return m_width == 0 || m_height == 0;
}

inline auto Image::size() const -> Vec2U {
  return {m_width, m_height};
}

inline auto Image::pixelFormat() const -> PixelFormat {
  return m_pixelFormat;
}

inline auto Image::data() const -> const std::uint8_t* {
  return m_data;
}

inline auto Image::data() -> std::uint8_t* {
  return m_data;
}

inline void Image::set(unsigned x, unsigned y, Vec4B const& c) {
  return set({x, y}, c);
}

inline void Image::set(unsigned x, unsigned y, Vec3B const& c) {
  return set({x, y}, c);
}

inline auto Image::get(unsigned x, unsigned y) const -> Vec4B {
  return get({x, y});
}

inline void Image::setrgb(unsigned x, unsigned y, Vec4B const& c) {
  return setrgb({x, y}, c);
}

inline void Image::setrgb(unsigned x, unsigned y, Vec3B const& c) {
  return setrgb({x, y}, c);
}

inline auto Image::getrgb(unsigned x, unsigned y) const -> Vec4B {
  return getrgb({x, y});
}

inline auto Image::clamp(int x, int y) const -> Vec4B {
  return clamp({x, y});
}

inline auto Image::clamprgb(int x, int y) const -> Vec4B {
  return clamprgb({x, y});
}

inline void Image::set32(Vec2U const& pos, Vec4B const& c) {
  set32(pos[0], pos[1], c);
}

inline void Image::set32(unsigned x, unsigned y, Vec4B const& c) {

  size_t offset = y * m_width * 4 + x * 4;
  m_data[offset] = c[0];
  m_data[offset + 1] = c[1];
  m_data[offset + 2] = c[2];
  m_data[offset + 3] = c[3];
}

inline auto Image::get32(unsigned x, unsigned y) const -> Vec4B {

  Vec4B c;
  size_t offset = y * m_width * 4 + x * 4;
  c[0] = m_data[offset];
  c[1] = m_data[offset + 1];
  c[2] = m_data[offset + 2];
  c[3] = m_data[offset + 3];
  return c;
}

inline void Image::set24(Vec2U const& pos, Vec3B const& c) {
  set24(pos[0], pos[1], c);
}

inline void Image::set24(unsigned x, unsigned y, Vec3B const& c) {

  size_t offset = y * m_width * 3 + x * 3;
  m_data[offset] = c[0];
  m_data[offset + 1] = c[1];
  m_data[offset + 2] = c[2];
}

inline auto Image::get24(unsigned x, unsigned y) const -> Vec3B {

  Vec3B c;
  size_t offset = y * m_width * 3 + x * 3;
  c[0] = m_data[offset];
  c[1] = m_data[offset + 1];
  c[2] = m_data[offset + 2];
  return c;
}

template <typename CallbackType>
void Image::forEachPixel(CallbackType&& callback) const {
  for (unsigned y = 0; y < m_height; y++) {
    for (unsigned x = 0; x < m_width; x++)
      callback(x, y, get(x, y));
  }
}

template <typename CallbackType>
void Image::forEachPixel(CallbackType&& callback) {
  for (unsigned y = 0; y < m_height; y++) {
    for (unsigned x = 0; x < m_width; x++) {
      Vec4B pixel = get(x, y);
      callback(x, y, pixel);
      set(x, y, pixel);
    }
  }
}

struct ImageView {
  [[nodiscard]] inline auto empty() const -> bool { return size.x() == 0 || size.y() == 0; }
  ImageView() = default;
  ImageView(Image const& image);

  Vec2U size{0, 0};
  std::uint8_t const* data = nullptr;
  PixelFormat format = PixelFormat::RGB24;
};

}
