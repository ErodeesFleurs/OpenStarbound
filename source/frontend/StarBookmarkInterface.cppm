module;
import std;
import star.type_identity;
import star.json;
import star.strong_typedef;
import star.data_stream;
import star.game_types;
import star.random;
import star.weighted_pool;
import star.either;
import star.casting;
import star.input_event;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.string;
import star.text;
import star.asset_path;
import star.listener;
import star.set;
import star.audio;
import star.map;


// Parse foundations before importing global bookmark value types on GCC.
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.sky_parameters;
import star.system_world;


import star.uuid;
import star.celestial_coordinate;
import star.warping;
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
import star.player_universe_map;
import star.format;

namespace Star {

class EditBookmarkDialog : public Pane {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<EditBookmarkDialog, Pane>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<EditBookmarkDialog>();
  }

  [[nodiscard]] static std::expected<std::shared_ptr<EditBookmarkDialog>, StarException> create(GuiContext& guiContext, std::shared_ptr<PlayerUniverseMap> playerUniverseMap);

  [[nodiscard]] std::expected<void, StarException> show() override;

  [[nodiscard]] std::expected<void, StarException> setBookmark(TeleportBookmark bookmark);

  [[nodiscard]] std::expected<void, StarException> ok();
  [[nodiscard]] std::expected<void, StarException> remove();
  void close();

private:
  EditBookmarkDialog(GuiContext& guiContext, std::shared_ptr<PlayerUniverseMap> playerUniverseMap);
  [[nodiscard]] std::expected<void, StarException> initialize();
  std::shared_ptr<PlayerUniverseMap> m_playerUniverseMap;
  TeleportBookmark m_bookmark;

  bool m_isNew;
};

[[nodiscard]] std::expected<void, StarException> setupBookmarkEntry(std::shared_ptr<Widget> const& entry, TeleportBookmark const& bookmark);
}

export module star.bookmark_interface;

export namespace Star {
  using ::Star::EditBookmarkDialog;
  using ::Star::setupBookmarkEntry;
}
