module;
import std;
import star.json;

import star.lua;
#include "StarCasting.hpp"
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
import star.variant;
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
import star.maybe;
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
using WidgetPtr = std::shared_ptr<Widget>;
using WidgetConstPtr = std::shared_ptr<Widget const>;
using WidgetWeakPtr = std::weak_ptr<Widget>;
using WidgetConstWeakPtr = std::weak_ptr<Widget const>;
using WidgetUPtr = std::unique_ptr<Widget>;
using WidgetConstUPtr = std::unique_ptr<Widget const>;
class CanvasWidget;
using CanvasWidgetPtr = std::shared_ptr<CanvasWidget>;
using CanvasWidgetConstPtr = std::shared_ptr<CanvasWidget const>;
using CanvasWidgetWeakPtr = std::weak_ptr<CanvasWidget>;
using CanvasWidgetConstWeakPtr = std::weak_ptr<CanvasWidget const>;
using CanvasWidgetUPtr = std::unique_ptr<CanvasWidget>;
using CanvasWidgetConstUPtr = std::unique_ptr<CanvasWidget const>;

template <>
struct LuaConverter<CanvasWidgetPtr> : LuaUserDataConverter<CanvasWidgetPtr> {};

template <>
struct LuaUserDataMethods<CanvasWidgetPtr> {
  static LuaMethods<CanvasWidgetPtr> make();
};

namespace LuaBindings {
  LuaCallbacks makeWidgetCallbacks(Widget* parentWidget, GuiReaderPtr reader = {});
}

}

export module star.widget_lua_bindings;

export namespace Star {
  template <>
  struct LuaConverter<CanvasWidgetPtr>;

  template <>
  struct LuaUserDataMethods<CanvasWidgetPtr>;

  using ::Star::LuaConverter;
  using ::Star::LuaUserDataMethods;
  using ::Star::Widget;
  using ::Star::WidgetPtr;
  using ::Star::WidgetConstPtr;
  using ::Star::WidgetWeakPtr;
  using ::Star::WidgetConstWeakPtr;
  using ::Star::WidgetUPtr;
  using ::Star::WidgetConstUPtr;
  using ::Star::CanvasWidget;
  using ::Star::CanvasWidgetPtr;
  using ::Star::CanvasWidgetConstPtr;
  using ::Star::CanvasWidgetWeakPtr;
  using ::Star::CanvasWidgetConstWeakPtr;
  using ::Star::CanvasWidgetUPtr;
  using ::Star::CanvasWidgetConstUPtr;
}

export namespace Star::LuaBindings {
  using ::Star::LuaBindings::makeWidgetCallbacks;
}
