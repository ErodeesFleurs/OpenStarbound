#include "StarBaseScriptPane.hpp"
#include "StarAlgorithm.hpp"
#include "StarGuiReader.hpp"
#include "StarJsonExtra.hpp"
#include "StarConfigLuaBindings.hpp"
#include "StarLuaGameConverters.hpp"
#include "StarWidgetLuaBindings.hpp"
#include "StarCanvasWidget.hpp"
#include "StarItemDatabase.hpp"
#include "StarItemTooltip.hpp"
#include "StarItemGridWidget.hpp"
#include "StarSimpleTooltip.hpp"
#include "StarImageWidget.hpp"

namespace Star {

BaseScriptPane::BaseScriptPane(Json config, bool construct, BaseScriptPaneServices services)
  : Pane(services.guiContext),
    m_rawConfig(config),
    m_assets(requireServiceValueAs<StarException>(std::move(services.assets), "BaseScriptPane", "assets")),
    m_itemDatabase(std::move(services.itemDatabase)),
    m_objectDatabase(std::move(services.objectDatabase)),
    m_statusEffectDatabase(std::move(services.statusEffectDatabase)),
    m_luaRootServices(std::move(services.luaRootServices)) {
  if (config.type() == Json::Type::Object && config.contains("baseConfig")) {
    auto baseConfig = m_assets->fetchJson(config.getString("baseConfig"));
    m_config = jsonMerge(baseConfig, config);
  } else {
    m_config = m_assets->fetchJson(config);
  }
  
  m_interactive = m_config.getBool("interactive", true);
  m_reader = make_shared<GuiReader>(context());
  m_reader->registerCallback("close", [this](Widget*) { dismiss(); });

  for (auto const& callbackName : jsonToStringList(m_config.get("scriptWidgetCallbacks", JsonArray{}))) {
    m_reader->registerCallback(callbackName, [this, callbackName](Widget* widget) {
      m_script.invoke(callbackName, widget->name(), widget->data());
    });
  }

  if (construct)
    this->construct(m_assets->fetchJson(m_config.get("gui")));
}


void BaseScriptPane::displayed() {
  Pane::displayed();
  if (!m_callbacksAdded) {
    m_script.addCallbacks("pane", makePaneCallbacks());
    m_script.addCallbacks("widget", LuaBindings::makeWidgetCallbacks(*this, m_reader));
    m_script.addCallbacks("config", LuaBindings::makeConfigCallbacks( [this](String const& name, Json const& def) {
      return m_config.query(name, def);
    }));
    m_callbacksAdded = true;
  }
  m_script.init();
  m_script.invoke("displayed");
}

void BaseScriptPane::dismissed() {
  Pane::dismissed();
  m_script.invoke("dismissed");
  m_script.uninit();
}

void BaseScriptPane::tick(float dt) {
  Pane::tick(dt);

  for (auto const& [canvas, callback] : m_canvasClickCallbacks) {
    for (auto const& clickEvent : canvas->pullClickEvents())
      m_script.invoke(callback, jsonFromVec2I(clickEvent.position), static_cast<uint8_t>(clickEvent.button), clickEvent.buttonDown);
  }
  for (auto const& [canvas, callback] : m_canvasKeyCallbacks) {
    for (auto const& keyEvent : canvas->pullKeyEvents())
      m_script.invoke(callback, static_cast<int>(keyEvent.key), keyEvent.keyDown, KeyNames.getRight(keyEvent.key));
  }

  m_script.update(m_script.updateDt(dt));
}

bool BaseScriptPane::sendEvent(InputEvent const& event) {
  // Intercept GuiClose before the canvas child so GuiClose always closes
  // BaseScriptPanes without having to support it in the script.
  if (context().actions(event).contains(InterfaceAction::GuiClose)
    && m_config.getBool("dismissable", true)) {
    dismiss();
    return true;
  }

  return Pane::sendEvent(event);
}

Json const& BaseScriptPane::config() const { return m_config; }
Json const& BaseScriptPane::rawConfig() const { return m_rawConfig; }

bool BaseScriptPane::interactive() const { return m_interactive; }

UniquePtr<Pane> BaseScriptPane::createTooltip(Vec2I const& screenPosition) {
  auto result = m_script.invoke<Json>("createTooltip", screenPosition);
  if (result && !result.value().isNull()) {
    if (result->type() == Json::Type::String) {
      return SimpleTooltipBuilder::buildTooltip(result->toString(), SimpleTooltipServices{m_assets, context()});
    } else {
      auto tooltip = make_unique<Pane>(context());
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
      return ItemTooltipBuilder::buildItemTooltip(item, {}, {m_assets, m_objectDatabase, m_statusEffectDatabase, context()});
    return {};
  }
}

Maybe<String> BaseScriptPane::cursorOverride(Vec2I const& screenPosition) {
  auto result = m_script.invoke<Maybe<String>>("cursorOverride", screenPosition);
  if (result)
    return *result;
  else
    return {};
}

Maybe<ItemPtr> BaseScriptPane::shiftItemFromInventory(ItemPtr const& input) {
  auto result = m_script.invoke<Json>("shiftItemFromInventory", input->descriptor().toJson());
  if (!result || result->isNull())
    return {};

  if (result->type() == Json::Type::Bool) {
    requireService(m_itemDatabase, "BaseScriptPane", "item database");
    if (result->toBool())
      return m_itemDatabase->item({});
    return {};
  }

  requireService(m_itemDatabase, "BaseScriptPane", "item database");
  return m_itemDatabase->item(ItemDescriptor(result.value()));
}

GuiReaderPtr BaseScriptPane::reader() {
  return m_reader;
}

void BaseScriptPane::construct(Json config) {
  m_reader->construct(config, this);

  for (auto const& [canvasName, callback] : m_config.getObject("canvasClickCallbacks", {}))
    m_canvasClickCallbacks.set(findChild<CanvasWidget>(canvasName).get(), callback.toString());
  for (auto const& [canvasName, callback] : m_config.getObject("canvasKeyCallbacks", {}))
    m_canvasKeyCallbacks.set(findChild<CanvasWidget>(canvasName).get(), callback.toString());

  m_script.setScripts(jsonToStringList(m_config.get("scripts", JsonArray())));
  m_script.setUpdateDelta(m_config.getUInt("scriptDelta", 1));
}

}
