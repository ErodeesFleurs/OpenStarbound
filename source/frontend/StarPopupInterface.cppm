module;
import std;
import star.json;

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
import star.pane;

namespace Star {

class PopupInterface : public Pane {
public:
  PopupInterface();

  virtual ~PopupInterface() {}

  void displayMessage(String const& message, String const& title, String const& subtitle, Maybe<String> const& onShowSound = {});

private:
};

}

export module star.popup_interface;

export namespace Star {
  using ::Star::PopupInterface;
}
