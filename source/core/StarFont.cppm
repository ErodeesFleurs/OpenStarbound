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
using FontImplPtr = std::shared_ptr<FontImpl>;
using FontImplConstPtr = std::shared_ptr<const FontImpl>;
using FontImplWeakPtr = std::weak_ptr<FontImpl>;
using FontImplConstWeakPtr = std::weak_ptr<const FontImpl>;
using FontImplUPtr = std::unique_ptr<FontImpl>;
using FontImplConstUPtr = std::unique_ptr<const FontImpl>;
class Font;
using FontPtr = std::shared_ptr<Font>;
using FontConstPtr = std::shared_ptr<const Font>;
using FontWeakPtr = std::weak_ptr<Font>;
using FontConstWeakPtr = std::weak_ptr<const Font>;
using FontUPtr = std::unique_ptr<Font>;
using FontConstUPtr = std::unique_ptr<const Font>;

class Font {
public:
  static FontPtr loadFont(String const& fileName, unsigned pixelSize = 12);
  static FontPtr loadFont(ByteArrayConstPtr const& bytes, unsigned pixelSize = 12);

  Font();
  ~Font();

  Font(Font const&) = delete;
  Font const& operator=(Font const&) = delete;

  // Create a new font from the same data
  FontPtr clone() const;

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
  FontImplPtr m_fontImpl;
  ByteArrayConstPtr m_fontBuffer;
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
  using ::Star::FontImplPtr;
  using ::Star::FontImplConstPtr;
  using ::Star::FontImplWeakPtr;
  using ::Star::FontImplConstWeakPtr;
  using ::Star::FontImplUPtr;
  using ::Star::FontImplConstUPtr;
  using ::Star::Font;
  using ::Star::FontPtr;
  using ::Star::FontConstPtr;
  using ::Star::FontWeakPtr;
  using ::Star::FontConstWeakPtr;
  using ::Star::FontUPtr;
  using ::Star::FontConstUPtr;
}
