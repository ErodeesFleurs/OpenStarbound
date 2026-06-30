#pragma once

#include "StarApplicationController.hpp"
#include "StarTextPainter.hpp"
#include "StarDrawablePainter.hpp"
#include "StarAssetTextureGroup.hpp"
#include "StarConfiguration.hpp"
#include "StarInputEvent.hpp"
#include "StarDrawable.hpp"
#include "StarThread.hpp"
#include "StarGuiTypes.hpp"
#include "StarRenderer.hpp"
#include "StarKeyBindings.hpp"
#include "StarMixer.hpp"
#include "StarImageMetadataDatabase.hpp"

namespace Star {

struct GuiContextExceptionTag { static constexpr char const* typeName = "GuiContextException"; };
using GuiContextException = TypedException<StarException, GuiContextExceptionTag>;

class ItemDatabase;
using ItemDatabaseConstPtr = SharedPtr<ItemDatabase const>;

struct GuiContextServices {
  AssetsConstPtr assets;
  ConfigurationPtr configuration;
  ImageMetadataDatabaseConstPtr imageMetadata;
  ItemDatabaseConstPtr itemDatabase;
  function<void(ListenerWeakPtr)> registerReloadListener;
  function<void(function<void()>)> withClipboardUnlock;
};

class GuiContext {
public:
  GuiContext(MixerPtr mixer, ApplicationControllerPtr appController, GuiContextServices services);
  ~GuiContext();

  GuiContext(GuiContext const&) = delete;
  GuiContext& operator=(GuiContext const&) = delete;

  void renderInit(RendererPtr renderer);

  [[nodiscard]] MixerPtr const& mixer() const;
  [[nodiscard]] ApplicationControllerPtr const& applicationController() const;
  [[nodiscard]] AssetsConstPtr const& assets() const;
  [[nodiscard]] ConfigurationPtr const& configuration() const;
  [[nodiscard]] ImageMetadataDatabaseConstPtr const& imageMetadata() const;
  [[nodiscard]] ItemDatabaseConstPtr const& itemDatabase() const;
  [[nodiscard]] RendererPtr const& renderer() const;
  [[nodiscard]] AssetTextureGroupPtr const& assetTextureGroup() const;
  [[nodiscard]] TextPainterPtr const& textPainter() const;

  [[nodiscard]] unsigned windowWidth() const;
  [[nodiscard]] unsigned windowHeight() const;

  [[nodiscard]] Vec2U windowSize() const;
  [[nodiscard]] Vec2U windowInterfaceSize() const;

  [[nodiscard]] float interfaceScale() const;
  void setInterfaceScale(float interfaceScale);

  [[nodiscard]] Maybe<Vec2I> mousePosition(InputEvent const& event, float pixelRatio) const;
  [[nodiscard]] Maybe<Vec2I> mousePosition(InputEvent const& event) const;

  [[nodiscard]] Set<InterfaceAction> actions(InputEvent const& event) const;
  // used to cancel chorded inputs on KeyUp
  [[nodiscard]] Set<InterfaceAction> actionsForKey(Key key) const;
  void refreshKeybindings();

  // Drawing wrappers to internal renderers.  Automatically loads textures before drawing.

  void setInterfaceScissorRect(RectI const& scissor);
  void resetInterfaceScissorRect();

  [[nodiscard]] Vec2U textureSize(AssetPath const& texName);

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
  [[nodiscard]] bool trySetCursor(Drawable const& drawable, Vec2I const& offset, int pixelRatio);

  [[nodiscard]] RectF renderText(String const& s, TextPositioning const& positioning);
  [[nodiscard]] RectF renderInterfaceText(String const& s, TextPositioning const& positioning);

  [[nodiscard]] RectF determineTextSize(String const& s, TextPositioning const& positioning);
  [[nodiscard]] RectF determineInterfaceTextSize(String const& s, TextPositioning const& positioning);

  void setFontSize(unsigned size);
  void setFontSize(unsigned size, float pixelRatio);
  void setFontColor(Vec4B const& color);
  void setFontMode(FontMode mode);
  void setFontProcessingDirectives(String const& directives);
  void setFont(String const& font);
  void setDefaultFont();
  [[nodiscard]] TextStyle& setTextStyle(TextStyle const& textStyle, float pixelRatio);
  [[nodiscard]] TextStyle& setTextStyle(TextStyle const& textStyle);
  void clearTextStyle();

  void setLineSpacing(float lineSpacing);
  void setDefaultLineSpacing();

  [[nodiscard]] int stringWidth(String const& s);
  [[nodiscard]] int stringInterfaceWidth(String const& s);

  [[nodiscard]] StringList wrapText(String const& s, Maybe<unsigned> wrapWidth);
  [[nodiscard]] StringList wrapInterfaceText(String const& s, Maybe<unsigned> wrapWidth);

  void playAudio(AudioInstancePtr audioInstance);
  void playAudio(String const& audioAsset, int loops = 0, float volume = 1.0f, float pitch = 1.0f);

  [[nodiscard]] bool shiftHeld() const;
  void setShiftHeld(bool held);

  [[nodiscard]] String getClipboard() const;
  [[nodiscard]] bool setClipboard(String text);
  [[nodiscard]] bool setClipboardData(StringMap<ByteArray> data);
  [[nodiscard]] bool setClipboardImage(Image const& image, ByteArray* png, String const* path = nullptr);
  [[nodiscard]] bool setClipboardFile(String const& path);
  void withClipboardUnlock(function<void()> callback);
  [[nodiscard]] float getDisplayScale() const;

  void cleanup();

private:
  MixerPtr m_mixer;
  ApplicationControllerPtr m_applicationController;
  AssetsConstPtr m_assets;
  ConfigurationPtr m_configuration;
  ImageMetadataDatabaseConstPtr m_imageMetadata;
  ItemDatabaseConstPtr m_itemDatabase;
  function<void(ListenerWeakPtr)> m_registerReloadListener;
  function<void(function<void()>)> m_withClipboardUnlock;
  RendererPtr m_renderer;

  AssetTextureGroupPtr m_textureCollection;
  TextPainterPtr m_textPainter;
  DrawablePainterPtr m_drawablePainter;

  KeyBindings m_keyBindings;

  float m_interfaceScale = 1.0f;

  bool m_shiftHeld = false;
};

}
