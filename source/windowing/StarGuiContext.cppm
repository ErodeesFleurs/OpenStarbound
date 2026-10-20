module;
import std;
import star.maybe;
import star.format;
import star.json;
import star.byte_array;

import star.string;
import star.poly;
import star.list;
import star.rect;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.input_event;
import star.vector;
import star.set;

namespace Star {
class Image;
class Root;
class Input;
class Voice;
class TileDrawer;
struct Drawable;
struct TextStyle;
struct TextPositioning;
enum class FontMode : std::uint8_t;
class ApplicationController;
class Renderer;
class Mixer;
class TextPainter;
class AssetTextureGroup;
class DrawablePainter;
class AudioInstance;
}

import star.gui_types;
import star.key_bindings;

namespace Star {

struct GuiContextExceptionTag {
  static constexpr char const* name() { return "GuiContextException"; }
};
using GuiContextException = StarError<GuiContextExceptionTag, StarException>;

class GuiContext {
public:
  GuiContext(Root& root, std::shared_ptr<Mixer> mixer, std::shared_ptr<ApplicationController> appController);
  ~GuiContext();

  Root& root() const;
  void bindInput(Input& input);
  Input& input() const;
  void bindVoice(Voice& voice);
  Voice& voice() const;
  void setTileDrawer(TileDrawer* tileDrawer);
  TileDrawer* tileDrawer() const;

  GuiContext(GuiContext const&) = delete;
  GuiContext& operator=(GuiContext const&) = delete;

  void renderInit(std::shared_ptr<Renderer> renderer);

  std::shared_ptr<Mixer> const& mixer() const;
  std::shared_ptr<ApplicationController> const& applicationController() const;
  std::shared_ptr<Renderer> const& renderer() const;
  std::shared_ptr<AssetTextureGroup> const& assetTextureGroup() const;
  std::shared_ptr<TextPainter> const& textPainter() const;

  unsigned windowWidth() const;
  unsigned windowHeight() const;

  Vec2U windowSize() const;
  Vec2U windowInterfaceSize() const;

  float interfaceScale() const;
  void setInterfaceScale(float interfaceScale);

  Maybe<Vec2I> mousePosition(InputEvent const& event, float pixelRatio) const;
  Maybe<Vec2I> mousePosition(InputEvent const& event) const;

  Set<InterfaceAction> actions(InputEvent const& event) const;
  // used to cancel chorded inputs on KeyUp
  Set<InterfaceAction> actionsForKey(Key key) const;
  void refreshKeybindings();

  // Drawing wrappers to internal renderers.  Automatically loads textures before drawing.

  void setInterfaceScissorRect(RectI const& scissor);
  void resetInterfaceScissorRect();

  Vec2U textureSize(AssetPath const& texName);

  void drawQuad(RectF const& screenCoords, Vec4B const& color = Vec4B::filled(255));
  void drawQuad(AssetPath const& texName, RectF const& screenCoords, Vec4B const& color = Vec4B::filled(255));
  void drawQuad(AssetPath const& texName, Vec2F const& screenPos, float pixelRatio, Vec4B const& color = Vec4B::filled(255));
  void drawQuad(AssetPath const& texName, RectF const& texCoords, RectF const& screenCoords, Vec4B const& color = Vec4B::filled(255));

  void drawDrawable(Drawable drawable, Vec2F const& screenPos, float pixelRatio, Vec4B const& color = Vec4B::filled(255));

  void drawLine(Vec2F const& begin, Vec2F const end, Vec4B const& color, float lineWidth = 1);
  void drawPolyLines(PolyF const& poly, Vec4B const& color, float lineWidth = 1);

  void drawTriangles(List<std::tuple<Vec2F, Vec2F, Vec2F>> const& triangles, Vec4B const& color);

  void drawInterfaceDrawable(Drawable drawable, Vec2F const& screenPos, Vec4B const& color = Vec4B::filled(255));

  void drawInterfaceLine(Vec2F const& begin, Vec2F const end, Vec4B const& color, float lineWidth = 1);
  void drawInterfacePolyLines(PolyF poly, Vec4B const& color, float lineWidth = 1);

