import std;
import star.json;
import star.json_extra;
import star.strong_typedef;
import star.data_stream;
import star.bi_map;
import star.game_types;
import star.vector;
import star.casting;
import star.input_event;
import star.color;
import star.font;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.version;
import star.string;
import star.text;
import star.asset_path;
import star.maybe;
import star.listener;
import star.set;
import star.audio;
import star.map;
import star.lua;

import star.celestial_coordinate;
import star.quest_descriptor;
import star.item;
import star.item_descriptor;
import star.item_recipe;

import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
import star.variant;
import star.renderer;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
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
import star.lua_components;
import star.widget_parsing;
import star.gui_reader;


import star.base_script_pane;
import star.lua_game_converters;
import star.widget_lua_bindings;
import star.canvas_widget;
import star.progress_widget;
import star.animation;
import star.item_slot_widget;
import star.item_grid_widget;
import star.image_widget;

import star.simple_tooltip;
import star.item_tooltip;
import star.config_lua_bindings;
import star.item_database;

namespace Star {

BaseScriptPane::BaseScriptPane(GuiContext& guiContext, Json config, bool construct) : Pane(guiContext), m_rawConfig(config), m_script(guiContext.root()) {
  auto& root = context()->root();
  auto assets = root.assets();

  if (config.type() == Json::Type::Object && config.contains("baseConfig")) {
    auto baseConfig = assets->fetchJson(config.getString("baseConfig"));
    m_config = jsonMerge(baseConfig, config);
  } else {
    m_config = assets->fetchJson(config);
  }
  
  m_interactive = m_config.getBool("interactive", true);
  m_reader = std::make_shared<GuiReader>(*context());
  m_reader->registerCallback("close", [this](Widget*) { dismiss(); });

  for (auto const& callbackName : jsonToStringList(m_config.get("scriptWidgetCallbacks", JsonArray{}))) {
    m_reader->registerCallback(callbackName, [this, callbackName](Widget* widget) {
      m_script.invoke(callbackName, widget->name(), widget->data());
    });
  }

  if (construct)
    this->construct(assets->fetchJson(m_config.get("gui")));

  m_callbacksAdded = false;
}

void BaseScriptPane::show() {
  Pane::show();
}

void BaseScriptPane::displayed() {
  Pane::displayed();
  if (!m_callbacksAdded) {
    m_script.addCallbacks("pane", makePaneCallbacks());
    m_script.addCallbacks("widget", LuaBindings::makeWidgetCallbacks(*context(), this, m_reader));
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

  for (auto p : m_canvasClickCallbacks) {
    for (auto const& clickEvent : p.first->pullClickEvents())
      m_script.invoke(p.second, jsonFromVec2I(clickEvent.position), (std::uint8_t)clickEvent.button, clickEvent.buttonDown);
  }
  for (auto p : m_canvasKeyCallbacks) {
    for (auto const& keyEvent : p.first->pullKeyEvents())
      m_script.invoke(p.second, (int)keyEvent.key, keyEvent.keyDown, KeyNames.getRight(keyEvent.key));
  }

  m_script.update(m_script.updateDt(dt));
}

bool BaseScriptPane::sendEvent(InputEvent const& event) {
  // Intercept GuiClose before the canvas child so GuiClose always closes
  // BaseScriptPanes without having to support it in the script.
  if (context()->actions(event).contains(InterfaceAction::GuiClose)
    && m_config.getBool("dismissable", true)) {
    dismiss();
    return true;
  }

  return Pane::sendEvent(event);
}

Json const& BaseScriptPane::config() const { return m_config; }
Json const& BaseScriptPane::rawConfig() const { return m_rawConfig; }

bool BaseScriptPane::interactive() const { return m_interactive; }

std::shared_ptr<Pane> BaseScriptPane::createTooltip(Vec2I const& screenPosition) {
  auto result = m_script.invoke<Json>("createTooltip", screenPosition);
  if (result && !result.value().isNull()) {
    if (result->type() == Json::Type::String) {
      return SimpleTooltipBuilder::buildTooltip(*context(), result->toString());
    } else {
      std::shared_ptr<Pane> tooltip = std::make_shared<Pane>(*context());
      m_reader->construct(*result, tooltip.get());
      return tooltip;
    }
  } else {
    std::shared_ptr<Item> item;
    if (auto child = getChildAt(screenPosition)) {
      if (auto itemSlot = as<ItemSlotWidget>(child))
        item = itemSlot->item();
      if (auto itemGrid = as<ItemGridWidget>(child))
        item = itemGrid->itemAt(screenPosition);
    }
    if (item)
      return ItemTooltipBuilder::buildItemTooltip(*context(), item);
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

Maybe<std::shared_ptr<Item>> BaseScriptPane::shiftItemFromInventory(std::shared_ptr<Item> const& input) {
  auto result = m_script.invoke<Json>("shiftItemFromInventory", input->descriptor().toJson());
  if (!result || result->isNull())
    return {};

  auto itemDatabase = context()->root().itemDatabase();

  if (result->type() == Json::Type::Bool) {
    if (result->toBool())
      return itemDatabase->item({});
    return {};
  }

  return itemDatabase->item(ItemDescriptor(result.value()));
}

std::shared_ptr<GuiReader> BaseScriptPane::reader() {
  return m_reader;
}

void BaseScriptPane::construct(Json config) {
  m_reader->construct(config, this);

  for (auto pair : m_config.getObject("canvasClickCallbacks", {}))
    m_canvasClickCallbacks.set(findChild<CanvasWidget>(pair.first), pair.second.toString());
  for (auto pair : m_config.getObject("canvasKeyCallbacks", {}))
    m_canvasKeyCallbacks.set(findChild<CanvasWidget>(pair.first), pair.second.toString());

  m_script.setScripts(jsonToStringList(m_config.get("scripts", JsonArray())));
  m_script.setUpdateDelta(m_config.getUInt("scriptDelta", 1));
}

}
