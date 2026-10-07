module;

#include "StarString.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
#include "StarJson.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarInputEvent.hpp"
#include "StarVector.hpp"
#include "StarSet.hpp"

namespace Star {
class Image;
struct Drawable;
struct TextStyle;
struct TextPositioning;
enum class FontMode : uint8_t;
STAR_CLASS(ApplicationController);
STAR_CLASS(Renderer);
STAR_CLASS(Mixer);
STAR_CLASS(TextPainter);
STAR_CLASS(AssetTextureGroup);
STAR_CLASS(DrawablePainter);
STAR_CLASS(AudioInstance);
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
  // Get pointer to the singleton root instance, if it exists.  Otherwise,
  // returns nullptr.
  static GuiContext* singletonPtr();

  // Gets reference to GuiContext singleton, throws GuiContextException if root
  // is not initialized.
  static GuiContext& singleton();

  GuiContext(MixerPtr mixer, ApplicationControllerPtr appController);
  ~GuiContext();

  GuiContext(GuiContext const&) = delete;
  GuiContext& operator=(GuiContext const&) = delete;

  void renderInit(RendererPtr renderer);

  MixerPtr const& mixer() const;
  ApplicationControllerPtr const& applicationController() const;
  RendererPtr const& renderer() const;
  AssetTextureGroupPtr const& assetTextureGroup() const;
  TextPainterPtr const& textPainter() const;

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

  void drawTriangles(List<tuple<Vec2F, Vec2F, Vec2F>> const& triangles, Vec4B const& color);

  void drawInterfaceDrawable(Drawable drawable, Vec2F const& screenPos, Vec4B const& color = Vec4B::filled(255));

  void drawInterfaceLine(Vec2F const& begin, Vec2F const end, Vec4B const& color, float lineWidth = 1);
  void drawInterfacePolyLines(PolyF poly, Vec4B const& color, float lineWidth = 1);

  void drawInterfaceTriangles(List<tuple<Vec2F, Vec2F, Vec2F>> const& triangles, Vec4B const& color);

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

  void playAudio(AudioInstancePtr audioInstance);
  AudioInstancePtr playAudio(String const& audioAsset, int loops = 0, float volume = 1.0f, float pitch = 1.0f);

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
  static GuiContext* s_singleton;

  MixerPtr m_mixer;
  ApplicationControllerPtr m_applicationController;
  RendererPtr m_renderer;

  AssetTextureGroupPtr m_textureCollection;
  TextPainterPtr m_textPainter;
  DrawablePainterPtr m_drawablePainter;

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
  using ::Star::ApplicationControllerPtr;
  using ::Star::ApplicationControllerConstPtr;
  using ::Star::ApplicationControllerWeakPtr;
  using ::Star::ApplicationControllerConstWeakPtr;
  using ::Star::ApplicationControllerUPtr;
  using ::Star::ApplicationControllerConstUPtr;
  using ::Star::Renderer;
  using ::Star::RendererPtr;
  using ::Star::RendererConstPtr;
  using ::Star::RendererWeakPtr;
  using ::Star::RendererConstWeakPtr;
  using ::Star::RendererUPtr;
  using ::Star::RendererConstUPtr;
  using ::Star::Mixer;
  using ::Star::MixerPtr;
  using ::Star::MixerConstPtr;
  using ::Star::MixerWeakPtr;
  using ::Star::MixerConstWeakPtr;
  using ::Star::MixerUPtr;
  using ::Star::MixerConstUPtr;
  using ::Star::TextPainter;
  using ::Star::TextPainterPtr;
  using ::Star::TextPainterConstPtr;
  using ::Star::TextPainterWeakPtr;
  using ::Star::TextPainterConstWeakPtr;
  using ::Star::TextPainterUPtr;
  using ::Star::TextPainterConstUPtr;
  using ::Star::AssetTextureGroup;
  using ::Star::AssetTextureGroupPtr;
  using ::Star::AssetTextureGroupConstPtr;
  using ::Star::AssetTextureGroupWeakPtr;
  using ::Star::AssetTextureGroupConstWeakPtr;
  using ::Star::AssetTextureGroupUPtr;
  using ::Star::AssetTextureGroupConstUPtr;
  using ::Star::DrawablePainter;
  using ::Star::DrawablePainterPtr;
  using ::Star::DrawablePainterConstPtr;
  using ::Star::DrawablePainterWeakPtr;
  using ::Star::DrawablePainterConstWeakPtr;
  using ::Star::DrawablePainterUPtr;
  using ::Star::DrawablePainterConstUPtr;
  using ::Star::AudioInstance;
  using ::Star::AudioInstancePtr;
  using ::Star::AudioInstanceConstPtr;
  using ::Star::AudioInstanceWeakPtr;
  using ::Star::AudioInstanceConstWeakPtr;
  using ::Star::AudioInstanceUPtr;
  using ::Star::AudioInstanceConstUPtr;
}
