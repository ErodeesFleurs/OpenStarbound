#pragma once

#include "StarString.hpp"
#include "StarImage.hpp"
#include "StarByteArray.hpp"
#include "StarMap.hpp"

namespace Star {

struct FontExceptionTag { static constexpr char const* typeName = "FontException"; };
using FontException = TypedException<StarException, FontExceptionTag>;

struct FontImpl;
using FontImplPtr = SharedPtr<FontImpl>;
class Font;
using FontPtr = SharedPtr<Font>;

class Font {
public:
  [[nodiscard]] static FontPtr loadFont(String const& fileName, unsigned pixelSize = 12);
  [[nodiscard]] static FontPtr loadFont(ByteArrayConstPtr const& bytes, unsigned pixelSize = 12);

  Font() = default;
  ~Font();

  Font(Font const&) = delete;
  Font const& operator=(Font const&) = delete;

  // Create a new font from the same data
  [[nodiscard]] FontPtr clone() const;

  void setPixelSize(unsigned pixelSize);
  void setAlphaThreshold(uint8_t alphaThreshold = 0);

  [[nodiscard]] unsigned height() const;
  [[nodiscard]] unsigned width(String::Char c);

  // May return empty image on unrenderable character (Normally, this will
  // render a box, but if there is an internal freetype error this may return
  // an empty image).
  [[nodiscard]] tuple<Image, Vec2I, bool> render(String::Char c);
  [[nodiscard]] bool exists(String::Char c);

private:
  FontImplPtr m_fontImpl;
  ByteArrayConstPtr m_fontBuffer;
  unsigned m_pixelSize = 0;
  uint8_t m_alphaThreshold = 0;

  void loadFontImpl();
  HashMap<pair<String::Char, unsigned>, unsigned> m_widthCache;
};

}
