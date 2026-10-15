module;
import std;
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
import star.uuid;

namespace Star {

class PlayerStorage;
class Player;

class CharSelectionPane : public Pane {
public:
  typedef std::function<void()> CreateCharCallback;
  typedef std::function<void(std::shared_ptr<Player> const&)> SelectCharacterCallback;
  typedef std::function<void(Uuid)> DeleteCharacterCallback;

  CharSelectionPane(std::shared_ptr<PlayerStorage> playerStorage, CreateCharCallback createCallback,
      SelectCharacterCallback selectCallback, DeleteCharacterCallback deleteCallback);

  bool sendEvent(InputEvent const& event) override;
  void show() override;
  void updateCharacterPlates();
  void setReadOnly(bool readOnly);

private:
  void shiftCharacters(int movement);
  void selectCharacter(unsigned buttonIndex);

  std::shared_ptr<PlayerStorage> m_playerStorage;
  unsigned m_downScroll;
  String m_search;
  List<Uuid> m_filteredList;
  bool m_readOnly = false;

  CreateCharCallback m_createCallback;
  SelectCharacterCallback m_selectCallback;
  DeleteCharacterCallback m_deleteCallback;
};
}

export module star.char_selection;

export namespace Star {
  using ::Star::PlayerStorage;
  using ::Star::Player;
  using ::Star::CharSelectionPane;
}
