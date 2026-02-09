#pragma once

#include "StarConfig.hpp"
#include "StarDirectives.hpp"
#include "StarFont.hpp"
#include "StarRenderer.hpp"

import std;

namespace Star {

class FontTextureGroup {
public:
  // Font* is only included for key uniqueness and should not be dereferenced
  using GlyphDescriptor = std::tuple<String::Char, unsigned, std::size_t, Font*>;

  struct GlyphTexture {
    RefPtr<Texture> texture;
    bool colored = false;
    std::int64_t time = 0;
    Vec2F offset;
  };

  FontTextureGroup(Ptr<TextureGroup> textureGroup);

  auto glyphTexture(String::Char, unsigned fontSize, Directives const* processingDirectives = nullptr) -> const GlyphTexture&;

  auto glyphTexturePtr(String::Char, unsigned fontSize) -> RefPtr<Texture>;
  auto glyphTexturePtr(String::Char, unsigned fontSize, Directives const* processingDirectives = nullptr) -> RefPtr<Texture>;

  auto glyphWidth(String::Char c, unsigned fontSize) -> unsigned;

  // Removes glyphs that haven't been used in more than the given time in
  // milliseconds
  void cleanup(std::int64_t timeout);
  // Switches the current font
  void switchFont(String const& font);
  auto activeFont() -> String const&;
  void addFont(Ptr<Font> const& font, String const& name);
  void clearFonts();
  void setFixedFonts(String const& defaultFontName, String const& fallbackFontName, String const& emojiFontName);

private:
  auto getFontForCharacter(String::Char) -> Font*;

  CaseInsensitiveStringMap<Ptr<Font>> m_fonts;
  String m_fontName;
  Ptr<Font> m_activeFont;
  Ptr<Font> m_defaultFont;
  Ptr<Font> m_fallbackFont;
  Ptr<Font> m_emojiFont;

  Ptr<TextureGroup> m_textureGroup;
  HashMap<GlyphDescriptor, GlyphTexture> m_glyphs;
};

}// namespace Star
