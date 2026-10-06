module;

#include "StarGameTypes.hpp"
#include "StarJson.hpp"

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarOrderedSet.hpp"
#include "StarStrongTypedef.hpp"




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
import star.pane;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.ai_types;
import star.radio_message_database;
import star.game_timers;

namespace Star {

STAR_CLASS(LabelWidget);
STAR_CLASS(ImageWidget);
STAR_CLASS(AudioInstance);

class RadioMessagePopup : public Pane {
public:
  RadioMessagePopup();

  void update(float dt) override;
  void dismissed() override;

  bool messageActive();

  void setMessage(RadioMessage message);
  void setChatHeight(int chatHeight);
  void interrupt();

private:
  enum PopupStage { AnimateIn, ScrollText, Persist, AnimateOut, Hidden };

  void updateAnchorOffset();
  void nextPopupStage();
  void enterStage(PopupStage newStage);

  PopupStage m_popupStage;
  GameTimer m_stageTimer;

  LabelWidgetPtr m_messageLabel;
  ImageWidgetPtr m_portraitImage;

  RadioMessage m_message;

  String m_backgroundImage;

  float m_animateInTime;
  String m_animateInImage;
  int m_animateInFrames;

  float m_animateOutTime;
  String m_animateOutImage;
  int m_animateOutFrames;

  Vec2I m_chatOffset;
  Vec2I m_chatStartPosition;
  Vec2I m_chatEndPosition;

  float m_slideTimer;
  float m_slideTime;

  AudioInstancePtr m_chatterSound;
};

}

export module star.radio_message_popup;

export namespace Star {
  using ::Star::LabelWidget;
  using ::Star::LabelWidgetPtr;
  using ::Star::LabelWidgetConstPtr;
  using ::Star::LabelWidgetWeakPtr;
  using ::Star::LabelWidgetConstWeakPtr;
  using ::Star::LabelWidgetUPtr;
  using ::Star::LabelWidgetConstUPtr;
  using ::Star::ImageWidget;
  using ::Star::ImageWidgetPtr;
  using ::Star::ImageWidgetConstPtr;
  using ::Star::ImageWidgetWeakPtr;
  using ::Star::ImageWidgetConstWeakPtr;
  using ::Star::ImageWidgetUPtr;
  using ::Star::ImageWidgetConstUPtr;
  using ::Star::AudioInstance;
  using ::Star::AudioInstancePtr;
  using ::Star::AudioInstanceConstPtr;
  using ::Star::AudioInstanceWeakPtr;
  using ::Star::AudioInstanceConstWeakPtr;
  using ::Star::AudioInstanceUPtr;
  using ::Star::AudioInstanceConstUPtr;
  using ::Star::RadioMessagePopup;
}
