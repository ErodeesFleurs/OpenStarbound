#pragma once

#include "StarAnchorTypes.hpp"
#include "StarAssets.hpp"
#include "StarFontTextureGroup.hpp"
#include "StarListener.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"

namespace Star {

// deprecated in favor of explicit shadow color
enum class FontMode : uint8_t {
  Normal,
  Shadow
};

inline Color const& fontModeToColor(FontMode mode) {
  return mode == FontMode::Shadow ? Color::Black : Color::Clear;
}

class TextPainter;
using TextPainterPtr = SharedPtr<TextPainter>;

struct TextPositioning {
  TextPositioning() = default;

  TextPositioning(Vec2F pos,
                  HorizontalAnchor hAnchor = HorizontalAnchor::LeftAnchor,
                  VerticalAnchor vAnchor = VerticalAnchor::BottomAnchor,
                  Maybe<unsigned> wrapWidth = {},
                  Maybe<unsigned> charLimit = {});

  explicit TextPositioning(Json const& v);
  [[nodiscard]] Json toJson() const;

  [[nodiscard]] TextPositioning translated(Vec2F translation) const;

  Vec2F pos;
  HorizontalAnchor hAnchor = HorizontalAnchor::LeftAnchor;
  VerticalAnchor vAnchor = VerticalAnchor::BottomAnchor;
  Maybe<unsigned> wrapWidth;
  Maybe<unsigned> charLimit;
};

// Renders text while caching individual glyphs for fast rendering but with *no
// kerning*.
class TextPainter {
public:
  TextPainter(RendererPtr renderer, TextureGroupPtr textureGroup, AssetsConstPtr assets, function<void(ListenerWeakPtr)> registerReloadListener);

  [[nodiscard]] RectF renderText(StringView s, TextPositioning const& position);
  [[nodiscard]] RectF renderLine(StringView s, TextPositioning const& position);
  [[nodiscard]] RectF renderGlyph(String::Char c, TextPositioning const& position);

  [[nodiscard]] RectF determineTextSize(StringView s, TextPositioning const& position);
  [[nodiscard]] RectF determineLineSize(StringView s, TextPositioning const& position);
  [[nodiscard]] RectF determineGlyphSize(String::Char c, TextPositioning const& position);

  [[nodiscard]] int glyphWidth(String::Char c);
  [[nodiscard]] int stringWidth(StringView s, unsigned charLimit = 0);

  using WrapTextCallback = function<bool(StringView, unsigned)>;
  [[nodiscard]] bool processWrapText(StringView s, unsigned* wrapWidth, WrapTextCallback textFunc);

  [[nodiscard]] List<StringView> wrapTextViews(StringView s, Maybe<unsigned> wrapWidth);
  [[nodiscard]] StringList wrapText(StringView s, Maybe<unsigned> wrapWidth);

  [[nodiscard]] unsigned fontSize() const;
  void setFontSize(unsigned size);
  void setLineSpacing(float lineSpacing);
  void setMode(FontMode mode);
  void setFontColor(Vec4B color);
  void setProcessingDirectives(StringView directives, bool back = false);
  void setFont(String const& font);
  [[nodiscard]] TextStyle& setTextStyle(TextStyle const& textStyle);
  void clearTextStyle();
  void addFont(FontPtr const& font, String const& name);
  void reloadFonts();

  void cleanup(int64_t textureTimeout);
  void applyCommands(StringView unsplitCommands);

private:
  void modifyDirectives(Directives& directives);
  [[nodiscard]] RectF doRenderText(StringView s, TextPositioning const& position, bool reallyRender, unsigned* charLimit);
  [[nodiscard]] RectF doRenderLine(StringView s, TextPositioning const& position, bool reallyRender, unsigned* charLimit);
  [[nodiscard]] RectF doRenderGlyph(String::Char c, TextPositioning const& position, bool reallyRender);

  void renderPrimitives();
  void renderGlyph(String::Char c, Vec2F const& screenPos, List<RenderPrimitive>& out, unsigned fontSize, float scale, Vec4B color, Directives const* processingDirectives = nullptr);
  [[nodiscard]] FontPtr loadFont(String const& fontPath, Maybe<String> fontName = {});

  RendererPtr m_renderer;
  AssetsConstPtr m_assets;
  function<void(ListenerWeakPtr)> m_registerReloadListener;
  List<RenderPrimitive> m_shadowPrimitives;
  List<RenderPrimitive> m_backPrimitives;
  List<RenderPrimitive> m_frontPrimitives;
  FontTextureGroup m_fontTextureGroup;

  TextStyle m_defaultRenderSettings;
  TextStyle m_renderSettings;
  TextStyle m_savedRenderSettings;

  String m_nonRenderedCharacters;
  TrackerListenerPtr m_reloadTracker;
};

}// namespace Star
