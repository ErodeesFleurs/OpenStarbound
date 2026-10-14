module;
#include "StarConfig.hpp"
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

namespace Star {

STAR_CLASS(CanvasWidget);
STAR_CLASS(BaseScriptPane);

// A more 'raw' script pane that doesn't depend on a world being present.
// Requires a derived class to provide a Lua root.
// Should maybe move into windowing?

class BaseScriptPane : public Pane {
public:
  BaseScriptPane(Json config, bool construct = true);

  virtual void show() override;
  void displayed() override;
  void dismissed() override;

  void tick(float dt) override;

  bool sendEvent(InputEvent const& event) override;
  
  Json const& config() const;
  Json const& rawConfig() const;

  bool interactive() const override;

  PanePtr createTooltip(Vec2I const& screenPosition) override;
  Maybe<String> cursorOverride(Vec2I const& screenPosition) override;
  Maybe<ItemPtr> shiftItemFromInventory(ItemPtr const& input) override;

protected:
  virtual GuiReaderPtr reader() override;
  void construct(Json config);

  Json m_config;
  Json m_rawConfig;

  GuiReaderPtr m_reader;

  Map<CanvasWidgetPtr, String> m_canvasClickCallbacks;
  Map<CanvasWidgetPtr, String> m_canvasKeyCallbacks;

  bool m_interactive;

  bool m_callbacksAdded;
  mutable LuaUpdatableComponent<LuaBaseComponent> m_script;
};

}

export module star.base_script_pane;

export namespace Star {
  using ::Star::CanvasWidget;
  using ::Star::CanvasWidgetPtr;
  using ::Star::CanvasWidgetConstPtr;
  using ::Star::CanvasWidgetWeakPtr;
  using ::Star::CanvasWidgetConstWeakPtr;
  using ::Star::CanvasWidgetUPtr;
  using ::Star::CanvasWidgetConstUPtr;
  using ::Star::BaseScriptPane;
  using ::Star::BaseScriptPanePtr;
  using ::Star::BaseScriptPaneConstPtr;
  using ::Star::BaseScriptPaneWeakPtr;
  using ::Star::BaseScriptPaneConstWeakPtr;
  using ::Star::BaseScriptPaneUPtr;
  using ::Star::BaseScriptPaneConstUPtr;
}
