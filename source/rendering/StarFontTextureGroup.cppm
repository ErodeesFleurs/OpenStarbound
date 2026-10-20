module;
import std;
import star.json;
import star.string;
import star.map;

import star.color;
import star.font;
import star.variant;
import star.image;
import star.bi_map;
import star.ref_ptr;
import star.renderer;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;

import star.vector;
namespace Star {
class Root;


class FontTextureGroup {
public:
  // Font* is only included for key uniqueness and should not be dereferenced
  typedef std::tuple<String::Char, unsigned, std::size_t, Font*> GlyphDescriptor;

  struct GlyphTexture {
    TexturePtr texture;
    bool colored = false;
    std::int64_t time = 0;
    Vec2F offset;
  };

  FontTextureGroup(Root& root, std::shared_ptr<TextureGroup> textureGroup);

  const GlyphTexture& glyphTexture(String::Char, unsigned fontSize, Directives const* processingDirectives = nullptr);

  TexturePtr glyphTexturePtr(String::Char, unsigned fontSize);
  TexturePtr glyphTexturePtr(String::Char, unsigned fontSize, Directives const* processingDirectives = nullptr);

  unsigned glyphWidth(String::Char c, unsigned fontSize);

  // Removes glyphs that haven't been used in more than the given time in
  // milliseconds
  void cleanup(std::int64_t timeout);
  // Switches the current font
  void switchFont(String const& font);
  String const& activeFont();
  void addFont(std::shared_ptr<Font> const& font, String const& name);
  void clearFonts();
  void setFixedFonts(String const& defaultFontName, String const& fallbackFontName, String const& emojiFontName);
private:
  Font* getFontForCharacter(String::Char);

  Root& m_root;
  CaseInsensitiveStringMap<std::shared_ptr<Font>> m_fonts;
  String m_fontName;
  std::shared_ptr<Font> m_activeFont;
  std::shared_ptr<Font> m_defaultFont;
  std::shared_ptr<Font> m_fallbackFont;
  std::shared_ptr<Font> m_emojiFont;

  std::shared_ptr<TextureGroup> m_textureGroup;
  HashMap<GlyphDescriptor, GlyphTexture> m_glyphs;
};

}

export module star.font_texture_group;

export namespace Star {
  using ::Star::FontTextureGroup;
}
