#pragma once

#include "StarLua.hpp"
#include "StarGuiReader.hpp"

namespace Star {

class Widget;
using WidgetPtr = SharedPtr<Widget>;
class CanvasWidget;
using CanvasWidgetPtr = SharedPtr<CanvasWidget>;

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
