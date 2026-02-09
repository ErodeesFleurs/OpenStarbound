#pragma once

#include "StarFontTextureGroup.hpp"
#include "StarAnchorTypes.hpp"
#include "StarListener.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"

import std;

namespace Star {

// deprecated in favor of explicit shadow color
enum class FontMode : std::uint8_t {
  Normal,
  Shadow
};

inline auto fontModeToColor(FontMode mode) -> Color const& {
  return mode == FontMode::Shadow ? Color::Black : Color::Clear;
}

struct TextPositioning {
  TextPositioning();

  TextPositioning(Vec2F pos,
      HorizontalAnchor hAnchor = HorizontalAnchor::LeftAnchor,
      VerticalAnchor vAnchor = VerticalAnchor::BottomAnchor,
      std::optional<unsigned> wrapWidth = {},
      std::optional<unsigned> charLimit = {});

  TextPositioning(Json const& v);
  [[nodiscard]] auto toJson() const -> Json;

  [[nodiscard]] auto translated(Vec2F translation) const -> TextPositioning;

  Vec2F pos;
  HorizontalAnchor hAnchor;
  VerticalAnchor vAnchor;
  std::optional<unsigned> wrapWidth;
  std::optional<unsigned> charLimit;
};

// Renders text while caching individual glyphs for fast rendering but with *no
// kerning*.
class TextPainter {
public:
  TextPainter(Ptr<Renderer> renderer, Ptr<TextureGroup> textureGroup);

  auto renderText(StringView s, TextPositioning const& position) -> RectF;
  auto renderLine(StringView s, TextPositioning const& position) -> RectF;
  auto renderGlyph(String::Char c, TextPositioning const& position) -> RectF;

  auto determineTextSize(StringView s, TextPositioning const& position) -> RectF;
  auto determineLineSize(StringView s, TextPositioning const& position) -> RectF;
  auto determineGlyphSize(String::Char c, TextPositioning const& position) -> RectF;

  auto glyphWidth(String::Char c) -> int;
  auto stringWidth(StringView s, unsigned charLimit = 0) -> int;


  using WrapTextCallback = std::function<bool(StringView, unsigned)>;
  auto processWrapText(StringView s, unsigned* wrapWidth, WrapTextCallback textFunc) -> bool;

  auto wrapTextViews(StringView s, std::optional<unsigned> wrapWidth) -> List<StringView>;
  auto wrapText(StringView s, std::optional<unsigned> wrapWidth) -> StringList;

  [[nodiscard]] auto fontSize() const -> unsigned;
  void setFontSize(unsigned size);
  void setLineSpacing(float lineSpacing);
  void setMode(FontMode mode);
  void setFontColor(Vec4B color);
  void setProcessingDirectives(StringView directives, bool back = false);
  void setFont(String const& font);
  auto setTextStyle(TextStyle const& textStyle) -> TextStyle&;
  void clearTextStyle();
  void addFont(Ptr<Font> const& font, String const& name);
  void reloadFonts();

  void cleanup(std::int64_t textureTimeout);
  void applyCommands(StringView unsplitCommands);
private:
  void modifyDirectives(Directives& directives);
  auto doRenderText(StringView s, TextPositioning const& position, bool reallyRender, unsigned* charLimit) -> RectF;
  auto doRenderLine(StringView s, TextPositioning const& position, bool reallyRender, unsigned* charLimit) -> RectF;
  auto doRenderGlyph(String::Char c, TextPositioning const& position, bool reallyRender) -> RectF;

  void renderPrimitives();
  void renderGlyph(String::Char c, Vec2F const& screenPos, List<RenderPrimitive>& out, unsigned fontSize, float scale, Vec4B color, Directives const* processingDirectives = nullptr);
  static auto loadFont(String const& fontPath, std::optional<String> fontName = {}) -> Ptr<Font>;

  Ptr<Renderer> m_renderer;
  List<RenderPrimitive> m_shadowPrimitives;
  List<RenderPrimitive> m_backPrimitives;
  List<RenderPrimitive> m_frontPrimitives;
  FontTextureGroup m_fontTextureGroup;

  TextStyle m_defaultRenderSettings;
  TextStyle m_renderSettings;
  TextStyle m_savedRenderSettings;

  String m_nonRenderedCharacters;
  Ptr<TrackerListener> m_reloadTracker;
};

}
