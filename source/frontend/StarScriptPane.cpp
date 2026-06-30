#include "StarScriptPane.hpp"
#include "StarAlgorithm.hpp"
#include "StarGuiReader.hpp"
#include "StarJsonExtra.hpp"
#include "StarConfigLuaBindings.hpp"
#include "StarPlayerLuaBindings.hpp"
#include "StarStatusControllerLuaBindings.hpp"
#include "StarCelestialLuaBindings.hpp"
#include "StarLuaGameConverters.hpp"
#include "StarWorldClient.hpp"
#include "StarPlayer.hpp"
#include "StarUniverseClient.hpp"
#include "StarWidgetLuaBindings.hpp"
#include "StarInterfaceLuaBindings.hpp"
#include "StarCanvasWidget.hpp"
#include "StarItemTooltip.hpp"
#include "StarItemGridWidget.hpp"
#include "StarSimpleTooltip.hpp"
#include "StarImageWidget.hpp"

namespace Star {

ScriptPane::ScriptPane(UniverseClientPtr client, Json config, EntityId sourceEntityId, BaseScriptPaneServices services)
  : BaseScriptPane(config, true, std::move(services))
  , m_client(requireServiceValueAs<StarException>(std::move(client), "ScriptPane", "universe client"))
  , m_sourceEntityId(sourceEntityId) {
  m_script.addCallbacks("player", LuaBindings::makePlayerCallbacks(*m_client->mainPlayer()));
  m_script.addCallbacks("status", LuaBindings::makeStatusControllerCallbacks(*m_client->mainPlayer()->statusController()));
  m_script.addCallbacks("celestial", LuaBindings::makeCelestialCallbacks(*m_client, m_client->biomeDatabase()));
}

void ScriptPane::displayed() {
  auto world = m_client->worldClient();
  if (world && world->inWorld()) {
    m_script.setLuaRoot(world->luaRoot());
    m_script.addCallbacks("world", LuaBindings::makeWorldCallbacks(*(World*)world.get()));
  }
  BaseScriptPane::displayed();
}

void ScriptPane::dismissed() {
  BaseScriptPane::dismissed();
  m_script.removeCallbacks("world");
}

void ScriptPane::tick(float dt) {
  if (m_sourceEntityId != NullEntityId && !m_client->worldClient()->playerCanReachEntity(m_sourceEntityId))
    dismiss();

  BaseScriptPane::tick(dt);
}

PanePtr ScriptPane::createTooltip(Vec2I const& screenPosition) {
  auto result = m_script.invoke<Json>("createTooltip", screenPosition);
  if (result && !result.value().isNull()) {
    if (result->type() == Json::Type::String) {
      return SimpleTooltipBuilder::buildTooltip(result->toString(), SimpleTooltipServices{m_assets, context()});
    } else {
      PanePtr tooltip = make_shared<Pane>(context());
      m_reader->construct(*result, tooltip.get());
      return tooltip;
    }
  } else {
    ItemPtr item;
    if (auto child = getChildAt(screenPosition)) {
      if (auto itemSlot = as<ItemSlotWidget>(child))
        item = itemSlot->item();
      if (auto itemGrid = as<ItemGridWidget>(child))
        item = itemGrid->itemAt(screenPosition);
    }
    if (item)
      return ItemTooltipBuilder::buildItemTooltip(item, m_client->mainPlayer(), {m_assets, m_objectDatabase, m_statusEffectDatabase, context()});
    return {};
  }
}

LuaCallbacks ScriptPane::makePaneCallbacks() {
  LuaCallbacks callbacks = BaseScriptPane::makePaneCallbacks();
  callbacks.registerCallback("sourceEntity", [this]() { return m_sourceEntityId; });
  return callbacks;
}

bool ScriptPane::openWithInventory() const {
  return m_config.getBool("openWithInventory", false);
}

bool ScriptPane::closeWithInventory() const {
  return m_config.getBool("closeWithInventory", openWithInventory());
}

EntityId ScriptPane::sourceEntityId() const {
  return m_sourceEntityId;
}

}
