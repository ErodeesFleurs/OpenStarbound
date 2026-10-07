module;

#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarVariant.hpp"
#include "StarImage.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
#include "StarBiMap.hpp"
#include "StarRefPtr.hpp"
import star.renderer;
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;

namespace Star {

STAR_CLASS(FontTextureGroup);

class FontTextureGroup {
public:
  // Font* is only included for key uniqueness and should not be dereferenced
  typedef tuple<String::Char, unsigned, size_t, Font*> GlyphDescriptor;

  struct GlyphTexture {
    TexturePtr texture;
    bool colored = false;
    int64_t time = 0;
    Vec2F offset;
  };

  FontTextureGroup(TextureGroupPtr textureGroup);

  const GlyphTexture& glyphTexture(String::Char, unsigned fontSize, Directives const* processingDirectives = nullptr);

  TexturePtr glyphTexturePtr(String::Char, unsigned fontSize);
  TexturePtr glyphTexturePtr(String::Char, unsigned fontSize, Directives const* processingDirectives = nullptr);

  unsigned glyphWidth(String::Char c, unsigned fontSize);

  // Removes glyphs that haven't been used in more than the given time in
  // milliseconds
  void cleanup(int64_t timeout);
  // Switches the current font
  void switchFont(String const& font);
  String const& activeFont();
  void addFont(FontPtr const& font, String const& name);
  void clearFonts();
  void setFixedFonts(String const& defaultFontName, String const& fallbackFontName, String const& emojiFontName);
private:
  Font* getFontForCharacter(String::Char);

  CaseInsensitiveStringMap<FontPtr> m_fonts;
  String m_fontName;
  FontPtr m_activeFont;
  FontPtr m_defaultFont;
  FontPtr m_fallbackFont;
  FontPtr m_emojiFont;

  TextureGroupPtr m_textureGroup;
  HashMap<GlyphDescriptor, GlyphTexture> m_glyphs;
};

}

export module star.font_texture_group;

export namespace Star {
  using ::Star::FontTextureGroup;
  using ::Star::FontTextureGroupPtr;
  using ::Star::FontTextureGroupConstPtr;
  using ::Star::FontTextureGroupWeakPtr;
  using ::Star::FontTextureGroupConstWeakPtr;
  using ::Star::FontTextureGroupUPtr;
  using ::Star::FontTextureGroupConstUPtr;
}
