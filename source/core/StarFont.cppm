module;
import std;
import star.format;
import star.string;
import star.vector;
import star.image;
import star.byte_array;
import star.map;

namespace Star {

struct FontExceptionTag {
  static constexpr char const* name() { return "FontException"; }
};
using FontException = StarError<FontExceptionTag, StarException>;

struct FontImpl;

class Font {
public:
  static std::shared_ptr<Font> loadFont(String const& fileName, unsigned pixelSize = 12);
  static std::shared_ptr<Font> loadFont(std::shared_ptr<ByteArray const> const& bytes, unsigned pixelSize = 12);

  Font();
  ~Font();

  Font(Font const&) = delete;
  Font const& operator=(Font const&) = delete;

  // Create a new font from the same data
  std::shared_ptr<Font> clone() const;

  void setPixelSize(unsigned pixelSize);
  void setAlphaThreshold(std::uint8_t alphaThreshold = 0);

  unsigned height() const;
  unsigned width(String::Char c);

  // May return empty image on unrenderable character (Normally, this will
  // render a box, but if there is an internal freetype error this may return
  // an empty image).
  std::tuple<Image, Vec2I, bool> render(String::Char c);
  bool exists(String::Char c);

private:
  std::shared_ptr<FontImpl> m_fontImpl;
  std::shared_ptr<ByteArray const> m_fontBuffer;
  unsigned m_pixelSize;
  std::uint8_t m_alphaThreshold;

  void loadFontImpl();
  HashMap<std::pair<String::Char, unsigned>, unsigned> m_widthCache;
};

}

export module star.font;

export namespace Star {
  using ::Star::FontExceptionTag;
  using ::Star::FontException;
  using ::Star::FontImpl;
  using ::Star::Font;
}
