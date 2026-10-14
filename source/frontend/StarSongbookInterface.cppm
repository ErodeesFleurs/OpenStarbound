module;
#include "StarConfig.hpp"
import star.json;

#include "StarCasting.hpp"
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

STAR_CLASS(Player);

STAR_CLASS(SongbookInterface);

class SongbookInterface : public Pane {
public:
  SongbookInterface(PlayerPtr player);

  void update(float dt) override;

private:
  PlayerPtr m_player;
  StringList m_files;
  String m_lastSearch;
  CallbackListenerPtr m_reloadListener;
  bool play();
  void refresh(bool reloadFiles = false);
};

}

export module star.songbook_interface;

export namespace Star {
  using ::Star::Player;
  using ::Star::PlayerPtr;
  using ::Star::PlayerConstPtr;
  using ::Star::PlayerWeakPtr;
  using ::Star::PlayerConstWeakPtr;
  using ::Star::PlayerUPtr;
  using ::Star::PlayerConstUPtr;
  using ::Star::SongbookInterface;
  using ::Star::SongbookInterfacePtr;
  using ::Star::SongbookInterfaceConstPtr;
  using ::Star::SongbookInterfaceWeakPtr;
  using ::Star::SongbookInterfaceConstWeakPtr;
  using ::Star::SongbookInterfaceUPtr;
  using ::Star::SongbookInterfaceConstUPtr;
}
