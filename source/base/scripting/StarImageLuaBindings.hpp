#pragma once

#include "StarLua.hpp"

namespace Star {

template <>
struct LuaConverter<Image> : LuaUserDataConverter<Image> {};

template <>
struct LuaUserDataMethods<Image> {
  static auto make() -> LuaMethods<Image>;
};

}
