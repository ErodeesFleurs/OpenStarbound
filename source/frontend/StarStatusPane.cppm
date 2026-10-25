module;
import std;
import star.type_identity;
import star.json;

import star.vector;
import star.casting;
import star.input_event;
import star.color;
import star.list;
import star.rect;
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

import star.ordered_map;




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
import star.game_timers;
import star.pane_manager;
import star.registered_pane_manager;
import star.main_interface_types;

import star.format;
namespace Star {

class Player;
class UniverseClient;

class StatusPane : public Pane {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<StatusPane, Pane>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<StatusPane>();
  }

  [[nodiscard]] static std::expected<std::shared_ptr<StatusPane>, StarException> create(GuiContext& guiContext, std::shared_ptr<UniverseClient> client);

  [[nodiscard]] std::expected<std::shared_ptr<Pane>, StarException> createTooltip(Vec2I const& screenPosition) override;

protected:
  [[nodiscard]] std::expected<void, StarException> renderImpl() override;
  [[nodiscard]] std::expected<void, StarException> update(float dt) override;

private:
  StatusPane(GuiContext& guiContext, std::shared_ptr<UniverseClient> client);
  struct StatusEffectIndicator {
    String icon;
    std::optional<float> durationPercentage;
    String label;
    RectF screenRect;
  };

  std::shared_ptr<UniverseClient> m_client;
  std::shared_ptr<Player> m_player;

  List<StatusEffectIndicator> m_statusIndicators;
};

}

export module star.status_pane;

export namespace Star {
  using ::Star::Player;
  using ::Star::UniverseClient;
  using ::Star::StatusPane;
}
