module;
import std;
import star.json;

import star.vector;
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
#include "StarLuaComponents.hpp"




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
import star.widget_parsing;
import star.gui_reader;
import star.base_script_pane;

namespace Star {

class CanvasWidget;
class UniverseClient;

class ScriptPane : public BaseScriptPane {
public:
  ScriptPane(std::shared_ptr<UniverseClient> client, Json config, EntityId sourceEntityId = NullEntityId);

  void displayed() override;
  void dismissed() override;

  void tick(float dt) override;

  std::shared_ptr<Pane> createTooltip(Vec2I const& screenPosition) override;

  bool openWithInventory() const;
  bool closeWithInventory() const;

  EntityId sourceEntityId() const;

  LuaCallbacks makePaneCallbacks() override;
private:
  std::shared_ptr<UniverseClient> m_client;
  EntityId m_sourceEntityId;
};

}

export module star.script_pane;

export namespace Star {
  using ::Star::CanvasWidget;
  using ::Star::ScriptPane;
  using ::Star::UniverseClient;
}
