import star.lua_root;
import std;
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


import star.voice_settings_menu;

import star.voice_lua_bindings;

namespace Star {

VoiceSettingsMenu::VoiceSettingsMenu(GuiContext& guiContext, Json const& config) : BaseScriptPane(guiContext, config) {
  m_script.setLuaRoot(std::make_shared<LuaRoot>(context()->root()));
  m_script.addCallbacks("voice", LuaBindings::makeVoiceCallbacks(context()->voice()));
}

void VoiceSettingsMenu::show() {
  BaseScriptPane::show();
}

void VoiceSettingsMenu::displayed() {
  BaseScriptPane::displayed();
}

void VoiceSettingsMenu::dismissed() {
  BaseScriptPane::dismissed();
}

}