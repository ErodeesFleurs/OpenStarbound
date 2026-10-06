#pragma once
#include "StarGameTypes.hpp"
#include "StarJson.hpp"

#include "StarPane.hpp"
#include "StarOrderedSet.hpp"
#include "StarJson.hpp"
#include "StarDataStream.hpp"
#include "StarBiMap.hpp"
import star.item_descriptor;
#include "StarJson.hpp"
#include "StarPoly.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarJson.hpp"
#include "StarDataStream.hpp"
#include "StarBiMap.hpp"
import star.item_descriptor;
#include "StarVector.hpp"
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
