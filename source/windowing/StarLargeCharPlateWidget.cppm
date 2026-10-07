module;

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
import star.application;
#include "StarStatisticsService.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarUserGeneratedContentService.hpp"
#include "StarDesktopService.hpp"
#include "StarImage.hpp"
#include "StarRect.hpp"
import star.application_controller;
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarVariant.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
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
#include "StarJson.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
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
