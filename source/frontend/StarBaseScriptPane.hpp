#pragma once

#include "StarPane.hpp"
#include "StarLuaComponents.hpp"
#include "StarGuiReader.hpp"
#include "StarIAssets.hpp"

namespace Star {

class CanvasWidget;
using CanvasWidgetPtr = SharedPtr<CanvasWidget>;
class ItemDatabase;
using ItemDatabaseConstPtr = SharedPtr<ItemDatabase const>;

struct BaseScriptPaneServices {
  IAssetsConstPtr assets;
  ItemDatabaseConstPtr itemDatabase;
};

// A more 'raw' script pane that doesn't depend on a world being present.
// Requires a derived class to provide a Lua root.
// Should maybe move into windowing?

class BaseScriptPane : public Pane {
public:
  BaseScriptPane(Json config, bool construct = true, BaseScriptPaneServices services = {});

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
  IAssetsConstPtr m_assets;
  ItemDatabaseConstPtr m_itemDatabase;

  GuiReaderPtr m_reader;

  Map<CanvasWidgetPtr, String> m_canvasClickCallbacks;
  Map<CanvasWidgetPtr, String> m_canvasKeyCallbacks;

  bool m_interactive;

  bool m_callbacksAdded;
  mutable LuaUpdatableComponent<LuaBaseComponent> m_script;
};

}
