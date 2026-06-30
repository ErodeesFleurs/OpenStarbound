#pragma once

#include "StarWidget.hpp"
#include "StarPlayer.hpp"

namespace Star {

class Player;
using PlayerPtr = SharedPtr<Player>;
class PortraitWidget;
using PortraitWidgetPtr = SharedPtr<PortraitWidget>;

class PortraitWidget : public Widget {
public:
  PortraitWidget(GuiContext& context, PortraitEntityPtr entity, PortraitMode mode = PortraitMode::Full);
  explicit PortraitWidget(GuiContext& context);
  virtual ~PortraitWidget() = default;

  void setEntity(PortraitEntityPtr entity);
  void setMode(PortraitMode mode);
  void setScale(float scale);
  void setIconMode();
  void setRenderHumanoid(bool);
  bool sendEvent(InputEvent const& event) override;

protected:
  RectI getScissorRect() const override;
  void renderImpl() override;

private:
  void init();
  void updateSize();

  PortraitEntityPtr m_entity;
  PortraitMode m_portraitMode = PortraitMode::Full;
  AssetPath m_noEntityImageFull;
  AssetPath m_noEntityImagePart;
  float m_scale = 1.0f;

  bool m_renderHumanoid = false;
  bool m_iconMode = false;
  AssetPath m_iconImage;
  Vec2I m_iconOffset;
};

}
