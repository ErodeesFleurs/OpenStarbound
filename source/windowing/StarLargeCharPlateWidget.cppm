module;
import std;
import star.json;

import star.vector;
import star.casting;
import star.input_event;
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
import star.color;
import star.font;
import star.variant;
import star.bi_map;
import star.ref_ptr;
import star.renderer;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.string;
import star.text;
import star.asset_path;
import star.maybe;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;
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

class Player;

class LargeCharPlateWidget : public ButtonWidget {
public:
  LargeCharPlateWidget(WidgetCallbackFunc mainCallback, std::shared_ptr<Player> player = std::shared_ptr<Player>());

  void mouseOut() override;

  void setPlayer(std::shared_ptr<Player> player = std::shared_ptr<Player>());

  void enableDelete(WidgetCallbackFunc const& callback);
  void disableDelete();

  virtual bool sendEvent(InputEvent const& event) override;

  void update(float dt) override;

protected:
  virtual void renderImpl() override;

private:
  std::shared_ptr<Player> m_player;
  Json m_config;

  std::shared_ptr<PortraitWidget> m_portrait;
  Vec2I m_portraitOffset;
  float m_portraitScale;

  String m_playerPlateHover;
  String m_noPlayerPlate;
  String m_noPlayerPlateHover;
  String m_playerPlate;

  std::shared_ptr<LabelWidget> m_playerName;
  std::shared_ptr<LabelWidget> m_playerPhrase;
  std::shared_ptr<LabelWidget> m_modeName;
  std::shared_ptr<LabelWidget> m_mode;

  std::shared_ptr<ButtonWidget> m_delete;

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
}
