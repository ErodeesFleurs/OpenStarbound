#include <ft2build.h>
#include FT_FREETYPE_H
import std;
import star.file;
import star.format;
import star.vector;

import star.string;
import star.image;
import star.font;

import star.byte_array;
import star.math_common;
namespace Star {

constexpr int FontLoadFlags = FT_LOAD_NO_SVG | FT_LOAD_COLOR;

struct FTContext {
  FT_Library library;
  std::mutex mutex;

  FTContext() {
    library = nullptr;
    if (FT_Init_FreeType(&library))
      throw FontException("Could not initialize FreeType library.");
  }

  ~FTContext() {
    if (library) {
      FT_Done_FreeType(library);
      library = nullptr;
    }
  }
};

FontLibrary::FontLibrary() : m_context(std::make_shared<FTContext>()) {}
FontLibrary::~FontLibrary() = default;

struct FontImpl {
  std::shared_ptr<FTContext> context;
  FT_Face face = nullptr;

  ~FontImpl() {
    if (face) {
      std::lock_guard lock(context->mutex);
      FT_Done_Face(face);
    }
  }
};

std::shared_ptr<Font> Font::loadFont(FontLibrary const& library, String const& fileName, unsigned pixelSize) {
  return loadFont(library, std::make_shared<ByteArray>(File::readFile(fileName)), pixelSize);
}

std::shared_ptr<Font> Font::loadFont(FontLibrary const& library, std::shared_ptr<ByteArray const> const& bytes, unsigned pixelSize) {
  std::shared_ptr<Font> font = std::make_shared<Font>(library);
  font->m_fontBuffer = bytes;
  font->setPixelSize(pixelSize);

  return font;
}

Font::Font(FontLibrary const& library) : Font(library.m_context) {}

Font::Font(std::shared_ptr<FTContext> context)
  : m_context(std::move(context)), m_pixelSize(0), m_alphaThreshold(0) {}

Font::~Font() = default;

std::shared_ptr<Font> Font::clone() const {
  std::shared_ptr<Font> font(new Font(m_context));
  font->m_fontBuffer = m_fontBuffer;
  font->setPixelSize(m_pixelSize);
  return font;
}

void Font::setPixelSize(unsigned pixelSize) {
  if (pixelSize == 0)
    pixelSize = 1;

  if (m_pixelSize == pixelSize)
    return;

  m_pixelSize = pixelSize;
  if (m_fontImpl && FT_Set_Pixel_Sizes(m_fontImpl->face, m_pixelSize, 0))
    throw FontException(strf("Cannot set font pixel size to: {}", m_pixelSize));
}

void Font::setAlphaThreshold(std::uint8_t alphaThreshold) {
  m_alphaThreshold = alphaThreshold;
}

unsigned Font::height() const {
  return m_pixelSize;
}

unsigned Font::width(String::Char c) {
  if (auto width = m_widthCache.maybe({c, m_pixelSize})) {
    return *width;
  } else {
    loadFontImpl();
    FT_Load_Char(m_fontImpl->face, c, FontLoadFlags);
    unsigned newWidth = (m_fontImpl->face->glyph->linearHoriAdvance + 32768) / 65536;
    m_widthCache.insert({c, m_pixelSize}, newWidth);
    return newWidth;
  }
}

void Font::loadFontImpl() {
  if (!m_fontImpl) {
    if (!m_fontBuffer || m_fontBuffer->empty())
      throw FontException("Font buffer is null or empty");

    std::shared_ptr<FontImpl> fontImpl = std::make_shared<FontImpl>();
    fontImpl->context = m_context;
    {
      std::lock_guard lock(m_context->mutex);
      if (FT_New_Memory_Face(m_context->library, (FT_Byte const*)m_fontBuffer->ptr(), m_fontBuffer->size(), 0, &fontImpl->face))
        throw FontException::format("Could not load font from buffer");
    }

    if (FT_Set_Pixel_Sizes(fontImpl->face, m_pixelSize, 0))
      throw FontException::format("Cannot set font pixel size to: {}", m_pixelSize);

    m_fontImpl = fontImpl;
  }
}

std::tuple<Image, Vec2I, bool> Font::render(String::Char c) {
  loadFontImpl();

  FT_Face face = m_fontImpl->face;
  if (FT_Load_Glyph(face, FT_Get_Char_Index(face, c), FontLoadFlags) != 0)
    return {};

  if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL) != 0)
    return {};

  FT_GlyphSlot slot = face->glyph;
  if (!slot->bitmap.buffer)
    return {};

  unsigned width = slot->bitmap.width;
  unsigned height = slot->bitmap.rows;
  bool colored = false;

  Image image(width + 2, height + 2, PixelFormat::BGRA32);
  if (slot->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY) {
    Vec4B white(255, 255, 255, 0);
    image.fill(white);
    for (unsigned y = 0; y != height; ++y) {
      std::uint8_t* p = slot->bitmap.buffer + y * slot->bitmap.pitch;
      for (unsigned x = 0; x != width; ++x) {
        if (x < width && y < height) {
          white[3] = m_alphaThreshold
            ? (*(p + x) >= m_alphaThreshold ? 255 : 0)
            :  *(p + x);
          image.set(x + 1, height - y, white);
        }
      }
    }
  } else if ((colored = (slot->bitmap.pixel_mode == FT_PIXEL_MODE_BGRA))) {
    unsigned bpp = image.bytesPerPixel();
    std::uint8_t* data = image.data() + bpp + ((image.width() * (image.height() - 2)) * bpp); // offset by 1 pixel as it's padded
    for (std::size_t y = 0; y != height; ++y) {
      std::memcpy(data - (y * image.width() * bpp),
             slot->bitmap.buffer + y * slot->bitmap.pitch,
             slot->bitmap.pitch);
    }
    // unfortunately FreeType pre-multiplied the color channels :(
    image.forEachPixel([](unsigned, unsigned, Vec4B& pixel) {
      if (pixel[3] != 0 && pixel[3] != 255) {
        float a = byteToFloat(pixel[3]);
        pixel[0] /= a;
        pixel[1] /= a;
        pixel[2] /= a;
      }
    });
  } else {
    return {};
  }

  return std::make_tuple(
    std::move(image),
    Vec2I(slot->bitmap_left - 1, (slot->bitmap_top - height) + (m_pixelSize / 4) - 1),
    colored);
}

bool Font::exists(String::Char c) {
  loadFontImpl();
  return FT_Get_Char_Index(m_fontImpl->face, c);
}

}
