module;
import std;
import star.json;
import star.lua;


namespace Star {

class Image;
using ImagePtr = std::shared_ptr<Image>;
using ImageConstPtr = std::shared_ptr<Image const>;
using ImageWeakPtr = std::weak_ptr<Image>;
using ImageConstWeakPtr = std::weak_ptr<Image const>;
using ImageUPtr = std::unique_ptr<Image>;
using ImageConstUPtr = std::unique_ptr<Image const>;

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
