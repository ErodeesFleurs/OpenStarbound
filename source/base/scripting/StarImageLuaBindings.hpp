#pragma once

#include "StarLua.hpp"

namespace Star {

class Image;

template <>
struct LuaConverter<Image> : LuaUserDataConverter<Image> {};

template <>
struct LuaUserDataMethods<Image> {
  static LuaMethods<Image> make();
};

}
