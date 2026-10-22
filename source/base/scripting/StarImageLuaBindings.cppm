module;
import std;
import star.format;
import star.json;
import star.lua;


namespace Star {

class Image;

template <>
struct LuaConverter<Image> : LuaUserDataConverter<Image> {};

template <>
struct LuaUserDataMethods<Image> {
  [[nodiscard]] static std::expected<LuaMethods<Image>, StarException> make();
};

}

export module star.image_lua_bindings;

export namespace Star {
  template <>
  struct LuaConverter<Image>;

  template <>
  struct LuaUserDataMethods<Image>;

  using ::Star::Image;
  using ::Star::LuaConverter;
  using ::Star::LuaUserDataMethods;
}
