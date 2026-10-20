module;
import std;
import star.type_identity;
import star.json;

import star.game_types;

import star.vector;
import star.casting;
import star.input_event;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.string;
import star.text;
import star.asset_path;
import star.maybe;
import star.listener;
import star.set;
import star.audio;
import star.map;
import star.ordered_set;
import star.strong_typedef;




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

class LabelWidget;
class ImageWidget;
class AudioInstance;

class RadioMessagePopup : public Pane {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<RadioMessagePopup, Pane>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<RadioMessagePopup>();
  }

  RadioMessagePopup(GuiContext& guiContext);

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

  std::shared_ptr<LabelWidget> m_messageLabel;
  std::shared_ptr<ImageWidget> m_portraitImage;

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

  std::shared_ptr<AudioInstance> m_chatterSound;
};

}

export module star.radio_message_popup;

export namespace Star {
  using ::Star::LabelWidget;
  using ::Star::ImageWidget;
  using ::Star::AudioInstance;
  using ::Star::RadioMessagePopup;
}
