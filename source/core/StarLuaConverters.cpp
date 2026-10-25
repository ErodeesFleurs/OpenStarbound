import std;
import star.format;
import star.lua_converters;
import star.lua;
import star.color;
import star.string_view;

namespace Star {

std::expected<LuaValue, StarException> LuaConverter<Color>::from(LuaEngine& engine, Color const& c) {
  auto table = c.alpha() == 255
      ? engine.createArrayTable(std::initializer_list<std::uint8_t>{c.red(), c.green(), c.blue()})
      : engine.createArrayTable(std::initializer_list<std::uint8_t>{c.red(), c.green(), c.blue(), c.alpha()});
  if (!table)
    return std::unexpected(std::move(table.error()));
  return std::move(*table);
}

std::expected<std::optional<Color>, StarException> LuaConverter<Color>::to(LuaEngine& engine, LuaValue const& v) {
  if (std::holds_alternative<LuaTable>(v)) {
    auto const& table = std::get<LuaTable>(v);
    std::array<int, 3> channels;
    for (std::size_t i = 0; i < channels.size(); ++i) {
      auto value = table.get<LuaValue>(i + 1);
      if (!value)
        return std::unexpected(std::move(value.error()));
      auto channel = engine.luaMaybeTo<int>(*value);
      if (!channel)
        return std::unexpected(std::move(channel.error()));
      if (!*channel)
        return std::nullopt;
      channels[i] = **channel;
    }
    Color c = Color::rgba(channels[0], channels[1], channels[2], 255);
    auto value = table.get<LuaValue>(4);
    if (!value)
      return std::unexpected(std::move(value.error()));
    auto alpha = engine.luaMaybeTo<int>(*value);
    if (!alpha)
      return std::unexpected(std::move(alpha.error()));
    if (*alpha)
      c.setAlpha(**alpha);
    return c;
  } else if (std::holds_alternative<LuaString>(v)) {
    auto text = std::get<LuaString>(v).view().utf8();
    auto color = Color::create(StringView(text.substr(0, text.find('\0'))));
    if (!color) {
      if (color.error().is<ColorException>())
        return std::nullopt;
      return std::unexpected(std::move(color.error()));
    }
    return std::move(*color);
  }
  return std::nullopt;
}

std::expected<LuaValue, StarException> LuaConverter<LuaCallbacks>::from(LuaEngine& engine, LuaCallbacks const& c) {
  auto created = engine.createTable(0, c.callbacks().size());
  if (!created)
    return std::unexpected(std::move(created.error()));
  auto table = std::move(*created);
  for (auto& callback : c.callbacks()) {
    auto function = engine.createWrappedFunction(callback.second);
    if (!function)
      return std::unexpected(std::move(function.error()));
    auto result = table.set(callback.first, std::move(*function));
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return table;
}

std::expected<std::optional<LuaCallbacks>, StarException> LuaConverter<LuaCallbacks>::to(LuaEngine&, LuaValue const&) {
  return std::nullopt;
}

}