  void drawInterfaceTriangles(List<std::tuple<Vec2F, Vec2F, Vec2F>> const& triangles, Vec4B const& color);

  void drawInterfaceQuad(RectF const& screenCoords, Vec4B const& color = Vec4B::filled(255));
  void drawInterfaceQuad(AssetPath const& texName, Vec2F const& screenPos, Vec4B const& color = Vec4B::filled(255));
  void drawInterfaceQuad(AssetPath const& texName, Vec2F const& screenPos, float scale, Vec4B const& color = Vec4B::filled(255));
  void drawInterfaceQuad(AssetPath const& texName, RectF const& texCoords, RectF const& screenCoords, Vec4B const& color = Vec4B::filled(255));

  void drawImageStretchSet(ImageStretchSet const& imageSet, RectF const& screenPos, GuiDirection direction = GuiDirection::Horizontal, Vec4B const& color = Vec4B::filled(255));

  // Returns true if the hardware cursor was successfully set to the drawable. Generally fails if the Drawable isn't an image part or the image is too big.
  bool trySetCursor(Drawable const& drawable, Vec2I const& offset, int pixelRatio);

  RectF renderText(String const& s, TextPositioning const& positioning);
  RectF renderInterfaceText(String const& s, TextPositioning const& positioning);

  RectF determineTextSize(String const& s, TextPositioning const& positioning);
  RectF determineInterfaceTextSize(String const& s, TextPositioning const& positioning);

  void setFontSize(unsigned size);
  void setFontSize(unsigned size, float pixelRatio);
  void setFontColor(Vec4B const& color);
  void setFontMode(FontMode mode);
  void setFontProcessingDirectives(String const& directives);
  void setFont(String const& font);
  void setDefaultFont();
  TextStyle& setTextStyle(TextStyle const& textStyle, float pixelRatio);
  TextStyle& setTextStyle(TextStyle const& textStyle);
  void clearTextStyle();

  void setLineSpacing(float lineSpacing);
  void setDefaultLineSpacing();

  int stringWidth(String const& s);
  int stringInterfaceWidth(String const& s);

  StringList wrapText(String const& s, Maybe<unsigned> wrapWidth);
  StringList wrapInterfaceText(String const& s, Maybe<unsigned> wrapWidth);

  void playAudio(std::shared_ptr<AudioInstance> audioInstance);
  std::shared_ptr<AudioInstance> playAudio(String const& audioAsset, int loops = 0, float volume = 1.0f, float pitch = 1.0f);

  bool shiftHeld() const;
  void setShiftHeld(bool held);

  String getClipboard() const;
  bool setClipboard(String text);
  bool setClipboardData(StringMap<ByteArray> data);
  bool setClipboardImage(Image const& image, ByteArray* png, String const* path = nullptr);
  bool setClipboardFile(String const& path);
  float getDisplayScale() const;

  void cleanup();

private:
  Root& m_root;
  Input* m_input = nullptr;
  Voice* m_voice = nullptr;
  TileDrawer* m_tileDrawer = nullptr;

  std::shared_ptr<Mixer> m_mixer;
  std::shared_ptr<ApplicationController> m_applicationController;
  std::shared_ptr<Renderer> m_renderer;

  std::shared_ptr<AssetTextureGroup> m_textureCollection;
  std::shared_ptr<TextPainter> m_textPainter;
  std::shared_ptr<DrawablePainter> m_drawablePainter;

  KeyBindings m_keyBindings;

  float m_interfaceScale;

  bool m_shiftHeld;
};

}

export module star.gui_context;

export namespace Star {
  using ::Star::GuiContextExceptionTag;
  using ::Star::GuiContext;
  using ::Star::GuiContextException;
  using ::Star::Image;
  using ::Star::Drawable;
  using ::Star::TextStyle;
  using ::Star::TextPositioning;
  using ::Star::FontMode;
  using ::Star::ApplicationController;
  using ::Star::Renderer;
  using ::Star::Mixer;
  using ::Star::TextPainter;
  using ::Star::AssetTextureGroup;
  using ::Star::DrawablePainter;
  using ::Star::AudioInstance;
}
