module;
import std;
import star.type_identity;
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
import star.format;

namespace Star {

class Player;

class SongbookInterface : public Pane {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<SongbookInterface, Pane>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<SongbookInterface>();
  }

  [[nodiscard]] static std::expected<std::shared_ptr<SongbookInterface>, StarException> create(GuiContext& guiContext, std::shared_ptr<Player> player);
  ~SongbookInterface() override;

  [[nodiscard]] std::expected<void, StarException> update(float dt) override;

private:
  SongbookInterface(GuiContext& guiContext, std::shared_ptr<Player> player);
  [[nodiscard]] std::expected<void, StarException> initialize();
  std::shared_ptr<Player> m_player;
  StringList m_files;
  String m_lastSearch;
  std::shared_ptr<CallbackListener> m_reloadListener;
  ListenerSubscription m_reloadSubscription;
  [[nodiscard]] std::expected<bool, StarException> play();
  [[nodiscard]] std::expected<void, StarException> refresh(bool reloadFiles = false);
};

}

export module star.songbook_interface;

export namespace Star {
  using ::Star::Player;
  using ::Star::SongbookInterface;
}
