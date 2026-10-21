module;
import std;
import star.json;
import star.color;
import star.font;
import star.list;
import star.rect;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.ordered_map;
import star.io_device;
import star.asset_path;
import star.ref_ptr;
import star.logging;
import star.listener;
import star.version;
import star.string;
import star.vector;
import star.text;


import star.image;
import star.renderer;
import star.font_texture_group;
import star.anchor_types;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

namespace Star {

// deprecated in favor of explicit shadow color
enum class FontMode : std::uint8_t {
  Normal,
  Shadow
};

inline Color const& fontModeToColor(FontMode mode) {
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
  Json toJson() const;

  TextPositioning translated(Vec2F translation) const;

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
  // Root must outlive this painter and its scoped reload subscription.
  TextPainter(Root& root, std::shared_ptr<Renderer> renderer, std::shared_ptr<TextureGroup> textureGroup);

  RectF renderText(StringView s, TextPositioning const& position);
  RectF renderLine(StringView s, TextPositioning const& position);
  RectF renderGlyph(String::Char c, TextPositioning const& position);

  RectF determineTextSize(StringView s, TextPositioning const& position);
  RectF determineLineSize(StringView s, TextPositioning const& position);
  RectF determineGlyphSize(String::Char c, TextPositioning const& position);

  int glyphWidth(String::Char c);
  int stringWidth(StringView s, unsigned charLimit = 0);

    
  typedef std::function<bool(StringView, unsigned)> WrapTextCallback;
  bool processWrapText(StringView s, unsigned* wrapWidth, WrapTextCallback textFunc);

  List<StringView> wrapTextViews(StringView s, std::optional<unsigned> wrapWidth);
  StringList wrapText(StringView s, std::optional<unsigned> wrapWidth);

  unsigned fontSize() const;
  void setFontSize(unsigned size);
  void setLineSpacing(float lineSpacing);
  void setMode(FontMode mode);
  void setFontColor(Vec4B color);
  void setProcessingDirectives(StringView directives, bool back = false);
  void setFont(String const& font);
  TextStyle& setTextStyle(TextStyle const& textStyle);
  void clearTextStyle();
  void addFont(std::shared_ptr<Font> const& font, String const& name);
  void reloadFonts();

  void cleanup(std::int64_t textureTimeout);
  void applyCommands(StringView unsplitCommands);
private:
  void modifyDirectives(Directives& directives);
  RectF doRenderText(StringView s, TextPositioning const& position, bool reallyRender, unsigned* charLimit);
  RectF doRenderLine(StringView s, TextPositioning const& position, bool reallyRender, unsigned* charLimit);
  RectF doRenderGlyph(String::Char c, TextPositioning const& position, bool reallyRender);

  void renderPrimitives();
  void renderGlyph(String::Char c, Vec2F const& screenPos, List<RenderPrimitive>& out, unsigned fontSize, float scale, Vec4B color, Directives const* processingDirectives = nullptr);
  std::shared_ptr<Font> loadFont(String const& fontPath, std::optional<String> fontName = {});

  Root& m_root;
  std::shared_ptr<Renderer> m_renderer;
  List<RenderPrimitive> m_shadowPrimitives;
  List<RenderPrimitive> m_backPrimitives;
  List<RenderPrimitive> m_frontPrimitives;
  FontTextureGroup m_fontTextureGroup;

  TextStyle m_defaultRenderSettings;
  TextStyle m_renderSettings;
  TextStyle m_savedRenderSettings;

  String m_nonRenderedCharacters;
  std::shared_ptr<TrackerListener> m_reloadTracker;
  ListenerSubscription m_reloadSubscription;
};

}

export module star.text_painter;

export namespace Star {
  using ::Star::FontMode;
  using ::Star::fontModeToColor;
  using ::Star::TextPositioning;
  using ::Star::TextPainter;
}
