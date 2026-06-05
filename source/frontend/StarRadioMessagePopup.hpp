#pragma once

#include "StarGameTimers.hpp"
#include "StarPane.hpp"
#include "StarAiTypes.hpp"
#include "StarRadioMessageDatabase.hpp"

namespace Star {

class LabelWidget;
using LabelWidgetPtr = SharedPtr<LabelWidget>;
class ImageWidget;
using ImageWidgetPtr = SharedPtr<ImageWidget>;
class AudioInstance;
using AudioInstancePtr = SharedPtr<AudioInstance>;

class RadioMessagePopup;
using RadioMessagePopupPtr = SharedPtr<RadioMessagePopup>;

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
