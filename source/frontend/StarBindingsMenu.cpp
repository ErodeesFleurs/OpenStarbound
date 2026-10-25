import star.lua_root;
import std;
import star.format;
import star.json;
import star.casting;
import star.input_event;
import star.color;
import star.font;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.version;
import star.string;
import star.text;
import star.asset_path;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;



import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
import star.renderer;
import star.lua_components;
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
import star.widget_parsing;
import star.gui_reader;


import star.base_script_pane;


import star.bindings_menu;

import star.input_lua_bindings;

namespace Star {

BindingsMenu::BindingsMenu(GuiContext& guiContext) : BaseScriptPane(guiContext) {}

std::expected<std::shared_ptr<BindingsMenu>, StarException> BindingsMenu::create(GuiContext& guiContext, Json const& config) {
  auto menu = std::shared_ptr<BindingsMenu>(new BindingsMenu(guiContext));
  if (auto result = menu->init(config); !result)
    return std::unexpected(std::move(result.error()));
  auto luaRoot = LuaRoot::create(guiContext.root());
  if (!luaRoot)
    return std::unexpected(std::move(luaRoot.error()));
  if (auto result = menu->m_script.setLuaRoot(std::move(*luaRoot)); !result)
    return std::unexpected(std::move(result.error()));
  auto input = guiContext.input();
  if (!input)
    return std::unexpected(std::move(input.error()));
  auto callbacks = LuaBindings::makeInputCallbacks(input->get());
  if (!callbacks)
    return std::unexpected(std::move(callbacks.error()));
  if (auto result = menu->m_script.addCallbacks("input", std::move(*callbacks)); !result)
    return std::unexpected(std::move(result.error()));
  return menu;
}

std::expected<void, StarException> BindingsMenu::show() {
  return BaseScriptPane::show();
}

std::expected<void, StarException> BindingsMenu::displayed() {
  return BaseScriptPane::displayed();
}

std::expected<void, StarException> BindingsMenu::dismissed() {
  return BaseScriptPane::dismissed();
}

}