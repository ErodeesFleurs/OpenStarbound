#pragma once

#include "StarConfig.hpp"
#include "StarException.hpp"
#include "StarImage.hpp"
#include "StarString.hpp"

import std;

namespace Star {

using FontException = ExceptionDerived<"FontException">;

struct FontImpl;

class Font {
public:
  static auto loadFont(String const& fileName, unsigned pixelSize = 12) -> Ptr<Font>;
  static auto loadFont(ConstPtr<ByteArray> const& bytes, unsigned pixelSize = 12) -> Ptr<Font>;

  Font();
  ~Font();

  Font(Font const&) = delete;
  auto operator=(Font const&) -> Font const& = delete;

  // Create a new font from the same data
  [[nodiscard]] auto clone() const -> Ptr<Font>;

  void setPixelSize(unsigned pixelSize);
  void setAlphaThreshold(std::uint8_t alphaThreshold = 0);

  [[nodiscard]] auto height() const -> unsigned;
  auto width(String::Char c) -> unsigned;

  // May return empty image on unrenderable character (Normally, this will
  // render a box, but if there is an internal freetype error this may return
  // an empty image).
  auto render(String::Char c) -> std::tuple<Image, Vec2I, bool>;
  auto exists(String::Char c) -> bool;

private:
  Ptr<FontImpl> m_fontImpl;
  ConstPtr<ByteArray> m_fontBuffer;
  unsigned m_pixelSize;
  std::uint8_t m_alphaThreshold;

  void loadFontImpl();
  HashMap<std::pair<String::Char, unsigned>, unsigned> m_widthCache;
};

}// namespace Star
