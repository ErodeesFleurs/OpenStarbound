module;

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarJson.hpp"
import star.input_event;
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
#include "StarRect.hpp"
import star.application_controller;
import star.color;
import star.font;
#include "StarVariant.hpp"
#include "StarPoly.hpp"
#include "StarBiMap.hpp"
#include "StarRefPtr.hpp"
import star.renderer;
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarString.hpp"
import star.text;
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
#include "StarThread.hpp"
import star.game_types;
#include "StarSet.hpp"
import star.audio;
#include "StarList.hpp"
#include "StarMap.hpp"
import star.font_texture_group;
import star.anchor_types;
import star.text_painter;
import star.drawable;
import star.asset_texture_group;
import star.drawable_painter;
import star.gui_types;
import star.key_bindings;
import star.mixer;
import star.gui_context;
import star.widget;
import star.button_group;
import star.button_widget;
import star.portrait_widget;
import star.label_widget;

namespace Star {

STAR_CLASS(Player);

STAR_CLASS(LargeCharPlateWidget);
class LargeCharPlateWidget : public ButtonWidget {
public:
  LargeCharPlateWidget(WidgetCallbackFunc mainCallback, PlayerPtr player = PlayerPtr());

  void mouseOut() override;

  void setPlayer(PlayerPtr player = PlayerPtr());

  void enableDelete(WidgetCallbackFunc const& callback);
  void disableDelete();

  virtual bool sendEvent(InputEvent const& event) override;

  void update(float dt) override;

protected:
  virtual void renderImpl() override;

private:
  PlayerPtr m_player;
  Json m_config;

  PortraitWidgetPtr m_portrait;
  Vec2I m_portraitOffset;
  float m_portraitScale;

  String m_playerPlateHover;
  String m_noPlayerPlate;
  String m_noPlayerPlateHover;
  String m_playerPlate;

  LabelWidgetPtr m_playerName;
  LabelWidgetPtr m_playerPhrase;
  LabelWidgetPtr m_modeName;
  LabelWidgetPtr m_mode;

  ButtonWidgetPtr m_delete;

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

export module star.large_char_plate_widget;

export namespace Star {
  using ::Star::LargeCharPlateWidget;
  using ::Star::Player;
  using ::Star::PlayerPtr;
  using ::Star::PlayerConstPtr;
  using ::Star::PlayerWeakPtr;
  using ::Star::PlayerConstWeakPtr;
  using ::Star::PlayerUPtr;
  using ::Star::PlayerConstUPtr;
  using ::Star::LargeCharPlateWidgetPtr;
  using ::Star::LargeCharPlateWidgetConstPtr;
  using ::Star::LargeCharPlateWidgetWeakPtr;
  using ::Star::LargeCharPlateWidgetConstWeakPtr;
  using ::Star::LargeCharPlateWidgetUPtr;
  using ::Star::LargeCharPlateWidgetConstUPtr;
}
