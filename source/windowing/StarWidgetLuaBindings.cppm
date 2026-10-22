module;
import std;
import star.format;
import star.json;

import star.lua;
import star.casting;
import star.input_event;
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
import star.color;
import star.font;
import star.bi_map;
import star.ref_ptr;
import star.renderer;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.string;
import star.text;
import star.asset_path;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;
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
import star.widget_parsing;
import star.gui_reader;

namespace Star {

class Widget;
class CanvasWidget;

template <>
struct LuaConverter<std::shared_ptr<CanvasWidget>> : LuaUserDataConverter<std::shared_ptr<CanvasWidget>> {};

template <>
struct LuaUserDataMethods<std::shared_ptr<CanvasWidget>> {
  [[nodiscard]] static std::expected<LuaMethods<std::shared_ptr<CanvasWidget>>, StarException> make();
};

namespace LuaBindings {
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeWidgetCallbacks(GuiContext& context, Widget* parentWidget, std::shared_ptr<GuiReader> reader = {});
}

}

export module star.widget_lua_bindings;

export namespace Star {
  template <>
  struct LuaConverter<std::shared_ptr<CanvasWidget>>;

  template <>
  struct LuaUserDataMethods<std::shared_ptr<CanvasWidget>>;

  using ::Star::LuaConverter;
  using ::Star::LuaUserDataMethods;
  using ::Star::Widget;
  using ::Star::CanvasWidget;
}

export namespace Star::LuaBindings {
  using ::Star::LuaBindings::makeWidgetCallbacks;
}
