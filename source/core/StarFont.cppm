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
struct FTContext;

// Explicit FreeType library owner. Loaded fonts retain the library, including
// when the originating asset catalog has been replaced.
class FontLibrary {
public:
  FontLibrary();
  ~FontLibrary();

  FontLibrary(FontLibrary const&) = delete;
  FontLibrary& operator=(FontLibrary const&) = delete;

private:
  friend class Font;
  std::shared_ptr<FTContext> m_context;
};

class Font {
public:
  static std::shared_ptr<Font> loadFont(FontLibrary const& library, String const& fileName, unsigned pixelSize = 12);
  static std::shared_ptr<Font> loadFont(FontLibrary const& library, std::shared_ptr<ByteArray const> const& bytes, unsigned pixelSize = 12);

  explicit Font(FontLibrary const& library);
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
  explicit Font(std::shared_ptr<FTContext> context);

  std::shared_ptr<FTContext> m_context;
  std::shared_ptr<ByteArray const> m_fontBuffer;
  std::shared_ptr<FontImpl> m_fontImpl;
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
  using ::Star::FTContext;
  using ::Star::FontLibrary;
  using ::Star::Font;
}
