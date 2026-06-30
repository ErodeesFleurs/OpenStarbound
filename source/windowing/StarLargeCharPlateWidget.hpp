#pragma once

#include "StarWidget.hpp"
#include "StarButtonWidget.hpp"
#include "StarPortraitWidget.hpp"
#include "StarLabelWidget.hpp"

namespace Star {

class Player;
using PlayerPtr = SharedPtr<Player>;

class LargeCharPlateWidget;
using LargeCharPlateWidgetPtr = SharedPtr<LargeCharPlateWidget>;
class LargeCharPlateWidget : public ButtonWidget {
public:
  LargeCharPlateWidget(GuiContext& context, WidgetCallbackFunc mainCallback, PlayerPtr player = PlayerPtr());

  void mouseOut() override;

  void setPlayer(PlayerPtr player = PlayerPtr());

  void enableDelete(WidgetCallbackFunc const& callback);
  void disableDelete();

  [[nodiscard]] bool sendEvent(InputEvent const& event) override;

  void update(float dt) override;

protected:
  void renderImpl() override;

private:
  PlayerPtr m_player;
  Json m_config;

  WidgetRef<PortraitWidget> m_portrait;
  Vec2I m_portraitOffset;
  float m_portraitScale = 0.0f;

  String m_playerPlateHover;
  String m_noPlayerPlate;
  String m_noPlayerPlateHover;
  String m_playerPlate;

  WidgetRef<LabelWidget> m_playerName;
  WidgetRef<LabelWidget> m_modeName;
  WidgetRef<LabelWidget> m_mode;

  WidgetRef<ButtonWidget> m_delete;

  Vec2I m_playerNameOffset;
  Vec2I m_playerPhraseOffset;
  Vec2I m_modeNameOffset;
  Vec2I m_modeOffset;
  Vec2I m_deleteOffset;

  String m_createCharText;
  Color m_createCharTextColor;

  Color m_regularTextColor;
  Color m_disabledTextColor;
};

}
