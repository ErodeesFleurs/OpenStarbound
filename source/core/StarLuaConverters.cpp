#include "StarLuaConverters.hpp"

#include "StarColor.hpp"

import std;

namespace Star {

auto LuaConverter<Color>::from(LuaEngine& engine, Color const& c) -> LuaValue {
  if (c.alpha() == 255)
    return engine.createArrayTable(std::initializer_list<std::uint8_t>{c.red(), c.green(), c.blue()});
  else
    return engine.createArrayTable(std::initializer_list<std::uint8_t>{c.red(), c.green(), c.blue(), c.alpha()});
}

auto LuaConverter<Color>::to(LuaEngine& engine, LuaValue const& v) -> std::optional<Color> {
  if (auto t = v.ptr<LuaTable>()) {
    Color c = Color::rgba(0, 0, 0, 255);
    std::optional<int> r = engine.luaMaybeTo<int>(t->get(1));
    std::optional<int> g = engine.luaMaybeTo<int>(t->get(2));
    std::optional<int> b = engine.luaMaybeTo<int>(t->get(3));
    if (!r || !g || !b)
      return {};

    c.setRed(*r);
    c.setGreen(*g);
    c.setBlue(*b);

    if (std::optional<int> a = engine.luaMaybeTo<int>(t->get(4))) {
      if (!a)
        return {};
      c.setAlpha(*a);
    }

    return c;
  } else if (auto s = v.ptr<LuaString>()) {
    try {
      return Color(s->ptr());
    } catch (ColorException const&) {}
  }

  return {};
}

auto LuaConverter<LuaCallbacks>::from(LuaEngine& engine, LuaCallbacks const& c) -> LuaValue {
  auto table = engine.createTable(0, c.callbacks().size());
  for (auto& callback : c.callbacks())
    table.set(callback.first, engine.createWrappedFunction(callback.second));

  return table;
}

auto LuaConverter<LuaCallbacks>::to(LuaEngine&, LuaValue const&) -> std::optional<LuaCallbacks> {
  return {};
}

}// namespace Star
