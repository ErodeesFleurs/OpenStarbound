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
using CanvasWidgetPtr = std::shared_ptr<CanvasWidget>;
using CanvasWidgetConstPtr = std::shared_ptr<CanvasWidget const>;
using CanvasWidgetWeakPtr = std::weak_ptr<CanvasWidget>;
using CanvasWidgetConstWeakPtr = std::weak_ptr<CanvasWidget const>;
using CanvasWidgetUPtr = std::unique_ptr<CanvasWidget>;
using CanvasWidgetConstUPtr = std::unique_ptr<CanvasWidget const>;
class ScriptPane;
using ScriptPanePtr = std::shared_ptr<ScriptPane>;
using ScriptPaneConstPtr = std::shared_ptr<ScriptPane const>;
using ScriptPaneWeakPtr = std::weak_ptr<ScriptPane>;
using ScriptPaneConstWeakPtr = std::weak_ptr<ScriptPane const>;
using ScriptPaneUPtr = std::unique_ptr<ScriptPane>;
using ScriptPaneConstUPtr = std::unique_ptr<ScriptPane const>;
class UniverseClient;
using UniverseClientPtr = std::shared_ptr<UniverseClient>;
using UniverseClientConstPtr = std::shared_ptr<UniverseClient const>;
using UniverseClientWeakPtr = std::weak_ptr<UniverseClient>;
using UniverseClientConstWeakPtr = std::weak_ptr<UniverseClient const>;
using UniverseClientUPtr = std::unique_ptr<UniverseClient>;
using UniverseClientConstUPtr = std::unique_ptr<UniverseClient const>;

class ScriptPane : public BaseScriptPane {
public:
  ScriptPane(UniverseClientPtr client, Json config, EntityId sourceEntityId = NullEntityId);

  void displayed() override;
  void dismissed() override;

  void tick(float dt) override;

  PanePtr createTooltip(Vec2I const& screenPosition) override;

  bool openWithInventory() const;
  bool closeWithInventory() const;

  EntityId sourceEntityId() const;

  LuaCallbacks makePaneCallbacks() override;
private:
  UniverseClientPtr m_client;
  EntityId m_sourceEntityId;
};

}

export module star.script_pane;

export namespace Star {
  using ::Star::CanvasWidget;
  using ::Star::CanvasWidgetPtr;
  using ::Star::CanvasWidgetConstPtr;
  using ::Star::CanvasWidgetWeakPtr;
  using ::Star::CanvasWidgetConstWeakPtr;
  using ::Star::CanvasWidgetUPtr;
  using ::Star::CanvasWidgetConstUPtr;
  using ::Star::ScriptPane;
  using ::Star::ScriptPanePtr;
  using ::Star::ScriptPaneConstPtr;
  using ::Star::ScriptPaneWeakPtr;
  using ::Star::ScriptPaneConstWeakPtr;
  using ::Star::ScriptPaneUPtr;
  using ::Star::ScriptPaneConstUPtr;
  using ::Star::UniverseClient;
  using ::Star::UniverseClientPtr;
  using ::Star::UniverseClientConstPtr;
  using ::Star::UniverseClientWeakPtr;
  using ::Star::UniverseClientConstWeakPtr;
  using ::Star::UniverseClientUPtr;
  using ::Star::UniverseClientConstUPtr;
}
