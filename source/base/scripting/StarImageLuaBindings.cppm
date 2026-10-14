module;
#include "StarConfig.hpp"
import star.json;
import star.lua;


namespace Star {

STAR_CLASS(Image);

template <>
struct LuaConverter<Image> : LuaUserDataConverter<Image> {};

template <>
struct LuaUserDataMethods<Image> {
  static LuaMethods<Image> make();
};

}

export module star.image_lua_bindings;

export namespace Star {
  template <>
  struct LuaConverter<Image>;

  template <>
  struct LuaUserDataMethods<Image>;

  using ::Star::Image;
  using ::Star::ImagePtr;
  using ::Star::ImageConstPtr;
  using ::Star::ImageWeakPtr;
  using ::Star::ImageConstWeakPtr;
  using ::Star::ImageUPtr;
  using ::Star::ImageConstUPtr;
  using ::Star::LuaConverter;
  using ::Star::LuaUserDataMethods;
}
